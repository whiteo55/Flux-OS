#include "gui.h"

// Forward declarations
static void button_widget_draw(widget_t* widget);
static void button_widget_handle_event(widget_t* widget, event_t* event);

// Create a new button
button_t* button_create(int x, int y, int width, int height, const char* text) {
    button_t* btn = (button_t*)malloc(sizeof(button_t));
    if (!btn) return 0;
    
    // Initialize button
    btn->base.type = WIDGET_BUTTON;
    btn->base.x = x;
    btn->base.y = y;
    btn->base.width = width;
    btn->base.height = height;
    btn->base.next = 0;
    btn->base.draw = button_widget_draw;
    btn->base.handle_event = button_widget_handle_event;
    btn->base.data = btn;  // Point back to self for callbacks
    
    btn->text = text;
    btn->bg_color = GUI_COLOR_BUTTON;
    btn->hover_color = GUI_COLOR_BUTTON_HOVER;
    btn->is_pressed = 0;
    btn->on_click = 0;
    
    return btn;
}

// Destroy a button
void button_destroy(button_t* btn) {
    if (!btn) return;
    free(btn);
}

// Set button click callback
void button_set_onclick(button_t* btn, void (*callback)(button_t*)) {
    if (!btn) return;
    btn->on_click = callback;
}

// Draw button widget
static void button_widget_draw(widget_t* widget) {
    button_t* btn = (button_t*)widget;
    
    // Choose color based on state
    uint32_t bg_color = btn->is_pressed ? GUI_COLOR_DARK_GRAY : 
                       btn->bg_color;
    
    // Draw button background
    fill_rect(btn->base.x, btn->base.y, btn->base.width, btn->base.height, bg_color);
    
    // Draw border
    draw_rect(btn->base.x, btn->base.y, btn->base.width, btn->base.height, GUI_COLOR_DARK_GRAY);
    
    // Draw highlight on top edge
    draw_line(btn->base.x + 1, btn->base.y, 
              btn->base.x + btn->base.width - 2, btn->base.y, 
              GUI_COLOR_WHITE);
    
    // Draw shadow on bottom edge
    draw_line(btn->base.x + 1, btn->base.y + btn->base.height - 1,
              btn->base.x + btn->base.width - 2, btn->base.y + btn->base.height - 1,
              GUI_COLOR_DARK_GRAY);
    
    // Draw text
    if (btn->text) {
        size_t text_len = strlen(btn->text);
        int text_width = text_len * 8;
        int text_x = btn->base.x + (btn->base.width - text_width) / 2;
        int text_y = btn->base.y + (btn->base.height - 7) / 2;
        
        draw_string(text_x, text_y, btn->text, GUI_COLOR_BLACK, bg_color);
    }
}

// Handle button events
static void button_widget_handle_event(widget_t* widget, event_t* event) {
    button_t* btn = (button_t*)widget;
    
    switch (event->type) {
        case EVENT_MOUSE_MOVE:
            // Check if mouse is over button
            if (event->mouse_x >= btn->base.x && 
                event->mouse_x < btn->base.x + btn->base.width &&
                event->mouse_y >= btn->base.y &&
                event->mouse_y < btn->base.y + btn->base.height) {
                // Redraw with hover color
                btn->base.draw(&btn->base);
            }
            break;
            
        case EVENT_MOUSE_DOWN:
            if (event->mouse_button == MOUSE_BUTTON_LEFT) {
                if (event->mouse_x >= btn->base.x && 
                    event->mouse_x < btn->base.x + btn->base.width &&
                    event->mouse_y >= btn->base.y &&
                    event->mouse_y < btn->base.y + btn->base.height) {
                    btn->is_pressed = 1;
                    btn->base.draw(&btn->base);
                }
            }
            break;
            
        case EVENT_MOUSE_UP:
            if (event->mouse_button == MOUSE_BUTTON_LEFT) {
                if (btn->is_pressed) {
                    btn->is_pressed = 0;
                    btn->base.draw(&btn->base);
                    
                    // Check if still over button (was a click)
                    if (event->mouse_x >= btn->base.x && 
                        event->mouse_x < btn->base.x + btn->base.width &&
                        event->mouse_y >= btn->base.y &&
                        event->mouse_y < btn->base.y + btn->base.height) {
                        
                        // Fire click event
                        if (btn->on_click) {
                            btn->on_click(btn);
                        }
                    }
                }
            }
            break;
    }
}

