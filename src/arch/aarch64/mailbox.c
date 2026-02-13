/*
 * Raspberry Pi Mailbox Implementation
 * Used for framebuffer and property tag communication
 */

#include "mailbox.h"

/* Mailbox volatile registers */
static volatile uint32_t *mailbox_read_reg  = (volatile uint32_t *)(MAILBOX_BASE + MAILBOX_READ);
static volatile uint32_t *mailbox_status_reg = (volatile uint32_t *)(MAILBOX_BASE + MAILBOX_STATUS);
static volatile uint32_t *mailbox_write_reg  = (volatile uint32_t *)(MAILBOX_BASE + MAILBOX_WRITE);

/* Mailbox buffer (in DRAM, 16-byte aligned) */
static mailbox_buf_t mbox __attribute__((aligned(16)));

/* Initialize mailbox */
void mailbox_init(void) {
    /* Mailbox is initialized by firmware, nothing to do */
}

/* Read from mailbox channel */
uint32_t mailbox_read(uint32_t channel) {
    uint32_t value;
    
    /* Wait for mail to be available */
    while (*mailbox_status_reg & MAILBOX_EMPTY) {
        __asm__ volatile ("nop");
    }
    
    /* Read the mail */
    value = *mailbox_read_reg;
    
    /* Extract channel from lower 4 bits */
    /* Return only if correct channel */
    if ((value & 0xF) == channel) {
        return value & ~0xF;
    }
    
    /* Wrong channel, try again */
    return mailbox_read(channel);
}

/* Write to mailbox channel */
void mailbox_write(uint32_t channel, uint32_t data) {
    /* Wait for write buffer to be available */
    while (*mailbox_status_reg & MAILBOX_FULL) {
        __asm__ volatile ("nop");
    }
    
    /* Write to mailbox (upper 28 bits are data, lower 4 bits are channel) */
    *mailbox_write_reg = (data & ~0xF) | (channel & 0xF);
}

/* Get framebuffer configuration */
int mailbox_get_fb(fb_info_t *fb) {
    /* Setup buffer for framebuffer request */
    mbox.buf_size = sizeof(mbox);
    mbox.code = 0; /* Request */
    
    /* Property tag: allocate buffer */
    mbox.tags[0].tag = TAG_ALLOCATE_BUFFER;
    mbox.tags[0].buf_size = 8;
    mbox.tags[0].req_resp = 8;
    mbox.tags[0].data[0] = 0; /* Alignment */
    mbox.tags[0].data[1] = 0;
    
    /* Property tag: set physical size */
    mbox.tags[1].tag = TAG_SET_PHY_WH;
    mbox.tags[1].buf_size = 8;
    mbox.tags[1].req_resp = 8;
    mbox.tags[1].data[0] = 1920; /* Width */
    mbox.tags[1].data[1] = 1080; /* Height */
    
    /* Property tag: set depth */
    mbox.tags[2].tag = TAG_SET_DEPTH;
    mbox.tags[2].buf_size = 4;
    mbox.tags[2].req_resp = 4;
    mbox.tags[2].data[0] = 32; /* 32-bit */
    
    /* Property tag: get pitch */
    mbox.tags[3].tag = TAG_GET_PITCH;
    mbox.tags[3].buf_size = 4;
    mbox.tags[3].req_resp = 4;
    mbox.tags[3].data[0] = 0;
    
    /* End tag */
    mbox.tags[4].tag = 0;
    mbox.tags[4].buf_size = 0;
    mbox.tags[4].req_resp = 0;
    
    /* Send to mailbox channel 8 (property tags) */
    mailbox_write(MAILBOX_CH_PROP, (uint32_t)&mbox);
    
    /* Wait for response */
    while (1) {
        uint32_t resp = mailbox_read(MAILBOX_CH_PROP);
        
        /* Check if this is our response */
        if (resp == (uint32_t)&mbox) {
            /* Check if successful */
            if (mbox.code == 0x80000000) {
                /* Get framebuffer info */
                fb->base = ((uint64_t)mbox.tags[0].data[1] << 32) | mbox.tags[0].data[0];
                fb->size = mbox.tags[0].buf_size;
                fb->width = mbox.tags[1].data[0];
                fb->height = mbox.tags[1].data[1];
                fb->depth = mbox.tags[2].data[0];
                fb->pitch = mbox.tags[3].data[0];
                return 0;
            }
            return -1;
        }
    }
}

/* Get ARM memory size */
uint32_t mailbox_get_arm_memory(void) {
    mbox.buf_size = sizeof(mbox);
    mbox.code = 0;
    
    mbox.tags[0].tag = TAG_GET_ARM_MEM;
    mbox.tags[0].buf_size = 8;
    mbox.tags[0].req_resp = 8;
    
    mbox.tags[1].tag = 0;
    
    mailbox_write(MAILBOX_CH_PROP, (uint32_t)&mbox);
    
    while (1) {
        uint32_t resp = mailbox_read(MAILBOX_CH_PROP);
        if (resp == (uint32_t)&mbox && mbox.code == 0x80000000) {
            return mbox.tags[0].data[1];
        }
    }
}

/* Get VC memory base */
uint64_t mailbox_get_vc_memory(void) {
    mbox.buf_size = sizeof(mbox);
    mbox.code = 0;
    
    mbox.tags[0].tag = TAG_GET_VC_MEM;
    mbox.tags[0].buf_size = 8;
    mbox.tags[0].req_resp = 8;
    
    mbox.tags[1].tag = 0;
    
    mailbox_write(MAILBOX_CH_PROP, (uint32_t)&mbox);
    
    while (1) {
        uint32_t resp = mailbox_read(MAILBOX_CH_PROP);
        if (resp == (uint32_t)&mbox && mbox.code == 0x80000000) {
            return ((uint64_t)mbox.tags[0].data[1] << 32) | mbox.tags[0].data[0];
        }
    }
}

