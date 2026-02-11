#!/bin/bash

# Run QEMU with the Flux-OS ISO
# Using i386 for 32-bit kernel with VESA graphics support

echo "Starting Flux-OS in QEMU..."
echo "Note: The GUI requires VBE/VESA graphics mode"
echo "If you see only text output, VBE initialization may have failed"
echo ""
echo "Tip: Press Ctrl+Alt+F to toggle fullscreen"
echo "Tip: Use View menu to show/hide device monitors"
echo ""

# Run with GTK3 display for full QEMU UI with tabs and menus
# This gives you the full QEMU interface with menus, tabs, and device monitors
qemu-system-i386 \
  -cdrom flux-os.iso \
  -boot d \
  -m 512 \
  -vga std \
  -display gtk \
  -show-cursor \
  -no-quit \
  -serial stdio

# Alternative: Run with SDL display (simpler, no full UI)
# qemu-system-i386 \
#   -cdrom flux-os.iso \
#   -boot d \
#   -m 512 \
#   -vga std \
#   -display sdl \
#   -show-cursor \
#   -no-quit \
#   -serial stdio

# Alternative: Run with Cocoa display (macOS, no full UI)
# qemu-system-i386 \
#   -cdrom flux-os.iso \
#   -boot d \
#   -m 512 \
#   -vga std \
#   -display cocoa \
#   -show-cursor \
#   -no-quit \
#   -serial stdio

# Alternative: Run with VNC display (headless/remote)
# qemu-system-i386 \
#   -cdrom flux-os.iso \
#   -boot d \
#   -m 512 \
#   -vga std \
#   -display vnc=:1 \
#   -no-quit

# Alternative: Run with curses (text mode display only)
# qemu-system-i386 \
#   -cdrom flux-os.iso \
#   -boot d \
#   -m 512 \
#   -display curses \
#   -no-quit

