#include "gui.h"

// Hit test return values
#define HTCAPTION      0x02
#define HTCLIENT       0x01
#define HTTOP          0x03
#define HTBOTTOM       0x06
#define HTLEFT         0x0A
#define HTRIGHT        0x0B
#define HTTOPLEFT      0x0D
#define HTTOPRIGHT     0x0E
#define HTBOTTOMLEFT   0x10
#define HTBOTTOMRIGHT  0x11

// Default window colors
#define WINDOW_BG_COLOR     GUI_COLOR_WINDOW_BG
#define WINDOW_TITLE_COLOR  GUI_COLOR_TITLE_BAR
#define WINDOW_BORDER_COLOR GUI_COLOR_BORDER
#define BUTTON_CLOSE_X      12

// Window list (using linked list through base.next)
static window_t* g_windows = 0;
static int g_next_window_id = 1;

// Forward declarations
static int hit_test_border(window_t* win, int x, int y);
static void window_widget_draw(widget_t* widget);
static void window_widget_handle_event(widget_t* widget, event_t* event);

// Create a new window
window_t* window_create(int x, int y, int width, int height, const char* title) {
    window_t* win = (window_t*)malloc(sizeof(window_t));
    if (!win) return 0;
    
    // Initialize window structure
    win->id = g_next_window_id++;
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->min_width = 100;
    win->min_height = 60;
    win->drag_offset_x = 0;
    win->drag_offset_y = 0;
    win->is_dragging = 0;
    win->is_resizing = 0;
    win->resize_dir = 0;
    win->is_minimized = 0;
    win->is_maximized = 0;
    win->saved_x = x;
    win->saved_y = y;
    win->saved_w = width;
    win->saved_h = height;
    win->flags = WINDOW_FLAG_HAS_CLOSE | WINDOW_FLAG_HAS_MINIMIZE | WINDOW_FLAG_HAS_MAXIMIZE;
    win->bg_color = WINDOW_BG_COLOR;
    win->border_color = WINDOW_BORDER_COLOR;
    win->title = title;
    win->on_close = 0;
    win->on_click = 0;
    win->content = 0;
    win->parent = 0;
    win->base.type = WIDGET_WINDOW;
    win->base.x = x;
    win->base.y = y;
    win->base.width = width;
    win->base.height = height;
    win->base.next = 0;
    win->base.draw = window_widget_draw;
    win->base.handle_event = window_widget_handle_event;
    win->base.data = 0;
    
    // Add to window list (at front for top-most)
    win->base.next = (widget_t*)g_windows;
    g_windows = win;
    
    // Bring to front
    window_bring_to_front(win);
    
    // Request redraw
    gui.needs_redraw = 1;
    
    return win;
}

// Destroy a window
void window_destroy(window_t* win) {
    if (!win) return;
    
    // Remove from window list
    window_t** ptr = &g_windows;
    while (*ptr && *ptr != win) {
        ptr = (window_t**)&(*ptr)->base.next;
    }
    if (*ptr) {
        *ptr = (window_t*)win->base.next;
    }
    
    // Free content if any
    if (win->content) {
        widget_destroy(win->content);
    }
    
    // Free window
    free(win);
    
    // Request redraw
    gui.needs_redraw = 1;
}

// Move a window
void window_move(window_t* win, int x, int y) {
    if (!win) return;
    
    // Update position
    win->x = x;
    win->y = y;
    
    // Request redraw
    gui.needs_redraw = 1;
}

// Resize a window
void window_resize(window_t* win, int width, int height) {
    if (!win) return;
    
    // Enforce minimum size
    if (width < win->min_width) width = win->min_width;
    if (height < win->min_height) height = win->min_height;
    
    // Update size
    win->width = width;
    win->height = height;
    
    // Request redraw
    gui.needs_redraw = 1;
}

// Set window title
void window_set_title(window_t* win, const char* title) {
    if (!win) return;
    win->title = title;
    gui.needs_redraw = 1;
}

