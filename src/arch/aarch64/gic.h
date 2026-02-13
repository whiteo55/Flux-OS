/*
 * GICv2 Interrupt Controller Header
 */

#ifndef GIC_H
#define GIC_H

#include <stdint.h>

/* GICv2 registers */
#define GICD_BASE       0xFE000000
#define GICC_BASE       0xFE000100

/* Initialize GIC */
void gic_init(void);

/* Enable interrupt */
void gic_enable_irq(uint32_t irq);

/* Disable interrupt */
void gic_disable_irq(uint32_t irq);

/* Send software interrupt */
void gic_send_sgi(uint32_t sgi, uint32_t cpu_mask);

/* Get current IRQ */
uint32_t gic_get_irq(void);

/* End of interrupt */
void gic_end_of_irq(uint32_t irq);

#endif /* GIC_H */

