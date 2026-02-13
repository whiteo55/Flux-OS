/*
 * Flux-OS AArch64 Kernel Entry Point
 * Raspberry Pi 3/4/500 support
 */

#include <stdint.h>
#include <stddef.h>

/* Board-specific includes */
#include "mailbox.h"
#include "fb.h"
#include "timer.h"
#include "gic.h"
#include "mmu.h"

/* Common GUI includes */
#include "../../gui/gui.h"

/* Architecture-specific definitions */
#define UART0_BASE       0xFE201000  /* PL011 UART */
#define UART1_BASE       0xFE215000  /* Mini UART */

/* UART registers */
#define UART_DR          0x00
#define UART_FR          0x18
#define UART_IBRD        0x24
#define UART_FBRD        0x28
#define UART_LCRH        0x2C
#define UART_CR          0x30
#define UART_IMSC        0x38

/* Framebuffer info from mailbox */
fb_info_t fb_info = {0};

/* Global GUI state */
gui_system_t gui = {0};

/* UART output function */
static void uart_putc(char c) {
    volatile uint32_t *uart = (volatile uint32_t *)UART0_BASE;
    
    /* Wait for transmit FIFO to be empty */
    while (uart[UART_FR >> 2] & (1 << 5)) {
        __asm__ volatile ("nop");
    }
    
    /* Write character */
    uart[UART_DR >> 2] = (uint32_t)c;
}

static void uart_write(const char *str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r');
        }
        uart_putc(*str++);
    }
}

static void uart_write_hex(uint64_t val) {
    char buf[17];
    const char *hex = "0123456789ABCDEF";
    buf[16] = '\0';
    for (int i = 15; i >= 0; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    uart_write(buf);
}

/* Simple delay */
static void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ volatile ("nop");
    }
}

/* Get board revision to detect Pi 500 */
static uint32_t get_board_revision(void) {
    /* Read from mailboxes - simplified */
    return 0; /* TODO: Implement proper detection */
}

/* Initialize UART0 */
static void uart_init(void) {
    volatile uint32_t *uart = (volatile uint32_t *)UART0_BASE;
    
    /* Disable UART */
    uart[UART_CR >> 2] = 0;
    
    /* Set 115200 baud rate (system clock is 48MHz) */
    uart[UART_IBRD >> 2] = 26;
    uart[UART_FBRD >> 2] = 17;
    
    /* 8-bit word, enable FIFO */
    uart[UART_LCRH >> 2] = (1 << 4) | (1 << 5) | (1 << 6);
    
    /* Enable UART, TX and RX */
    uart[UART_CR >> 2] = (1 << 0) | (1 << 8) | (1 << 9);
}

/* Main kernel entry */
void kernel_main(void) {
    uart_init();
    
    uart_write("\r\n");
    uart_write("=======================================\r\n");
    uart_write("Flux-OS AArch64 for Raspberry Pi\r\n");
    uart_write("=======================================\r\n");
    
    /* Get board revision */
    uint32_t rev = get_board_revision();
    uart_write("Board revision: ");
    uart_write_hex(rev);
    uart_write("\r\n");
    
    /* Initialize memory management unit */
    uart_write("Initializing MMU...\r\n");
    mmu_init();
    
    /* Initialize interrupt controller */
    uart_write("Initializing GIC...\r\n");
    gic_init();
    
    /* Initialize system timer */
    uart_write("Initializing timer...\r\n");
    timer_init();
    
    /* Get framebuffer from mailbox */
    uart_write("Requesting framebuffer...\r\n");
    if (mailbox_get_fb(&fb_info) == 0) {
        uart_write("Framebuffer allocated:\r\n");
        uart_write("  Address: ");
        uart_write_hex(fb_info.base);
        uart_write("\r\n");
        uart_write("  Size: ");
        uart_write_hex(fb_info.size);
        uart_write("\r\n");
    } else {
        uart_write("Failed to get framebuffer!\r\n");
    }
    
    /* Initialize GUI with framebuffer */
    if (fb_info.base != 0) {
        uart_write("Initializing GUI...\r\n");
        gui_init(fb_info.width, fb_info.height, (void *)fb_info.base, fb_info.pitch);
        gui_create_desktop();
    }
    
    uart_write("Kernel initialized!\r\n");
    
    /* Run GUI */
    if (gui.initialized) {
        gui_run();
    }
    
    /* Fallback: text output only */
    uart_write("GUI not available, text mode only.\r\n");
    
    /* Idle loop */
    while (1) {
        __asm__ volatile ("wfi");
    }
}