// Bring window to front
void window_bring_to_front(window_t* win) {
    if (!win) return;
    
    // Remove from current position
    window_t** ptr = &g_windows;
    while (*ptr && *ptr != win) {
        ptr = (window_t**)&(*ptr)->base.next;
    }
    if (*ptr) {
        *ptr = (window_t*)win->base.next;
    }
    
    // Add to front
    win->base.next = (widget_t*)g_windows;
    g_windows = win;
    
    // Set as active
    gui.active_window = win;
    
    // Request redraw
    gui.needs_redraw = 1;
}

// Minimize window
void window_minimize(window_t* win) {
    if (!win) return;
    win->is_minimized = 1;
    gui.needs_redraw = 1;
}

// Restore window
void window_restore(window_t* win) {
    if (!win) return;
    win->is_minimized = 0;
    gui.needs_redraw = 1;
}

// Toggle maximize
void window_toggle_maximize(window_t* win) {
    if (!win) return;
    
    if (win->is_maximized) {
        // Restore
        win->x = win->saved_x;
        win->y = win->saved_y;
        win->width = win->saved_w;
        win->height = win->saved_h;
        win->is_maximized = 0;
    } else {
        // Save and maximize
        win->saved_x = win->x;
        win->saved_y = win->y;
        win->saved_w = win->width;
        win->saved_h = win->height;
        win->x = 0;
        win->y = 0;
        win->width = gui.width;
        win->height = gui.height - gui.taskbar_height;
        win->is_maximized = 1;
    }
    
    gui.needs_redraw = 1;
}

// Close window
void window_close(window_t* win) {
    if (!win) return;
    
    // Call close handler if set
    if (win->on_close) {
        win->on_close(win);
    }
    
    window_destroy(win);
}

// Get window at position (top-most first)
window_t* window_at(int x, int y) {
    // Search from front (top-most) to back
    for (window_t* win = g_windows; win; win = (window_t*)win->base.next) {
        if (win->is_minimized) continue;
        
        if (x >= win->x && x < win->x + win->width &&
            y >= win->y && y < win->y + win->height) {
            return win;
        }
    }
    return 0;
}

// Hit test for borders/resize handles
static int hit_test_border(window_t* win, int x, int y) {
    if (win->is_maximized) return 0;
    if (!(win->flags & WINDOW_FLAG_RESIZABLE)) return 0;
    
    // Top border
    if (y >= win->y && y < win->y + RESIZE_HANDLE) {
        if (x >= win->x && x < win->x + win->width) return HTTOP;
    }
    
    // Bottom border
    if (y >= win->y + win->height - RESIZE_HANDLE && y < win->y + win->height) {
        if (x >= win->x && x < win->x + win->width) return HTBOTTOM;
    }
    
    // Left border
    if (x >= win->x && x < win->x + RESIZE_HANDLE) {
        if (y >= win->y && y < win->y + win->height) return HTLEFT;
    }
    
    // Right border
    if (x >= win->x + win->width - RESIZE_HANDLE && x < win->x + win->width) {
        if (y >= win->y && y < win->y + win->height) return HTRIGHT;
    }
    
    return HTCLIENT;
}

