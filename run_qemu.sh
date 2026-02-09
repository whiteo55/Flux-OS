#!/bin/bash

# Run QEMU with the Flux-OS ISO
# Using i386 for 32-bit kernel with VESA graphics support

echo "Starting Flux-OS in QEMU..."
echo "Note: The GUI requires VBE/VESA graphics mode"
echo "If you see only text output, VBE initialization may have failed"
echo ""

# Run with SDL display for graphics
# Enable VESA mode and graphics console
qemu-system-i386 \
  -cdrom flux-os.iso \
  -boot d \
  -m 512 \
  -vga std \
  -display sdl \
  -no-quit \
  -serial stdio

# Alternative: Run with VNC display
# qemu-system-i386 \
#   -cdrom flux-os.iso \
#   -boot d \
#   -m 512 \
#   -vga std \
#   -display vnc=:1 \
#   -no-quit

# Alternative: Run with curses (text mode display)
# qemu-system-i386 \
#   -cdrom flux-os.iso \
#   -boot d \
#   -m 512 \
#   -display curses \
#   -no-quit

