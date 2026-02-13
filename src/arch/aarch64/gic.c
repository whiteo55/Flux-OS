/*
 * GICv2 Interrupt Controller Implementation
 */

#include "gic.h"

/* GIC distributor registers */
static volatile uint32_t *gicd = (volatile uint32_t *)GICD_BASE;
static volatile uint32_t *gicc = (volatile uint32_t *)GICC_BASE;

/* GIC register offsets */
#define GICD_CTLR       0x000
#define GICD_TYPER       0x004
#define GICD_IGROUPR     0x080
#define GICD_ISENABLER   0x100
#define GICD_ICENABLER   0x180
#define GICD_IPRIORITYR  0x400
#define GICD_ITARGETSR   0x800
#define GICD_SGIR        0xF00

#define GICC_CTLR        0x000
#define GICC_PMR         0x004
#define GICC_IAR         0x00C
#define GICC_EOIR        0x010

/* Initialize GIC */
void gic_init(void) {
    uint32_t num_irqs;
    
    /* Disable GIC distributor */
    gicd[GICD_CTLR >> 2] = 0;
    
    /* Get number of interrupts supported */
    num_irqs = ((gicd[GICD_TYPER >> 2] & 0x1F) + 1) * 32;
    
    /* Disable all interrupts */
    for (uint32_t i = 32; i < num_irqs; i += 32) {
        gicd[GICD_ICENABLER >> 2] = 0xFFFFFFFF;
    }
    
    /* Set all interrupts to group 0 (secure) */
    for (uint32_t i = 32; i < num_irqs; i += 32) {
        gicd[GICD_IGROUPR >> 2] = 0x00000000;
    }
    
    /* Set priority (all to lowest for now) */
    for (uint32_t i = 32; i < num_irqs; i += 4) {
        gicd[GICD_IPRIORITYR >> 2] = 0xA0A0A0A0;
    }
    
    /* Route all interrupts to CPU 0 */
    for (uint32_t i = 32; i < num_irqs; i += 4) {
        gicd[GICD_ITARGETSR >> 2] = 0x01010101;
    }
    
    /* Enable distributor */
    gicd[GICD_CTLR >> 2] = 1;
    
    /* Enable CPU interface */
    gicc[GICC_CTLR >> 2] = 1;
    
    /* Set priority mask (allow all) */
    gicc[GICC_PMR >> 2] = 0xFF;
}

/* Enable interrupt */
void gic_enable_irq(uint32_t irq) {
    gicd[GICD_ISENABLER >> 2] = (1 << (irq % 32));
}

/* Disable interrupt */
void gic_disable_irq(uint32_t irq) {
    gicd[GICD_ICENABLER >> 2] = (1 << (irq % 32));
}

/* Send software interrupt */
void gic_send_sgi(uint32_t sgi, uint32_t cpu_mask) {
    gicd[GICD_SGIR >> 2] = (cpu_mask << 16) | (sgi & 0x0F);
}

/* Get current IRQ */
uint32_t gic_get_irq(void) {
    return gicc[GICC_IAR >> 2] & 0x3FF;
}

/* End of interrupt */
void gic_end_of_irq(uint32_t irq) {
    gicc[GICC_EOIR >> 2] = irq;
}

