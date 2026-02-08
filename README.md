# FLUX-OS

![Version](https://img.shields.io/badge/version-2.3--pre--alpha-orange)
![Language](https://img.shields.io/badge/language-C%20%2B%20Assembly-blue)
![Status](https://img.shields.io/badge/status-Text--Mode-red)
![License](https://img.shields.io/badge/license-Proprietary-teal)

**FLUX-OS** is a 32-bit, hobbyist operating system written from scratch in C and x86 Assembly. It aims to be a "Nothing OS" inspired desktop environment—minimalist, typography-driven, and philosophically bare metal.

---

## 🚀 Current Status

**Version:** 2.3 Pre-Alpha  
**Kernel:** Operational  
**Graphics:** Text Mode Only (VBE Pending)

The system boots successfully via GRUB into a protected-mode kernel. It currently displays diagnostic information via VGA text (80x25). The graphics library (GFX) is implemented but awaiting VESA BIOS Extensions (VBE) initialization to switch to a graphical framebuffer.

### Development Roadmap

- [x] **Phase 1: Bootstrapping** (Complete)
  - Custom Assembly Bootloader (Multiboot v1)
  - Protected Mode Transition
  - GDT and Basic Memory Management
  
- [x] **Phase 2: Kernel Core** (Complete)
  - C Kernel Entry Point
  - VGA Text Driver
  - Multiboot Info Parsing
  
- [ ] **Phase 3: Graphics System** (In Progress)
  - VESA BIOS Extensions (VBE) Driver
  - Framebuffer Memory Mapping
  - Software Rendering Library (GFX)
  
- [ ] **Phase 4: User Interface** (Planned)
  - Desktop Environment (FluxWM)
  - Window Management
  - Taskbar and System Tray
  
- [ ] **Phase 5: Hardware Interaction** (Future)
  - PS/2 Keyboard Driver
  - Serial Mouse Driver
  - PIT/RTC Timer

---

## ✨ Vision

To create a desktop experience that feels like a digital instrument cluster. No bloat. No clutter. Just raw interaction between user and machine.

### Design Philosophy

1. **Minimalism over Convenience:** We prioritize raw code efficiency and resource speed over user hand-holding. The OS should feel like a tool for experts, not a toy for the masses.
2. **Typography as UI:** The interface is driven by a custom bitmap font system. The screen is a canvas for text and geometry, not stock icons and heavy assets.
3. **Bare Metal Truth:** FLUX-OS does not hide the hardware. It exposes the Multiboot memory maps, the framebuffer addresses, and the interrupt descriptor tables to the user (if they dare to look).

---

## 🛠️ Build Instructions

FLUX-OS is built using a Bash-based build system targeting the `i386` architecture.

### Requirements

- **OS:** Linux (Ubuntu 22.04 LTS recommended)
- **Compiler:** GCC (with `i386` multilib support)
- **Assembler:** GNU Assembler (GAS)
- **Linker:** GNU Linker (LD)
- **Bootloader:** GRUB 2 (via `grub-mkrescue`)
- **Emulator:** QEMU (for testing)

### Detailed Build Process

The build process is automated via `build.sh`, which performs the following steps:

1. **Cleaning:** Removes any previous object files or kernel binaries to ensure a clean slate.
2. **Assembly:** Compiles `src/boot/boot.s` into `boot.o` using `as --32`. This creates the Multiboot header and initializes the stack.
3. **Kernel Compilation:** Compiles the C kernel source (`src/kernel/kernel.c`) using `gcc -m32 -ffreestanding`. This ensures no standard library dependencies.
4. **Graphics Compilation:** Compiles the graphics library (`src/graphics/gfx.c`) using the same flags.
5. **Linking:** Links `boot.o`, `kernel.o`, and `gfx.o` using the custom `linker.ld` script. This script defines the memory layout, placing the kernel at `0x100000` (1MB).
6. **ISO Creation:** Copies the resulting `flux-kernel` binary to the `isodir/boot/` directory and uses `grub-mkrescue` to generate a bootable ISO image.

### Quick Start

```bash
# Clone the repository
git clone https://github.com/whiteo55/Flux-OS.git
cd Flux-OS

# Build the Operating System
bash build.sh

# Run in QEMU (Requires X11 Display)
DISPLAY=:1 qemu-system-i386 -m 512 -cdrom flux-os.iso -vga std
```

---

## 📁 Project Structure

The repository is organized to separate low-level hardware assembly from high-level kernel logic and user interface rendering.

```
Flux-OS/
├── src/
│   ├── boot/              # Assembly bootloader (Multiboot header, GDT setup)
│   │   └── boot.s
│   ├── kernel/            # Main kernel logic & VGA text driver
│   │   └── kernel.c
│   └── graphics/          # Software rendering library (GUI, Fonts, Primitives)
│       ├── gfx.c
│       └── gfx.h
├── isodir/                # Bootable ISO filesystem structure
│   └── boot/
│       ├── grub/
│       │   └── grub.cfg   # GRUB Configuration
│       └── flux-kernel    # Compiled kernel binary (copied here during build)
├── build.sh               # Master build script
├── linker.ld              # Memory layout & linker configuration
├── LICENSE.md             # Proprietary License Agreement
└── README.md              # This file
```

### Key Components Explained

- **`boot.s`**: The first code that runs. It sets up the Multiboot header so GRUB recognizes the OS, switches to 32-bit Protected Mode, sets up a stack, and jumps to `kernel_main`.
- **`kernel.c`**: The heart of the OS. It parses the Multiboot info structure provided by GRUB to find memory maps and VBE info. It handles the switch from text to graphics mode.
- **`gfx.c`**: A software rasterizer. Since we don't have a GPU driver yet, all drawing (pixels, lines, rectangles, text) is calculated by the CPU and written directly to the framebuffer.

---

## 🐛 Known Issues

### Critical Bugs

- **VBE Initialization Failure:** The kernel cannot currently retrieve the framebuffer address from GRUB. The `mb_info->vbe_mode_info` pointer is returning NULL. This forces the OS to fall back to VGA text mode (80x25).
- **QEMU Graphics:** Hardcoding framebuffer addresses (e.g., `0xFD000000`) has not yielded visual results. The VBE protocol handshake needs debugging.

### Minor Bugs

- **Memory Layout:** The linker script assumes a 1MB load address. If GRUB loads the kernel higher due to high memory maps, the kernel may panic.
- **Interrupts:** The IDT (Interrupt Descriptor Table) is currently empty. Any interrupt or exception (like a divide by zero) will cause a Triple Fault and reboot the system.

---

## 🔧 Debugging

If you are experiencing build errors or boot failures, follow these steps:

1. **Verify Build Output:**
   Ensure `bash build.sh` completes without errors. Check that `flux-kernel` is an ELF executable using `readelf -h flux-kernel`.

2. **Check Multiboot Compliance:**
   Use `grub-file --is-x86-multiboot flux-kernel`. If it returns `false`, your Multiboot header in `boot.s` is incorrect or misaligned.

3. **QEMU Debugging:**
   Run QEMU with the `-d int,cpu_reset` flag to see low-level CPU behavior and interrupts.

---

## 📜 License

This project and its source code are proprietary software owned by Oliver Lebaigue.

Copyright (c) 2026 Oliver Lebaigue. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are strictly prohibited without the express written permission of Oliver Lebaigue.

See `LICENSE.md` for the full legal text.

---

## 🤝 Acknowledgments

While FLUX-OS is a solo project, it stands on the shoulders of giants:

- **OSDev.org Wiki:** For the invaluable documentation on x86 architecture and Multiboot.
- **GRUB Developers:** For creating a robust bootloader that handles the messy transition from Real Mode to Protected Mode.
- **GNU Project:** For the GCC and Binutils toolchains that make cross-compilation possible.

---
*"The void is not empty. It is full of potential."*