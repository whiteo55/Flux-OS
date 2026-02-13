/* Flux-OS AArch64 Linker Script for Raspberry Pi */
/* Based on the Raspberry Pi 3/4 boot requirements */

ENTRY(_start)

/*
 * Pi 3/4 boot requirements:
 * - Kernel must be loaded at 0x80000 (512KB) by the firmware
 * - Or we can use the -fPIE approach with _start at 0x80000
 * - Kernel image header: 0x04000000 (magic for ARM64)
 */

/* Base address for kernel */
BASE_ADDR = 0x80000;

/* Text section at base */
SECTIONS
{
    . = BASE_ADDR;

    .text : {
        *(.text.boot)
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.data.rel.ro)
        *(.data.rel.ro.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    /* Exception vectors must be aligned to 4KB */
    .vectors : {
        *(.vectors)
    }

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /* Discard .note sections that we don't need */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
    }
}

