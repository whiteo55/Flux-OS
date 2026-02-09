#include "gui.h"

// Keyboard ports
#define KEYBOARD_DATA_PORT  0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_CMD_PORT   0x64

// Keyboard status bits
#define KEYBOARD_STATUS_OUT_BUFFER  0x01
#define KEYBOARD_STATUS_IN_BUFFER   0x02
#define KEYBOARD_STATUS_SYSTEM      0x04
#define KEYBOARD_STATUS_GATE_A20    0x20
#define KEYBOARD_STATUS_TRANS_TIMEOUT 0x40
#define KEYBOARD_STATUS_PARITY_TIMEOUT 0x80

// PS/2 controller commands
#define KEYBOARD_CMD_READ_CONFIG  0x20
#define KEYBOARD_CMD_WRITE_CONFIG 0x60
#define KEYBOARD_CMD_DISABLE_PORT1 0xAD
#define KEYBOARD_CMD_ENABLE_PORT1  0xAE

// Key codes
#define KEY_ESCAPE       0x01
#define KEY_1            0x02
#define KEY_2            0x03
#define KEY_3            0x04
#define KEY_4            0x05
#define KEY_5            0x06
#define KEY_6            0x07
#define KEY_7            0x08
#define KEY_8            0x09
#define KEY_9            0x0A
#define KEY_0            0x0B
#define KEY_MINUS        0x0C
#define KEY_EQUALS       0x0D
#define KEY_BACKSPACE    0x0E
#define KEY_TAB          0x0F
#define KEY_Q            0x10
#define KEY_W            0x11
#define KEY_E            0x12
#define KEY_R            0x13
#define KEY_T            0x14
#define KEY_Y            0x15
#define KEY_U            0x16
#define KEY_I            0x17
#define KEY_O            0x18
#define KEY_P            0x19
#define KEY_LBRACKET     0x1A
#define KEY_RBRACKET     0x1B
#define KEY_ENTER        0x1C
#define KEY_LCTRL        0x1D
#define KEY_A            0x1E
#define KEY_S            0x1F
#define KEY_D            0x20
#define KEY_F            0x21
#define KEY_G            0x22
#define KEY_H            0x23
#define KEY_J            0x24
#define KEY_K            0x25
#define KEY_L            0x26
#define KEY_SEMICOLON    0x27
#define KEY_QUOTE        0x28
#define KEY_BACKQUOTE    0x29
#define KEY_LSHIFT       0x2A
#define KEY_BACKSLASH    0x2B
#define KEY_Z            0x2C
#define KEY_X            0x2D
#define KEY_C            0x2E
#define KEY_V            0x2F
#define KEY_B            0x30
#define KEY_N            0x31
#define KEY_M            0x32
#define KEY_COMMA        0x33
#define KEY_PERIOD       0x34
#define KEY_SLASH        0x35
#define KEY_RSHIFT       0x36
#define KEY_KP_MULTIPLY  0x37
#define KEY_RALT         0x38
#define KEY_SPACE        0x39
#define KEY_CAPSLOCK     0x3A
#define KEY_F1           0x3B
#define KEY_F2           0x3C
#define KEY_F3           0x3D
#define KEY_F4           0x3E
#define KEY_F5           0x3F
#define KEY_F6           0x40
#define KEY_F7           0x41
#define KEY_F8           0x42
#define KEY_F9           0x43
#define KEY_F10          0x44
#define KEY_F11          0x57
#define KEY_F12          0x58

// Key state tracking
static uint8_t g_keyboard_state[256];
static uint8_t g_last_scancode = 0;
static int g_keyboard_shift = 0;
static int g_keyboard_ctrl = 0;
static int g_keyboard_alt = 0;

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

// Convert scancode to ASCII character
static char scancode_to_ascii(uint8_t scancode) {
    static const char unshifted[] = {
        0, 0x01, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
        '-', '=', 0x08, '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
        '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
        '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
        0, '*', 0, 0x38, ' ', 0x3A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    
    static const char shifted[] = {
        0, 0x01, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
        '_', '+', 0x08, '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
        '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
        '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
        0, '*', 0, 0x38, ' ', 0x3A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    
    if (scancode >= 128) {
        return 0;  // Ignore release codes
    }
    
    if (scancode < sizeof(unshifted)) {
        if (g_keyboard_shift) {
            return shifted[scancode];
        }
        return unshifted[scancode];
    }
    return 0;
}

// Handle keyboard scancode
void keyboard_handle_scancode(uint8_t scancode) {
    int is_press = !(scancode & 0x80);
    scancode &= 0x7F;
    
    // Update modifier keys
    if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
        g_keyboard_shift = is_press;
    }
    if (scancode == KEY_LCTRL) {
        g_keyboard_ctrl = is_press;
    }
    if (scancode == KEY_RALT) {
        g_keyboard_alt = is_press;
    }
    
    // Track key state
    int prev_state = g_keyboard_state[scancode];
    g_keyboard_state[scancode] = is_press;
    
    // Create event for key press
    if (is_press && !prev_state) {
        event_t event;
        event.type = EVENT_KEY_DOWN;
        event.key_code = scancode;
        gui_queue_event(&event);
    } else if (!is_press && prev_state) {
        event_t event;
        event.type = EVENT_KEY_UP;
        event.key_code = scancode;
        gui_queue_event(&event);
    }
    
    g_last_scancode = scancode | (is_press ? 0 : 0x80);
}

// Keyboard interrupt handler
void keyboard_interrupt_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    keyboard_handle_scancode(scancode);
}

// Get current key
int keyboard_get_key() {
    return g_last_scancode;
}

// Check if key is pressed
int keyboard_key_pressed(int key) {
    if (key >= 256 || key < 0) return 0;
    return g_keyboard_state[key & 0x7F];
}

// Update modifier state from scancode
void keyboard_update_modifiers(uint8_t scancode, int pressed) {
    scancode &= 0x7F;
    
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            g_keyboard_shift = pressed;
            break;
        case KEY_LCTRL:
            g_keyboard_ctrl = pressed;
            break;
        case KEY_RALT:
            g_keyboard_alt = pressed;
            break;
    }
}

// Get shift state
int keyboard_get_shift() {
    return g_keyboard_shift;
}

// Get ctrl state
int keyboard_get_ctrl() {
    return g_keyboard_ctrl;
}

// Get alt state
int keyboard_get_alt() {
    return g_keyboard_alt;
}

// Initialize keyboard
void keyboard_init() {
    // Clear state
    for (int i = 0; i < 256; i++) {
        g_keyboard_state[i] = 0;
    }
    g_last_scancode = 0;
    g_keyboard_shift = 0;
    g_keyboard_ctrl = 0;
    g_keyboard_alt = 0;
    
    // Clear keyboard buffer
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUT_BUFFER) {
        inb(KEYBOARD_DATA_PORT);
    }
}

