#!/bin/bash

# Run QEMU with SDL display (alternative to GTK)
# Use this if GTK causes issues

echo "Starting Flux-OS with SDL display..."
echo ""

qemu-system-i386 \
  -cdrom flux-os.iso \
  -boot d \
  -m 512 \
  -vga std \
  -display sdl \
  -show-cursor \
  -no-quit \
  -serial stdio

