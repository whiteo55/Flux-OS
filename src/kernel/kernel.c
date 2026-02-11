#include <stdint.h>
#include <stddef.h>

// Include our libc compatibility layer
#include "../libc_compat.h"

// Forward declaration of GUI functions
extern void gui_init(int width, int height, void* fb, int pitch);
extern void gui_run();
extern void gui_shutdown();
extern void gui_create_desktop();

// Graphics globals (defined in gfx.c)
extern uint32_t* framebuffer;
extern int screen_width;
extern int screen_height;
extern int pitch;

// Graphics function declarations
extern void set_pixel(int x, int y, uint32_t color);
extern void fill_rect(int x, int y, int width, int height, uint32_t color);
extern void draw_rect(int x, int y, int width, int height, uint32_t color);
extern void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
extern void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
extern void draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg);
extern void clear_screen(uint32_t color);

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

// VGA text mode buffer
static volatile uint16_t* vga_buf = (volatile uint16_t*)0xB8000;

// Port I/O helpers
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Serial port for debugging
#define SERIAL_PORT 0x3F8

static void serial_init() {
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x03);
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03);
    outb(SERIAL_PORT + 2, 0xC7);
    outb(SERIAL_PORT + 4, 0x0B);
}

static void serial_write_char(char c) {
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
    outb(SERIAL_PORT, c);
}

static void serial_write(const char* str) {
    while (*str) {
        if (*str == '\n') serial_write_char('\r');
        serial_write_char(*str++);
    }
}

static void serial_write_hex(uint32_t val) {
    char buf[9];
    const char* hex = "0123456789ABCDEF";
    buf[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    serial_write(buf);
}

// VGA helpers
static void vga_write_text(const char* str, int row, uint8_t color) {
    if (row < 0) row = 0;
    if (row >= 25) row = 24;
    int col = 0;
    for (int i = 0; str[i] && col < 80; i++) {
        if (str[i] == '\n') {
            row++;
            col = 0;
            if (row >= 25) break;
        } else {
            vga_buf[row * 80 + col++] = (uint16_t)str[i] | ((uint16_t)color << 8);
        }
    }
}

void kernel_main(uint32_t mb_info, uint32_t mb_magic) {
    // Note: boot.s pushes EBX (info pointer) first, so mb_info is actually EBX
    // And EAX contains the magic. With cdecl, first arg is on stack = EBX
    // mb_magic = EAX (passed in register)
    
    // Initialize heap first
    heap_init();
    
    // Initialize serial
    serial_init();
    serial_write("\nFLUX-OS Starting...\n");
    
    // Clear screen
    vga_clear();
    vga_write_text("FLUX-OS", 0, 0x0A);
    
    // mb_magic is in EAX, mb_info is the pushed EBX value
    serial_write("Magic (EAX): ");
    serial_write_hex(mb_magic);
    serial_write("\n");
    serial_write("Info ptr (EBX): ");
    serial_write_hex(mb_info);
    serial_write("\n");
    
    // The multiboot magic should be in EAX, but we need to get it from the register
    uint32_t magic;
    __asm__ volatile ("mov %%eax, %0" : "=r"(magic));
    
    serial_write("Real magic: ");
    serial_write_hex(magic);
    serial_write("\n");
    
    if (magic != 0x2BADB002) {
        vga_write_text("No Multiboot!", 2, 0x0C);
        serial_write("ERROR: No Multiboot!\n");
        goto text_mode;
    }
    
    vga_write_text("Multiboot OK", 2, 0x0A);
    serial_write("Multiboot OK\n");
    
    // Parse multiboot info safely
    uint32_t flags = *(uint32_t*)mb_info;
    serial_write("Flags: ");
    serial_write_hex(flags);
    serial_write("\n");
    
    uint32_t fb_addr = 0;
    uint32_t fb_width = 0;
    uint32_t fb_height = 0;
    uint32_t fb_pitch = 0;
    int graphics_mode = 0;
    
    // Check for framebuffer info (bit 12)
    if ((flags >> 12) & 1) {
        vga_write_text("Checking FB info...", 3, 0x0B);
        serial_write("Checking FB info...\n");
        
        // Framebuffer info is at offset 48 in multiboot info
        uint32_t fb_type = *(uint32_t*)(mb_info + 56);
        uint32_t fb_w = *(uint32_t*)(mb_info + 60);
        uint32_t fb_h = *(uint32_t*)(mb_info + 64);
        uint32_t fb_p = *(uint32_t*)(mb_info + 68);
        uint32_t fb_a = *(uint32_t*)(mb_info + 72);
        
        serial_write("FB type: ");
        serial_write_hex(fb_type);
        serial_write("\n");
        serial_write("FB addr: ");
        serial_write_hex(fb_a);
        serial_write("\n");
        
        if (fb_type == 1 && fb_a != 0) {
            fb_addr = fb_a;
            fb_width = fb_w;
            fb_height = fb_h;
            fb_pitch = fb_p;
            graphics_mode = 1;
            
            vga_write_text("FB: OK", 4, 0x0A);
            serial_write("Using Multiboot FB\n");
        }
    }
    
    // If no framebuffer, try VGA 13h
    if (!graphics_mode) {
        vga_write_text("Trying VGA 13h...", 5, 0x0B);
        serial_write("Trying VGA 13h...\n");
        
        // Set VGA mode 13h
        __asm__ volatile (
            "mov $0x13, %%ax\n"
            "int $0x10\n"
            : : : "ax"
        );
        
        fb_addr = 0xA0000;
        fb_width = 320;
        fb_height = 200;
        fb_pitch = 320;
        graphics_mode = 1;
        
        vga_write_text("VGA 13h active", 6, 0x0A);
        serial_write("VGA 13h active\n");
    }
    
    // Set global graphics state
    framebuffer = (uint32_t*)fb_addr;
    screen_width = fb_width;
    screen_height = fb_height;
    pitch = fb_pitch;
    
    // Show info
    char info[64];
    vga_write_text("Resolution: ", 8, 0x07);
    serial_write("Resolution: ");
    snprintf(info, sizeof(info), "%dx%d", (int)fb_width, (int)fb_height);
    vga_write_text(info, 8, 0x07);
    serial_write(info);
    serial_write("\n");
    
    vga_write_text("Initializing GUI...", 10, 0x0A);
    serial_write("Initializing GUI...\n");
    
    // Initialize GUI
    gui_init(screen_width, screen_height, framebuffer, pitch);
    gui_create_desktop();
    
    vga_write_text("GUI Running! Press ESC.", 12, 0x0A);
    serial_write("GUI Running!\n");
    
    // Run GUI
    gui_run();
    
text_mode:
    // Fallback to text mode
    vga_clear();
    vga_write_text("FLUX-OS Text Mode", 0, 0x0A);
    vga_write_text("GUI not available", 2, 0x07);
    serial_write("Text mode only\n");
    
    // Idle loop
    while (1) __asm__("hlt");
}

