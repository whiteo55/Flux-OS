#include "gui.h"
#include "../graphics/gfx.h"

// Global GUI system
gui_system_t gui;

// Forward declarations
static void draw_taskbar();
static void draw_start_button();
static void draw_clock();
static void handle_mouse_event(event_t* event);
static void handle_keyboard_event(event_t* event);

// Initialize GUI system
void gui_init(int width, int height, void* fb, int pitch) {
    // Initialize state
    gui.initialized = 1;
    gui.width = width;
    gui.height = height;
    gui.pitch = pitch;
    gui.framebuffer = fb;
    
    // Initialize input state
    gui.mouse.x = width / 2;
    gui.mouse.y = height / 2;
    gui.mouse.dx = 0;
    gui.mouse.dy = 0;
    gui.mouse.buttons = 0;
    gui.mouse.wheel = 0;
    gui.mouse.is_abs = 1;
    
    gui.keyboard.scancode = 0;
    gui.keyboard.key = 0;
    gui.keyboard.pressed = 0;
    gui.keyboard.shift = 0;
    gui.keyboard.ctrl = 0;
    gui.keyboard.alt = 0;
    
    // Window management
    gui.windows = 0;
    gui.active_window = 0;
    gui.dragging_window = 0;
    gui.resizing_window = 0;
    gui.window_counter = 0;
    
    // Desktop
    gui.taskbar_height = TASKBAR_HEIGHT;
    gui.clock_x = width - 80;
    gui.start_text = "Start";
    
    // Event queue
    gui.event_head = 0;
    gui.event_tail = 0;
    
    // Loop control
    gui.running = 1;
    gui.needs_redraw = 1;
    
    // Initialize subsystems
    window_system_init();
    keyboard_init();
    
    // Enable mouse
    enable_mouse();
}

// Shutdown GUI
void gui_shutdown() {
    gui.running = 0;
    gui.initialized = 0;
}

// Queue an event
void gui_queue_event(event_t* event) {
    int next = (gui.event_tail + 1) % 64;
    if (next != gui.event_head) {
        gui.event_queue[gui.event_tail] = *event;
        gui.event_tail = next;
    }
}

// Poll for event (non-blocking)
int gui_poll_event(event_t* event) {
    if (gui.event_head == gui.event_tail) {
        return 0;  // No events
    }
    *event = gui.event_queue[gui.event_head];
    gui.event_head = (gui.event_head + 1) % 64;
    return 1;
}

// Handle GUI events
void gui_handle_event(event_t* event) {
    if (!event) return;
    
    switch (event->type) {
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_DOWN:
        case EVENT_MOUSE_UP:
        case EVENT_MOUSE_CLICK:
            handle_mouse_event(event);
            break;
            
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            handle_keyboard_event(event);
            break;
            
        case EVENT_REDRAW:
            gui.needs_redraw = 1;
            break;
            
        default:
            break;
    }
}

// Handle mouse events
static void handle_mouse_event(event_t* event) {
    // Update mouse position
    gui.mouse.x = event->mouse_x;
    gui.mouse.y = event->mouse_y;
    
    // Update mouse buttons
    gui.mouse.buttons = 0;
    if (event->mouse_button == MOUSE_BUTTON_LEFT) {
        gui.mouse.buttons |= 1;
    }
    
    // Check if clicking on taskbar
    if (event->mouse_y >= gui.height - gui.taskbar_height) {
        // Check start button
        if (event->mouse_x < 80 && event->type == EVENT_MOUSE_CLICK) {
            // Toggle start menu
            // start_menu_open = !start_menu_open;
            gui.needs_redraw = 1;
        }
        return;
    }
    
    // Check windows from top-most to bottom
    for (window_t* win = gui.windows; win; win = (window_t*)win->base.next) {
        if (win->is_minimized) continue;
        
        // Check if mouse is over this window
        if (event->mouse_x >= win->x && event->mouse_x < win->x + win->width &&
            event->mouse_y >= win->y && event->mouse_y < win->y + win->height) {
            
            // Send event to window
            window_handle_event(win, event);
            return;
        }
    }
}

// Handle keyboard events
static void handle_keyboard_event(event_t* event) {
    gui.keyboard.scancode = event->key_code;
    gui.keyboard.pressed = (event->type == EVENT_KEY_DOWN);
    
    // Update modifiers
    gui.keyboard.shift = keyboard_get_shift();
    gui.keyboard.ctrl = keyboard_get_ctrl();
    gui.keyboard.alt = keyboard_get_alt();
    
    // Handle shortcuts
    if (event->type == EVENT_KEY_DOWN) {
        // Alt+F4 to close window
        if (gui.keyboard.alt && event->key_code == 0x3B) {  // F4
            if (gui.active_window) {
                window_close(gui.active_window);
            }
        }
        
        // F11 for fullscreen toggle (if maximized)
        if (event->key_code == 0x44 && gui.active_window) {  // F11
            window_toggle_maximize(gui.active_window);
        }
    }
}

