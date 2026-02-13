# Flux-OS Build Guide

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential grub-common xorriso libncurses5-dev gcc-multilib g++-multilib
```

### Fedora/RHEL
```bash
sudo dnf install @development-tools grub2-tools xorriso ncurses-devel gcc gcc-c++ glibc-devel.i686
```

### For AArch64 (Raspberry Pi) Cross-Compilation
```bash
# Ubuntu/Debian
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu

# Fedora
sudo dnf install aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc-c++ aarch64-linux-gnu-binutils
```

---

## Build Commands

### Default Build (x86 32-bit)

```bash
# Build ISO image (bootable)
./build.sh --iso

# Build raw disk image
./build.sh --img

# Build compressed disk image
./build.sh --img.xz

# Show help
./build.sh
```

### x86 64-bit Build

```bash
ARCH=x86_64 ./build.sh --img
```

### AArch64 (Raspberry Pi) Build

```bash
# Install cross-compiler first (see prerequisites)
ARCH=aarch64 ./build.sh --img

# Compressed
ARCH=aarch64 ./build.sh --img.xz
```

---

## Output Files

| Architecture | Output Type | Filename |
|-------------|------------|----------|
| x86_32 | ISO | `flux-os.iso` |
| x86_32 | Raw img | `flux-kernel` |
| x86_32 | Compressed | `flux-kernel.xz` |
| x86_64 | Raw img | `flux-kernel` |
| aarch64 | Raw img | `kernel8.img` |
| aarch64 | Compressed | `kernel8.img.xz` |

---

## Running the OS

### QEMU (x86)

```bash
# Run with graphics
./run_gui.sh

# Run with SDL
./run_qemu_sdl.sh

# Text-only
./run_qemu.sh
```

### Raspberry Pi

1. Copy `kernel8.img` to SD card boot partition
2. Place `config.txt` in boot partition with:
   ```
   arm_64bit=1
   kernel=kernel8.img
   ```
3. Boot Pi 3/4/500

---

## Troubleshooting

### "args needed" error
Make sure to pass a valid argument:
```bash
./build.sh --iso
```

### "gcc not found" for aarch64
Install cross-compiler:
```bash
sudo apt install gcc-aarch64-linux-gnu
```

### Multiboot compliance warning
The kernel still works, but may not boot with some GRUB versions.

---

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `ARCH` | `x86_32` | Target architecture |
| `BOARD` | `generic` | Target board variant |

---

## Clean Build

```bash
# Remove all generated files
rm -f *.o *.img *.iso *.xz kernel8.img flux-kernel
rm -rf isodir
```

