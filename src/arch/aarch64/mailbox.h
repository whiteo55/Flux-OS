/*
 * Raspberry Pi Mailbox Interface
 * Used for framebuffer and property tag communication
 */

#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>
#include <stddef.h>

/* Mailbox channels */
#define MAILBOX_CH_PROP      8   /* Property tags channel */
#define MAILBOX_CH_FB        1   /* Framebuffer channel */

/* Mailbox registers */
#define MAILBOX_BASE         0xFE00B880
#define MAILBOX_READ          0x00
#define MAILBOX_STATUS        0x18
#define MAILBOX_WRITE         0x20

/* Status bits */
#define MAILBOX_FULL          0x80000000
#define MAILBOX_EMPTY         0x40000000

/* Property tags */
#define TAG_GET_FIRMWARE      0x00000001
#define TAG_GET_BOARD_MODEL   0x00010001
#define TAG_GET_BOARD_REV     0x00010002
#define TAG_GET_MAC_ADDR      0x00010003
#define TAG_GET_SERIAL        0x00010004
#define TAG_GET_ARM_MEM       0x00010005
#define TAG_GET_VC_MEM       0x00010006
#define TAG_GET_CLOCKS        0x00010007
#define TAG_GET_POWER         0x00020001
#define TAG_GET_CLOCK_RATE    0x00030002
#define TAG_SET_CLOCK_RATE    0x00038002
#define TAG_GET_VOLTAGE       0x00030003
#define TAG_SET_VOLTAGE       0x00038003
#define TAG_GET_TEMP          0x0003000A
#define TAG_GET_TEMP_MAX      0x0003000B
#define TAG_ALLOCATE_BUFFER   0x00040001
#define TAG_GET_PITCH         0x00040008
#define TAG_SET_PHY_WH        0x00048003
#define TAG_SET_VIR_WH        0x00048005
#define TAG_SET_DEPTH         0x00048007
#define TAG_SET_PIXEL_ORDER   0x00048009
#define TAG_GET_ALPHA_MODE    0x0004800B
#define TAG_SET_ALPHA_MODE    0x0004800C

/* Framebuffer info structure */
typedef struct {
    uint64_t base;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t depth;
} fb_info_t;

/* Mailbox buffer structure */
typedef struct mailbox_buf {
    uint32_t buf_size;
    uint32_t code;
    struct {
        uint32_t tag;
        uint32_t buf_size;
        uint32_t req_resp;
        uint32_t data[32];
    } tags[];
} mailbox_buf_t;

/* Initialize mailbox */
void mailbox_init(void);

/* Read from mailbox */
uint32_t mailbox_read(uint32_t channel);

/* Write to mailbox */
void mailbox_write(uint32_t channel, uint32_t data);

/* Get framebuffer configuration */
int mailbox_get_fb(fb_info_t *fb);

/* Get ARM memory size */
uint32_t mailbox_get_arm_memory(void);

/* Get VC memory base */
uint64_t mailbox_get_vc_memory(void);

#endif