// Draw the taskbar
static void draw_taskbar() {
    int y = gui.height - gui.taskbar_height;
    
    // Taskbar background
    fill_rect(0, y, gui.width, gui.taskbar_height, GUI_COLOR_TASKBAR);
    
    // Top border
    draw_line(0, y, gui.width, y, GUI_COLOR_LIGHT_GRAY);
    
    // Draw start button
    draw_start_button();
    
    // Draw clock
    draw_clock();
}

// Draw start button
static void draw_start_button() {
    int x = 5;
    int y = gui.height - gui.taskbar_height + 4;
    int w = 70;
    int h = 24;
    
    // Button background
    fill_rect(x, y, w, h, GUI_COLOR_LIGHT_GRAY);
    
    // Button border
    draw_rect(x, y, w, h, GUI_COLOR_DARK_GRAY);
    
    // Button highlight
    draw_line(x + 1, y, x + w - 2, y, GUI_COLOR_WHITE);
    
    // Start text
    draw_string(x + 8, y + 8, gui.start_text, GUI_COLOR_BLACK, GUI_COLOR_LIGHT_GRAY);
}

// Draw clock
static void draw_clock() {
    char time_str[32];
    gui_get_time_string(time_str, sizeof(time_str));
    
    int x = gui.width - 90;
    int y = gui.height - gui.taskbar_height + 8;
    
    // Background for clock
    fill_rect(x - 5, y - 2, 85, 18, GUI_COLOR_LIGHT_GRAY);
    draw_rect(x - 5, y - 2, 85, 18, GUI_COLOR_DARK_GRAY);
    
    // Time text
    draw_string(x, y, time_str, GUI_COLOR_BLACK, GUI_COLOR_LIGHT_GRAY);
}

// Get time string
void gui_get_time_string(char* buffer, size_t size) {
    // Static time for demo (would use RTC in real kernel)
    snprintf(buffer, size, "12:00");
}

// Draw desktop background
static void draw_desktop() {
    // Desktop background
    fill_rect(0, 0, gui.width, gui.height - gui.taskbar_height, GUI_COLOR_DESKTOP);
}

// Draw all windows
static void draw_windows() {
    windows_draw_all();
}

// Redraw everything
void gui_redraw_all() {
    if (!gui.framebuffer) return;
    
    // Draw desktop
    draw_desktop();
    
    // Draw windows
    draw_windows();
    
    // Draw taskbar
    draw_taskbar();
    
    gui.needs_redraw = 0;
}

// Mouse cursor drawing
static int g_last_mouse_x = -1;
static int g_last_mouse_y = -1;

void draw_mouse_cursor(int x, int y) {
    // Clear old cursor if valid
    if (g_last_mouse_x >= 0 && g_last_mouse_y >= 0) {
        // This would need to restore the background
        // For now, we just redraw everything
    }
    
    // Save current position
    g_last_mouse_x = x;
    g_last_mouse_y = y;
    
    // Draw simple arrow cursor
    uint32_t fg = GUI_COLOR_WHITE;
    uint32_t bg = GUI_COLOR_BLACK;
    
    // Main arrow
    draw_line(x + 8, y, x + 8, y + 16, fg);
    draw_line(x, y + 8, x + 16, y + 8, fg);
    
    // Arrow head
    draw_line(x + 8, y, x, y + 8, fg);
    draw_line(x + 8, y, x + 16, y + 8, fg);
    draw_line(x + 8, y + 8, x, y + 8, fg);
    draw_line(x + 8, y + 8, x + 16, y + 8, fg);
}

// Port I/O helpers (inline assembly)
static inline uint8_t port_inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void port_outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Keyboard and mouse port definitions
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_DATA_PORT 0x60

// Track last mouse button state
static int last_mouse_buttons = 0;
static int g_mouse_packet_byte = 0;
static uint8_t g_mouse_packet[3] = {0};

