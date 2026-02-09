#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include <stddef.h>

// GUI Common definitions
#define WINDOW_TITLE_HEIGHT 24
#define TASKBAR_HEIGHT 32
#define BUTTON_HEIGHT 24
#define BORDER_SIZE 1
#define RESIZE_HANDLE 8

// GUI Colors (ARGB format)
#define GUI_COLOR_BLACK      0xFF000000
#define GUI_COLOR_WHITE      0xFFFFFFFF
#define GUI_COLOR_RED        0xFFFF0000
#define GUI_COLOR_GREEN      0xFF00FF00
#define GUI_COLOR_BLUE       0xFF0000FF
#define GUI_COLOR_GRAY       0xFF808080
#define GUI_COLOR_DARK_GRAY  0xFF404040
#define GUI_COLOR_LIGHT_GRAY 0xFFC0C0C0
#define GUI_COLOR_CYAN       0xFF00FFFF
#define GUI_COLOR_YELLOW     0xFFFFFF00
#define GUI_COLOR_MAGENTA    0xFFFF00FF
#define GUI_COLOR_TITLE_BAR  0xFF2a5f7f
#define GUI_COLOR_TASKBAR    0xFF1a4d6d
#define GUI_COLOR_DESKTOP    0xFF0d3d52
#define GUI_COLOR_WINDOW_BG  0xFFE0E0E0
#define GUI_COLOR_BUTTON     0xFFD0D0D0
#define GUI_COLOR_BUTTON_HOVER 0xFFE8E8E8
#define GUI_COLOR_BORDER     0xFF808080

// Event types
typedef enum {
    EVENT_NONE = 0,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_DOWN,
    EVENT_MOUSE_UP,
    EVENT_MOUSE_CLICK,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_WINDOW_CREATE,
    EVENT_WINDOW_DESTROY,
    EVENT_WINDOW_FOCUS,
    EVENT_WINDOW_BLUR,
    EVENT_WINDOW_MOVE,
    EVENT_BUTTON_CLICK,
    EVENT_REDRAW
} event_type_t;

// Mouse button types
typedef enum {
    MOUSE_BUTTON_LEFT = 0,
    MOUSE_BUTTON_RIGHT = 1,
    MOUSE_BUTTON_MIDDLE = 2
} mouse_button_t;

// Widget types
typedef enum {
    WIDGET_WINDOW = 0,
    WIDGET_BUTTON,
    WIDGET_LABEL,
    WIDGET_TEXTBOX
} widget_type_t;

// Window flags
typedef enum {
    WINDOW_FLAG_RESIZABLE = 1 << 0,
    WINDOW_FLAG_HAS_CLOSE = 1 << 1,
    WINDOW_FLAG_HAS_MINIMIZE = 1 << 2,
    WINDOW_FLAG_HAS_MAXIMIZE = 1 << 3,
    WINDOW_FLAG_MODAL = 1 << 4,
    WINDOW_FLAG_TOPMOST = 1 << 5
} window_flags_t;

// Mouse state structure
typedef struct {
    int x, y;
    int dx, dy;
    int buttons;
    int wheel;
    int is_abs;
} mouse_state_t;

// Keyboard state structure
typedef struct {
    int scancode;
    int key;
    int pressed;
    int shift;
    int ctrl;
    int alt;
} keyboard_state_t;

// Event structure
typedef struct {
    event_type_t type;
    int mouse_x, mouse_y;
    mouse_button_t mouse_button;
    int key_code;
    int window_id;
    int widget_id;
    void* data;
} event_t;

// Forward declaration
struct window;
typedef struct window window_t;

// Widget base structure
typedef struct widget {
    struct widget* next;
    widget_type_t type;
    int x, y;
    int width, height;
    void (*draw)(struct widget*);
    void (*handle_event)(struct widget*, event_t*);
    void* data;
} widget_t;

