/*
 * Framebuffer Driver Header
 */

#ifndef FB_H
#define FB_H

#include <stdint.h>

/* Framebuffer info */
typedef struct {
    uint64_t base;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t depth;
} fb_info_t;

/* Initialize framebuffer */
int fb_init(fb_info_t *fb);

/* Get framebuffer info */
void fb_get_info(fb_info_t *fb);

/* Clear framebuffer */
void fb_clear(uint32_t color);

#endif /* FB_H */