// Main GUI loop
void gui_run() {
    if (!gui.initialized) return;
    
    event_t event;
    
    // Initialize mouse state
    g_mouse_packet_byte = 0;
    last_mouse_buttons = gui.mouse.buttons;
    
    // Main event loop
    while (gui.running) {
        // Poll for keyboard input
        if (port_inb(KEYBOARD_STATUS_PORT) & 0x01) {
            uint8_t scancode = port_inb(KEYBOARD_DATA_PORT);
            keyboard_handle_scancode(scancode);
        }
        
        // Poll for mouse input
        if (port_inb(MOUSE_STATUS_PORT) & 0x20) {
            uint8_t byte = port_inb(MOUSE_DATA_PORT);
            
            if ((byte & 0x08) == 0) {
                // First byte should have bit 3 set
                g_mouse_packet_byte = 0;
            } else {
                g_mouse_packet[g_mouse_packet_byte++] = byte;
                
                if (g_mouse_packet_byte >= 3) {
                    g_mouse_packet_byte = 0;
                    
                    // Process mouse packet
                    int dx = (int8_t)g_mouse_packet[1];
                    int dy = -(int8_t)g_mouse_packet[2];
                    
                    gui.mouse.x += dx;
                    gui.mouse.y += dy;
                    gui.mouse.buttons = g_mouse_packet[0] & 0x07;
                    
                    // Clamp to screen
                    if (gui.mouse.x < 0) gui.mouse.x = 0;
                    if (gui.mouse.x >= gui.width) gui.mouse.x = gui.width - 1;
                    if (gui.mouse.y < 0) gui.mouse.y = 0;
                    if (gui.mouse.y >= gui.height) gui.mouse.y = gui.height - 1;
                    
                    // Queue mouse move event
                    event.type = EVENT_MOUSE_MOVE;
                    event.mouse_x = gui.mouse.x;
                    event.mouse_y = gui.mouse.y;
                    gui_queue_event(&event);
                    
                    // Check for button clicks
                    for (int i = 0; i < 3; i++) {
                        int was_pressed = (last_mouse_buttons & (1 << i));
                        int is_pressed = (gui.mouse.buttons & (1 << i));
                        
                        if (!was_pressed && is_pressed) {
                            event.type = EVENT_MOUSE_DOWN;
                            event.mouse_x = gui.mouse.x;
                            event.mouse_y = gui.mouse.y;
                            event.mouse_button = (mouse_button_t)i;
                            gui_queue_event(&event);
                        } else if (was_pressed && !is_pressed) {
                            event.type = EVENT_MOUSE_UP;
                            event.mouse_x = gui.mouse.x;
                            event.mouse_y = gui.mouse.y;
                            event.mouse_button = (mouse_button_t)i;
                            gui_queue_event(&event);
                            
                            // Click event
                            event.type = EVENT_MOUSE_CLICK;
                            gui_queue_event(&event);
                        }
                    }
                    last_mouse_buttons = gui.mouse.buttons;
                }
            }
        }
        
        // Process all queued events
        while (gui_poll_event(&event)) {
            gui_handle_event(&event);
        }
        
        // Redraw if needed
        if (gui.needs_redraw) {
            gui_redraw_all();
        }
        
        // Draw mouse cursor
        if (gui.mouse.x >= 0 && gui.mouse.x < gui.width &&
            gui.mouse.y >= 0 && gui.mouse.y < gui.height) {
            draw_mouse_cursor(gui.mouse.x, gui.mouse.y);
        }
        
        // Simple delay
        for (int i = 0; i < 10000; i++) {
            __asm__("nop");
        }
    }
}

// Update clock (call periodically)
void gui_update_clock() {
    if (!gui.framebuffer) return;
    
    // Clear clock area
    int x = gui.width - 90;
    int y = gui.height - gui.taskbar_height + 8;
    fill_rect(x - 5, y - 2, 85, 18, GUI_COLOR_TASKBAR);
    
    // Redraw clock
    draw_clock();
}

// Placeholder for widget draw
void widget_draw(widget_t* widget) {
    if (!widget) return;
    if (widget->draw) {
        widget->draw(widget);
    }
}

// Placeholder for widget destroy
void widget_destroy(widget_t* widget) {
    if (!widget) return;
    
    // Destroy children first
    if (widget->next) {
        widget_destroy(widget->next);
    }
    
    // Call type-specific destroy if any
    // Then free the widget
    free(widget);
}

// Start menu callback
static void on_start_click() {
    // Toggle start menu visibility
    // This would show/hide a popup menu
    gui.needs_redraw = 1;
}

// Create desktop environment
void gui_create_desktop() {
    // Create initial windows
    
    // Welcome window
    window_t* welcome = window_create(
        gui.width / 2 - 200,
        gui.height / 2 - 150,
        400,
        300,
        "Welcome to Flux-OS"
    );
    welcome->flags = WINDOW_FLAG_HAS_CLOSE | WINDOW_FLAG_HAS_MINIMIZE;
    
    // About window
    window_t* about = window_create(
        gui.width / 2 - 150,
        gui.height / 2 - 100,
        300,
        200,
        "About Flux-OS"
    );
    about->flags = WINDOW_FLAG_HAS_CLOSE | WINDOW_FLAG_HAS_MINIMIZE;
}

// Register keyboard shortcut
static void (*g_shortcut_callbacks[256])(void) = {0};
static int g_shortcut_keys[256] = {0};
static int g_shortcut_ctrls[256] = {0};
static int g_shortcut_count = 0;

void gui_register_shortcut(int key, int ctrl, void (*callback)()) {
    if (g_shortcut_count < 256) {
        g_shortcut_keys[g_shortcut_count] = key;
        g_shortcut_ctrls[g_shortcut_count] = ctrl;
        g_shortcut_callbacks[g_shortcut_count] = callback;
        g_shortcut_count++;
    }
}

// Initialize mouse cursor position
void gui_set_mouse_position(int x, int y) {
    gui.mouse.x = x;
    gui.mouse.y = y;
}

