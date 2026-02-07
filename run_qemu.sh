#!/bin/bash

# Run QEMU with the Flux-OS ISO on the VNC desktop
DISPLAY=:1 qemu-system-x86_64 \
  -cdrom flux-os.iso \
  -boot d \
  -m 512 \
  -display sdl \
  -no-quit
