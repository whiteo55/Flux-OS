#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stddef.h>

// Color definitions (32-bit ARGB format)
#define COLOR_BLACK     0xFF000000
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_RED       0xFFFF0000
#define COLOR_GREEN     0xFF00FF00
#define COLOR_BLUE      0xFF0000FF
#define COLOR_GRAY      0xFF808080
#define COLOR_DARK_GRAY 0xFF404040
#define COLOR_LIGHT_GRAY 0xFFC0C0C0
#define COLOR_CYAN      0xFF00FFFF
#define COLOR_YELLOW    0xFFFFFF00
#define COLOR_MAGENTA   0xFFFF00FF

// Basic drawing functions
void set_pixel(int x, int y, uint32_t color);
void fill_rect(int x, int y, int width, int height, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
void draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg);
void clear_screen(uint32_t color);

#endif // GFX_H
