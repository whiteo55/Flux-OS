/*
 * Framebuffer Driver
 */

#include "fb.h"
#include "mailbox.h"

/* Global framebuffer info */
fb_info_t g_fb = {0};

/* Initialize framebuffer */
int fb_init(fb_info_t *fb) {
    if (mailbox_get_fb(fb) == 0) {
        g_fb = *fb;
        return 0;
    }
    return -1;
}

/* Get framebuffer info */
void fb_get_info(fb_info_t *fb) {
    *fb = g_fb;
}

/* Clear framebuffer */
void fb_clear(uint32_t color) {
    if (g_fb.base == 0) return;
    
    uint32_t *fb = (uint32_t *)g_fb.base;
    uint32_t count = g_fb.size / 4;
    
    for (uint32_t i = 0; i < count; i++) {
        fb[i] = color;
    }
}

