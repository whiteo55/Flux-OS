#include "gui.h"
#include "gfx.h"

// PS/2 Mouse Controller ports
#define MOUSE_DATA_PORT    0x60
#define MOUSE_STATUS_PORT  0x64
#define MOUSE_COMMAND_PORT 0x64

// Mouse status register bits
#define MOUSE_STATUS_OUT_BUFFER  0x01
#define MOUSE_STATUS_IN_BUFFER  0x02
#define MOUSE_STATUS_SYSTEM      0x04
#define MOUSE_STATUS_GATE_A20    0x20
#define MOUSE_STATUS_TRANS_TIMEOUT  0x40
#define MOUSE_STATUS_PARITY_TIMEOUT 0x80

// Mouse packet flags
#define MOUSE_PACKET_Y_OVERFLOW  0x20
#define MOUSE_PACKET_X_OVERFLOW  0x10
#define MOUSE_PACKET_Y_SIGN      0x08
#define MOUSE_PACKET_X_SIGN      0x04
#define MOUSE_PACKET_MID_BUTTON  0x04
#define MOUSE_PACKET_RIGHT_BUTTON 0x02
#define MOUSE_PACKET_LEFT_BUTTON  0x01

// Mouse state
static int g_mouse_x = 0;
static int g_mouse_y = 0;
static int g_mouse_buttons = 0;
static int g_mouse_wheel = 0;
static int g_mouse_packet_byte = 0;
static uint8_t g_mouse_packet[3] = {0};
static int g_mouse_dx = 0;
static int g_mouse_dy = 0;

// Outb wrapper for port I/O
static void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Inb wrapper for port I/O
static uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Wait for mouse to be ready
static void mouse_wait(uint8_t type) {
    int timeout = 100000;
    while (timeout--) {
        uint8_t status = inb(MOUSE_STATUS_PORT);
        if (type == 0) {
            // Wait for output buffer to be empty (can write)
            if ((status & MOUSE_STATUS_OUT_BUFFER) == 0) {
                return;
            }
        } else {
            // Wait for input buffer to be empty (can read)
            if ((status & MOUSE_STATUS_IN_BUFFER) == 0) {
                return;
            }
        }
    }
}

// Write to mouse
static void mouse_write(uint8_t val) {
    mouse_wait(0);  // Wait for write
    outb(MOUSE_COMMAND_PORT, 0xD4);  // Write to mouse
    mouse_wait(0);
    outb(MOUSE_DATA_PORT, val);
}

// Read from mouse
static uint8_t mouse_read() {
    mouse_wait(1);  // Wait for read
    return inb(MOUSE_DATA_PORT);
}

// Process a complete mouse packet
static void process_mouse_packet() {
    int buttons = 0;
    
    // Extract button states
    if (g_mouse_packet[0] & MOUSE_PACKET_LEFT_BUTTON) buttons |= 1 << MOUSE_BUTTON_LEFT;
    if (g_mouse_packet[0] & MOUSE_PACKET_RIGHT_BUTTON) buttons |= 1 << MOUSE_BUTTON_RIGHT;
    if (g_mouse_packet[0] & MOUSE_PACKET_MID_BUTTON) buttons |= 1 << MOUSE_BUTTON_MIDDLE;
    
    // Extract movement deltas
    int dx = (int8_t)g_mouse_packet[1];
    int dy = -(int8_t)g_mouse_packet[2];  // Y is inverted
    
    // Handle overflow
    if (!(g_mouse_packet[0] & MOUSE_PACKET_X_OVERFLOW)) {
        g_mouse_dx = dx;
    } else {
        g_mouse_dx = 0;
    }
    
    if (!(g_mouse_packet[0] & MOUSE_PACKET_Y_OVERFLOW)) {
        g_mouse_dy = dy;
    } else {
        g_mouse_dy = 0;
    }
    
    // Update absolute position
    g_mouse_x += g_mouse_dx;
    g_mouse_y += g_mouse_dy;
    
    // Clamp to screen bounds
    if (g_mouse_x < 0) g_mouse_x = 0;
    if (g_mouse_y < 0) g_mouse_y = 0;
    if (g_mouse_x >= gui.width) g_mouse_x = gui.width - 1;
    if (g_mouse_y >= gui.height) g_mouse_y = gui.height - 1;
    
    // Check for button changes
    int button_changed = (g_mouse_buttons != buttons);
    int button_pressed = (~g_mouse_buttons) & buttons;
    int button_released = g_mouse_buttons & (~buttons);
    
    g_mouse_buttons = buttons;
    
    // Create events
    event_t event;
    
    // Movement event
    event.type = EVENT_MOUSE_MOVE;
    event.mouse_x = g_mouse_x;
    event.mouse_y = g_mouse_y;
    gui_queue_event(&event);
    
    // Button press events
    for (int i = 0; i < 3; i++) {
        if (button_pressed & (1 << i)) {
            event.type = EVENT_MOUSE_DOWN;
            event.mouse_x = g_mouse_x;
            event.mouse_y = g_mouse_y;
            event.mouse_button = (mouse_button_t)i;
            gui_queue_event(&event);
        }
        if (button_released & (1 << i)) {
            event.type = EVENT_MOUSE_UP;
            event.mouse_x = g_mouse_x;
            event.mouse_y = g_mouse_y;
            event.mouse_button = (mouse_button_t)i;
            gui_queue_event(&event);
            
            // Click event (up after down at same position)
            event.type = EVENT_MOUSE_CLICK;
            event.mouse_x = g_mouse_x;
            event.mouse_y = g_mouse_y;
            event.mouse_button = (mouse_button_t)i;
            gui_queue_event(&event);
        }
    }
}

