#!/bin/bash

# Build GUI sources first
echo "Building GUI sources..."

# Bootloader assembly
as --32 src/boot/boot.s -o boot.o

# Kernel compilation - freestanding, no standard headers
gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/graphics -I src/gui \
    -c src/kernel/kernel.c -o kernel.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: Kernel compilation failed."
    exit 1
fi

# Graphics compilation
gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/graphics -I src/gui \
    -c src/graphics/gfx.c -o gfx.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: Graphics compilation failed."
    exit 1
fi

# GUI compilation - freestanding environment
echo "Compiling GUI..."
gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/gui -I src/graphics \
    -c src/gui/desktop.c -o gui_desktop.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: desktop.c compilation failed."
    exit 1
fi

gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/gui -I src/graphics \
    -c src/gui/mouse.c -o gui_mouse.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: mouse.c compilation failed."
    exit 1
fi

gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/gui -I src/graphics \
    -c src/gui/keyboard.c -o gui_keyboard.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: keyboard.c compilation failed."
    exit 1
fi

gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/gui -I src/graphics \
    -c src/gui/window.c -o gui_window.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: window.c compilation failed."
    exit 1
fi

gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/gui -I src/graphics \
    -c src/gui/button.c -o gui_button.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: button.c compilation failed."
    exit 1
fi

gcc -m32 -ffreestanding -fno-stack-protector -fno-pie \
    -I src/gui -I src/graphics \
    -c src/gui/string.c -o gui_string.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: string.c compilation failed."
    exit 1
fi

# Linking
echo "Linking..."
ld -m elf_i386 -T linker.ld -o flux-kernel boot.o kernel.o gfx.o gui_desktop.o gui_mouse.o gui_keyboard.o gui_window.o gui_button.o gui_string.o 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi

# Check for Multiboot 1 (not 2)
echo "Checking Multiboot compliance..."
grub-file --is-x86-multiboot flux-kernel 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: Kernel is not Multiboot compliant."
    exit 1
fi

echo "Multiboot compliant!"

# Create ISO
mkdir -p isodir/boot/grub
cp flux-kernel isodir/boot/flux-kernel

cat > isodir/boot/grub/grub.cfg << 'CFGEOF'
set timeout=0
set default=0

# Don't try graphics mode - let kernel handle it
terminal_output console

menuentry "Flux-OS" {
    multiboot /boot/flux-kernel
    boot
}
CFGEOF

grub-mkrescue -o flux-os.iso isodir 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: ISO creation failed."
    exit 1
fi

# Clean up object files
rm -f boot.o kernel.o gfx.o gui_desktop.o gui_mouse.o gui_keyboard.o gui_window.o gui_button.o gui_string.o

echo ""
echo "========================================"
echo "Build Complete! Run with: ./run_qemu.sh"
echo "========================================"

