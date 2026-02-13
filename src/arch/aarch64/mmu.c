/*
 * MMU/Memory Management Implementation
 * ARMv8-A Stage 1 MMU with 4KB granule
 */

#include "mmu.h"

/* Page table entry attributes */
#define PTE_VALID       (1ULL << 0)
#define PTE_TABLE       (1ULL << 1)
#define PTE_AF          (1ULL << 10)
#define PTE_SH_INNER    (3ULL << 8)
#define PTE_SH_OUTER    (2ULL << 8)
#define PTE_AP_RW       (0ULL << 6)
#define PTE_AP_RO       (2ULL << 6)
#define PTE_NORMAL      (3ULL << 2)
#define PTE_DEVICE      (0ULL << 2)
#define PTE_XN          (1ULL << 54)

/* Translation table base (aligned to 4KB) */
static uint64_t tt_base[512] __attribute__((aligned(4096)));
static uint64_t l2_table[512] __attribute__((aligned(4096)));

/* MAIR register encoding */
#define MAIR_NORMAL_WB      0xFF
#define MAIR_DEVICE_nGnRnE 0x00

/* Create a page table entry */
static uint64_t make_pte(uint64_t phys, uint64_t attrs) {
    return (phys & 0x0000FFFFFFFFFFFFULL) | attrs;
}

/* Map a single page (4KB) */
static void map_page(uint64_t virt, uint64_t phys, uint64_t attrs) {
    /* Level 1 descriptor index (bits 38-30) */
    uint64_t l1_idx = (virt >> 30) & 0x1FF;
    
    /* Point L1 entry to L2 table */
    tt_base[l1_idx] = make_pte((uint64_t)l2_table, PTE_TABLE | PTE_AF);
    
    /* Level 2 descriptor index (bits 29-21) */
    uint64_t l2_idx = (virt >> 21) & 0x1FF;
    
    /* Set L2 entry */
    l2_table[l2_idx] = make_pte(phys, PTE_VALID | PTE_AF | attrs);
}

/* Initialize MMU */
void mmu_init(void) {
    /* Clear translation tables */
    for (int i = 0; i < 512; i++) {
        tt_base[i] = 0;
        l2_table[i] = 0;
    }
    
    /* Identity map kernel region (0x0 to 0x40000000) - 1GB */
    
    /* Map device memory for peripherals */
    for (uint64_t virt = 0xFE000000; virt < 0x100000000ULL; virt += 0x200000) {
        /* Large page (2MB) for efficiency */
        uint64_t l1_idx = (virt >> 30) & 0x1FF;
        tt_base[l1_idx] = make_pte(virt, PTE_VALID | PTE_AF | PTE_DEVICE | PTE_AP_RW | PTE_XN);
    }
    
    /* Normal memory - DDR at 0x0 (large pages) */
    for (uint64_t phys = 0; phys < 0x40000000; phys += 0x200000) {
        uint64_t l1_idx = (phys >> 30) & 0x1FF;
        tt_base[l1_idx] = make_pte(phys, PTE_VALID | PTE_AF | PTE_NORMAL | PTE_AP_RW | PTE_SH_INNER);
    }
    
    /* Set MAIR */
    uint64_t mair = (MAIR_NORMAL_WB << 0) | (MAIR_DEVICE_nGnRnE << 8);
    __asm__ volatile ("msr mair_el1, %0" : : "r" (mair));
    
    /* Set TTBR0 */
    __asm__ volatile ("msr ttbr0_el1, %0" : : "r" (tt_base));
    
    /* Configure TCR */
    uint64_t tcr = (39 << 0) |    /* T0SZ = 48-bit VA space */
                   (5 << 16) |    /* TG0 = 4KB granules */
                   (3 << 8)  |    /* SH0 = Inner shareable */
                   (1 << 6)  |    /* ORGN0 = Normal, WB RA */
                   (1 << 4)  |    /* IRGN0 = Normal, WB RA */
                   (0 << 32);     /* IPS = Default */
    
    __asm__ volatile ("msr tcr_el1, %0" : : "r" (tcr));
    
    /* Enable MMU */
    uint64_t sctlr;
    __asm__ volatile (
        "mrs %0, sctlr_el1\n"
        "orr %0, %0, #1\n"
        "msr sctlr_el1, %0\n"
        : "=r" (sctlr)
    );
}

/* Enable MMU (assumes MMU already initialized) */
void mmu_enable(void) {
    uint64_t val;
    __asm__ volatile (
        "mrs %0, sctlr_el1\n"
        "orr %0, %0, #1\n"
        "msr sctlr_el1, %0\n"
        "dsb ish\n"
        "isb\n"
        : "=r" (val)
    );
}

/* Get current TTBR0 value */
uint64_t mmu_get_ttbr(void) {
    uint64_t val;
    __asm__ volatile ("mrs %0, ttbr0_el1" : "=r" (val));
    return val;
}