// Handle incoming mouse byte
void mouse_handle_packet(uint8_t byte0, uint8_t byte1, uint8_t byte2) {
    g_mouse_packet[0] = byte0;
    g_mouse_packet[1] = byte1;
    g_mouse_packet[2] = byte2;
    
    // Always process if we got here
    process_mouse_packet();
}

// Mouse interrupt handler
void mouse_interrupt_handler() {
    uint8_t byte = inb(MOUSE_DATA_PORT);
    
    if ((g_mouse_packet[0] & 0x08) == 0) {
        // First byte should have bit 3 set
        g_mouse_packet_byte = 0;
        return;
    }
    
    g_mouse_packet[g_mouse_packet_byte++] = byte;
    
    if (g_mouse_packet_byte >= 3) {
        g_mouse_packet_byte = 0;
        process_mouse_packet();
    }
}

// Get current mouse position
void mouse_get_position(int* x, int* y) {
    *x = g_mouse_x;
    *y = g_mouse_y;
}

// Initialize mouse
void mouse_init() {
    // Initialize state
    g_mouse_x = gui.width / 2;
    g_mouse_y = gui.height / 2;
    g_mouse_buttons = 0;
    g_mouse_packet_byte = 0;
    
    // Enable PS/2 mouse
    mouse_wait(0);
    outb(MOUSE_COMMAND_PORT, 0xA8);  // Enable auxiliary device
    
    // Get mouse configuration
    mouse_write(0xF5);  // Disable reporting
    mouse_read();  // ACK
    
    // Set sample rate to 100Hz
    mouse_write(0xF3);
    mouse_read();
    mouse_write(100);
    mouse_read();
    
    // Set resolution
    mouse_write(0xE8);
    mouse_read();
    mouse_write(3);  // 8 counts per mm
    mouse_read();
    
    // Enable packet streaming
    mouse_write(0xF4);
    mouse_read();  // ACK
    
    // Enable IRQ12 on PIC (would need PIC setup)
    // For now, we'll poll in the event loop
    
    // Mouse cursor is drawn by desktop.c
}

// Clear mouse cursor (restore background)
void clear_mouse_cursor(int x, int y) {
    // This needs to save and restore the background
    // For now, we just redraw the screen portion that was covered
    // A proper implementation would save the background before drawing
    
    // Request redraw of cursor area
    gui.needs_redraw = 1;
}

// Enable mouse streaming
void mouse_enable_streaming() {
    mouse_write(0xF4);
    mouse_read();
}

// Disable mouse
void mouse_disable() {
    mouse_write(0xF5);
    mouse_read();
}

// Set mouse position (for absolute positioning)
void mouse_set_position(int x, int y) {
    g_mouse_x = x;
    g_mouse_y = y;
}

// Enable mouse for polling
void enable_mouse() {
    // Enable PS/2 mouse port
    mouse_wait(0);
    outb(MOUSE_COMMAND_PORT, 0xA8);  // Enable auxiliary device
    
    // Send enable command to mouse
    mouse_write(0xF4);  // Enable data reporting
    mouse_read();  // ACK
    
    // Small delay
    for (int i = 0; i < 10000; i++) {
        __asm__("nop");
    }
}

