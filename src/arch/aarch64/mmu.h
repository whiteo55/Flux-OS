/*
 * MMU/Memory Management Header
 */

#ifndef MMU_H
#define MMU_H

#include <stdint.h>

/* Initialize MMU with identity mapping */
void mmu_init(void);

/* Enable MMU */
void mmu_enable(void);

/* Get current TTBR0 value */
uint64_t mmu_get_ttbr(void);

/* Map virtual address to physical */
void mmu_map(uint64_t virt, uint64_t phys, uint64_t size, uint32_t flags);

/* Unmap virtual address */
void mmu_unmap(uint64_t virt, uint64_t size);

#endif /* MMU_H */

