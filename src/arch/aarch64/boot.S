/*
 * Flux-OS AArch64 Bootloader for Raspberry Pi 3/4/500
 * 
 * The Pi firmware loads our kernel at 0x80000 and jumps to _start.
 * We need to:
 * 1. Set up EL2 (hypervisor) -> EL1 (kernel) transition
 * 2. Disable MMU and caches initially
 * 3. Set up stack and basic registers
 * 4. Enable MMU and jump to C kernel
 */

.section .text.boot

.global _start
.type _start, @function

_start:
    /* Pi firmware already puts us in EL2 (hypervisor mode) on Pi 3/4 */
    /* On Pi 500 (CM4), we start in EL2 */

    /* Check current exception level */
    mrs x0, CurrentEL
    cmp x0, 0xC          /* EL2 = 0xC, EL1 = 0x4, EL0 = 0x0 */
    b.eq 1f
    b 2f

1:
    /* We're in EL2 - need to drop to EL1 */
    
    /* Set EL1 register state */
    mov x0, #0x3035      /* RNDR, RNDRRS, DIS64, DIS16, 0x30 */
    msr id_aa64pfr0_el1, x0
    
    /* SCTLR_EL1: disable things initially */
    mov x0, #0
    msr SCTLR_EL1, x0
    
    /* Clear DAIF (enable interrupts) */
    msr daifclr, #0xF
    
    /* Set stack pointer (temp, will be re-set in C) */
    ldr x0, =boot_stack_top
    mov sp, x0
    
    /* Enable MMU, caches, and alignment checking */
    ldr x0, =SCTLR_VALUE
    msr SCTLR_EL1, x0
    
    /* Set up exception vectors */
    ldr x0, =vector_table
    msr vbar_el1, x0
    
    /* Switch to EL1 */
    mov x0, #0x3C5       /* EL1t: SP, D, A, I, F enabled */
    msr spsr_el2, x0
    adr x0, 1f
    msr elr_el2, x0
    eret

2:
    /* Already in EL1 or EL0 */
    /* Set stack */
    ldr x0, =boot_stack_top
    mov sp, x0
    
    /* Jump to kernel_main */
    b kernel_main

/* 
 * SCTLR_EL1 configuration
 * Enables: MMU (bit 0), Alignment check (bit 1), Data cache (bit 2),
 * Instruction cache (bit 12), Stack alignment check (bit 21)
 */
SCTLR_VALUE = 0x1C5C7D  /* Standard enable bits */

.align 12
boot_stack_bottom:
    .skip 16384      /* 16KB stack */
boot_stack_top:

/*
 * Exception Vector Table (simplified)
 * Standard ARM64 vectors - must be aligned to 4KB
 */
.section .vectors, "ax"
.global vector_table
.align 12

vector_table:
    /* Current EL with SP0 (synchronous, irq, fiq, serror) */
    b _start           /* Synchronous */
    b _start           /* IRQ */
    b _start           /* FIQ */
    b _start           /* SError */

    /* Current EL with SPx */
    b _start           /* Synchronous */
    b _start           /* IRQ */
    b _start           /* FIQ */
    b _start           /* SError */

    /* Lower EL (AArch64) */
    b _start           /* Synchronous */
    b _start           /* IRQ */
    b _start           /* FIQ */
    b _start           /* SError */

    /* Lower EL (AArch32) - not used */
    b _start           /* Synchronous */
    b _start           /* IRQ */
    b _start           /* FIQ */
    b _start           /* SError */

.size _start, . - _start