// Window structure
typedef struct window {
    widget_t base;
    int id;
    int x, y;
    int width, height;
    int min_width, min_height;
    int drag_offset_x, drag_offset_y;
    int is_dragging;
    int is_resizing;
    int resize_dir;
    int is_minimized;
    int is_maximized;
    int saved_x, saved_y, saved_w, saved_h;
    int flags;
    uint32_t bg_color;
    uint32_t border_color;
    const char* title;
    void (*on_close)(struct window*);
    void (*on_click)(struct window*, int, int);
    widget_t* content;
    struct window* parent;
} window_t;

// Button widget structure
typedef struct button {
    widget_t base;
    const char* text;
    uint32_t bg_color;
    uint32_t hover_color;
    int is_pressed;
    void (*on_click)(struct button*);
} button_t;

// GUI system state
typedef struct {
    int initialized;
    int width;
    int height;
    int pitch;
    void* framebuffer;
    
    // Input state
    mouse_state_t mouse;
    keyboard_state_t keyboard;
    
    // Window management
    window_t* windows;
    window_t* active_window;
    window_t* dragging_window;
    window_t* resizing_window;
    int window_counter;
    
    // Desktop
    int taskbar_height;
    int clock_x;
    const char* start_text;
    
    // Event queue
    event_t event_queue[64];
    int event_head;
    int event_tail;
    
    // GUI loop control
    int running;
    int needs_redraw;
} gui_system_t;

// Global GUI system
extern gui_system_t gui;

// GUI initialization
void gui_init(int width, int height, void* fb, int pitch);
void gui_shutdown();

// Event handling
void gui_handle_event(event_t* event);
void gui_queue_event(event_t* event);
int gui_poll_event(event_t* event);

// Mouse input
void mouse_init();
void mouse_handle_packet(uint8_t byte0, uint8_t byte1, uint8_t byte2);
void mouse_enable_streaming();
void mouse_get_position(int* x, int* y);

// Keyboard input
void keyboard_init();
void keyboard_handle_scancode(uint8_t scancode);
int keyboard_get_key();
int keyboard_key_pressed(int key);
int keyboard_get_shift();
int keyboard_get_ctrl();
int keyboard_get_alt();

// Window management
window_t* window_create(int x, int y, int width, int height, const char* title);
void window_destroy(window_t* win);
void window_move(window_t* win, int x, int y);
void window_resize(window_t* win, int width, int height);
void window_set_title(window_t* win, const char* title);
void window_bring_to_front(window_t* win);
void window_minimize(window_t* win);
void window_restore(window_t* win);
void window_toggle_maximize(window_t* win);
void window_close(window_t* win);
void window_draw(window_t* win);
void windows_draw_all();
void window_system_init();
void window_handle_event(window_t* win, event_t* event);
window_t* window_at(int x, int y);

// Widget management
void widget_draw(widget_t* widget);
void widget_destroy(widget_t* widget);

// Button functions
button_t* button_create(int x, int y, int width, int height, const char* text);
void button_destroy(button_t* btn);
void button_set_onclick(button_t* btn, void (*callback)(button_t*));

// Drawing functions (from gfx.h)
extern void set_pixel(int x, int y, uint32_t color);
extern void fill_rect(int x, int y, int width, int height, uint32_t color);
extern void draw_rect(int x, int y, int width, int height, uint32_t color);
extern void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
extern void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
extern void draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg);
extern void clear_screen(uint32_t color);

// Main GUI loop
void gui_run();
void gui_redraw_all();

// Time functions
void gui_update_clock();
void gui_get_time_string(char* buffer, size_t size);

// Registry for keyboard shortcuts
void gui_register_shortcut(int key, int ctrl, void (*callback)());

// Desktop creation
void gui_create_desktop();

// Memory allocation (freestanding)
void* malloc(size_t size);
void free(void* ptr);

// String functions (freestanding)
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);

// Simple snprintf for freestanding
int snprintf(char* buf, size_t size, const char* fmt, ...);

#endif // GUI_H

