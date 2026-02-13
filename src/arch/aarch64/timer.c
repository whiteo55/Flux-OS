/*
 * ARM Generic Timer Implementation
 */

#include "timer.h"

/* Timer registers */
#define CNTPCT_EL0     ((volatile uint64_t *)0xFE00D020)
#define CNTFRQ_EL0     ((volatile uint32_t *)0xFE00D018)
#define CNTP_CTL_EL0   ((volatile uint32_t *)0xFE00D010)
#define CNTP_TVAL_EL0  ((volatile uint32_t *)0xFE00D018)

/* Timer frequency (set by firmware, usually 54MHz on Pi) */
static uint32_t timer_freq = 54000000;

/* Initialize system timer */
void timer_init(void) {
    /* Read timer frequency */
    timer_freq = *CNTFRQ_EL0;
}

/* Get current timer tick */
uint64_t timer_get_ticks(void) {
    return *CNTPCT_EL0;
}

/* Delay for specified microseconds */
void delay_us(uint32_t us) {
    uint64_t start = timer_get_ticks();
    uint64_t delay = (uint64_t)us * (timer_freq / 1000000);
    
    while (timer_get_ticks() - start < delay) {
        __asm__ volatile ("nop");
    }
}

/* Delay for specified milliseconds */
void delay_ms(uint32_t ms) {
    uint64_t start = timer_get_ticks();
    uint64_t delay = (uint64_t)ms * (timer_freq / 1000);
    
    while (timer_get_ticks() - start < delay) {
        __asm__ volatile ("nop");
    }
}