// Draw window frame (title bar and borders)
static void draw_window_frame(window_t* win) {
    // Draw title bar
    fill_rect(win->x, win->y, win->width, WINDOW_TITLE_HEIGHT, 
              win->is_maximized ? WINDOW_TITLE_COLOR : GUI_COLOR_TITLE_BAR);
    
    // Draw title text
    if (win->title) {
        int text_x = win->x + 4;
        draw_string(text_x, win->y + 6, win->title, GUI_COLOR_WHITE, GUI_COLOR_TITLE_BAR);
    }
    
    // Draw close button
    if (win->flags & WINDOW_FLAG_HAS_CLOSE) {
        int btn_x = win->x + win->width - BUTTON_CLOSE_X - 8;
        int btn_y = win->y + 4;
        
        // Close button background
        fill_rect(btn_x, btn_y, 16, 16, GUI_COLOR_LIGHT_GRAY);
        
        // Close X mark
        draw_line(btn_x + 4, btn_y + 4, btn_x + 12, btn_y + 12, GUI_COLOR_RED);
        draw_line(btn_x + 12, btn_y + 4, btn_x + 4, btn_y + 12, GUI_COLOR_RED);
        
        // Border
        draw_rect(btn_x, btn_y, 16, 16, GUI_COLOR_DARK_GRAY);
    }
    
    // Draw minimize button
    if (win->flags & WINDOW_FLAG_HAS_MINIMIZE) {
        int btn_x = win->x + win->width - BUTTON_CLOSE_X - 28;
        int btn_y = win->y + 4;
        
        fill_rect(btn_x, btn_y, 16, 16, GUI_COLOR_LIGHT_GRAY);
        draw_line(btn_x + 4, btn_y + 8, btn_x + 12, btn_y + 8, GUI_COLOR_BLACK);
        draw_rect(btn_x, btn_y, 16, 16, GUI_COLOR_DARK_GRAY);
    }
    
    // Draw maximize button
    if (win->flags & WINDOW_FLAG_HAS_MAXIMIZE) {
        int btn_x = win->x + win->width - BUTTON_CLOSE_X - 48;
        int btn_y = win->y + 4;
        
        fill_rect(btn_x, btn_y, 16, 16, GUI_COLOR_LIGHT_GRAY);
        
        if (win->is_maximized) {
            // Restore icon
            fill_rect(btn_x + 4, btn_y + 6, 8, 6, GUI_COLOR_BLACK);
            draw_rect(btn_x + 3, btn_y + 5, 10, 8, GUI_COLOR_BLACK);
        } else {
            // Maximize icon
            fill_rect(btn_x + 5, btn_y + 5, 8, 8, GUI_COLOR_BLACK);
        }
        draw_rect(btn_x, btn_y, 16, 16, GUI_COLOR_DARK_GRAY);
    }
    
    // Draw border
    if (!win->is_maximized) {
        draw_rect(win->x, win->y, win->width, win->height, win->border_color);
    }
}

// Draw window content area
static void draw_window_content(window_t* win) {
    // Draw client area background
    int client_x = win->x + 1;
    int client_y = win->y + WINDOW_TITLE_HEIGHT;
    int client_w = win->width - 2;
    int client_h = win->height - WINDOW_TITLE_HEIGHT - 1;
    
    if (client_w > 0 && client_h > 0) {
        fill_rect(client_x, client_y, client_w, client_h, win->bg_color);
    }
}

// Draw a window
void window_draw(window_t* win) {
    if (!win || win->is_minimized) return;
    
    // Draw frame first
    draw_window_frame(win);
    
    // Draw content
    draw_window_content(win);
    
    // Draw widgets if any
    if (win->content) {
        widget_draw(win->content);
    }
}

// Draw all windows
void windows_draw_all() {
    // Draw from back to front (reverse order of linked list)
    // We need to build a list first, then draw in reverse
    window_t* list[64];
    int count = 0;
    
    for (window_t* win = g_windows; win && count < 64; win = (window_t*)win->base.next) {
        list[count++] = win;
    }
    
    // Draw from back to front
    for (int i = count - 1; i >= 0; i--) {
        window_t* win = list[i];
        if (!win->is_minimized) {
            window_draw(win);
        }
    }
}

