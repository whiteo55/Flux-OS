/*
 * ARM Generic Timer Header
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Initialize system timer */
void timer_init(void);

/* Get current timer tick */
uint64_t timer_get_ticks(void);

/* Delay for specified microseconds */
void delay_us(uint32_t us);

/* Delay for specified milliseconds */
void delay_ms(uint32_t ms);

#endif /* TIMER_H */