// Handle window event
void window_handle_event(window_t* win, event_t* event) {
    if (!win || !event) return;
    
    switch (event->type) {
        case EVENT_MOUSE_DOWN:
            if (event->mouse_button == MOUSE_BUTTON_LEFT) {
                // Check close button
                if (win->flags & WINDOW_FLAG_HAS_CLOSE) {
                    int btn_x = win->x + win->width - BUTTON_CLOSE_X - 8;
                    int btn_y = win->y + 4;
                    if (event->mouse_x >= btn_x && event->mouse_x < btn_x + 16 &&
                        event->mouse_y >= btn_y && event->mouse_y < btn_y + 16) {
                        window_close(win);
                        return;
                    }
                }
                
                // Check minimize button
                if (win->flags & WINDOW_FLAG_HAS_MINIMIZE) {
                    int btn_x = win->x + win->width - BUTTON_CLOSE_X - 28;
                    int btn_y = win->y + 4;
                    if (event->mouse_x >= btn_x && event->mouse_x < btn_x + 16 &&
                        event->mouse_y >= btn_y && event->mouse_y < btn_y + 16) {
                        window_minimize(win);
                        return;
                    }
                }
                
                // Check maximize button
                if (win->flags & WINDOW_FLAG_HAS_MAXIMIZE) {
                    int btn_x = win->x + win->width - BUTTON_CLOSE_X - 48;
                    int btn_y = win->y + 4;
                    if (event->mouse_x >= btn_x && event->mouse_x < btn_x + 16 &&
                        event->mouse_y >= btn_y && event->mouse_y < btn_y + 16) {
                        window_toggle_maximize(win);
                        return;
                    }
                }
                
                // Check if dragging title bar
                if (event->mouse_y >= win->y && event->mouse_y < win->y + WINDOW_TITLE_HEIGHT) {
                    win->is_dragging = 1;
                    win->drag_offset_x = event->mouse_x - win->x;
                    win->drag_offset_y = event->mouse_y - win->y;
                }
                
                // Check if resizing
                int resize_dir = hit_test_border(win, event->mouse_x, event->mouse_y);
                if (resize_dir && (win->flags & WINDOW_FLAG_RESIZABLE)) {
                    win->is_resizing = 1;
                    win->resize_dir = resize_dir;
                    win->drag_offset_x = event->mouse_x;
                    win->drag_offset_y = event->mouse_y;
                }
                
                // Bring to front
                window_bring_to_front(win);
            }
            break;
            
        case EVENT_MOUSE_MOVE:
            if (win->is_dragging) {
                int new_x = event->mouse_x - win->drag_offset_x;
                int new_y = event->mouse_y - win->drag_offset_y;
                
                // Constrain to screen
                if (new_y < 0) new_y = 0;
                if (new_y + WINDOW_TITLE_HEIGHT > gui.height - gui.taskbar_height) {
                    new_y = gui.height - gui.taskbar_height - WINDOW_TITLE_HEIGHT;
                }
                
                window_move(win, new_x, new_y);
            }
            else if (win->is_resizing) {
                int dx = event->mouse_x - win->drag_offset_x;
                int dy = event->mouse_y - win->drag_offset_y;
                
                int new_x = win->x;
                int new_y = win->y;
                int new_w = win->width;
                int new_h = win->height;
                
                switch (win->resize_dir) {
                    case HTTOP:
                        new_y += dy;
                        new_h -= dy;
                        break;
                    case HTBOTTOM:
                        new_h += dy;
                        break;
                    case HTLEFT:
                        new_x += dx;
                        new_w -= dx;
                        break;
                    case HTRIGHT:
                        new_w += dx;
                        break;
                    case HTTOPLEFT:
                        new_x += dx;
                        new_y += dy;
                        new_w -= dx;
                        new_h -= dy;
                        break;
                    case HTTOPRIGHT:
                        new_y += dy;
                        new_w += dx;
                        new_h -= dy;
                        break;
                    case HTBOTTOMLEFT:
                        new_x += dx;
                        new_w -= dx;
                        new_h += dy;
                        break;
                    case HTBOTTOMRIGHT:
                        new_w += dx;
                        new_h += dy;
                        break;
                }
                
                if (new_w >= win->min_width) {
                    win->x = new_x;
                    win->width = new_w;
                }
                if (new_h >= win->min_height) {
                    win->y = new_y;
                    win->height = new_h;
                }
                
                gui.needs_redraw = 1;
            }
            break;
            
        case EVENT_MOUSE_UP:
            win->is_dragging = 0;
            win->is_resizing = 0;
            break;
            
        default:
            break;
    }
}

// Window widget draw function
static void window_widget_draw(widget_t* widget) {
    window_t* win = (window_t*)widget;
    window_draw(win);
}

// Window widget event handler
static void window_widget_handle_event(widget_t* widget, event_t* event) {
    window_t* win = (window_t*)widget;
    window_handle_event(win, event);
}

// Initialize window system
void window_system_init() {
    g_windows = 0;
    g_next_window_id = 1;
}

// Get first window
window_t* window_get_first() {
    return g_windows;
}

// Get next window
window_t* window_get_next(window_t* win) {
    return win ? (window_t*)win->base.next : 0;
}

