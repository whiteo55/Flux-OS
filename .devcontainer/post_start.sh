#!/bin/bash
set -e

echo "Installing build dependencies..."
apt-get update -qq
apt-get install -y -qq grub-pc-bin xorriso qemu-system-x86 binutils gcc > /dev/null 2>&1

echo "Building Flux-OS ISO..."
cd /workspaces/Flux-OS
bash build.sh

echo "Starting QEMU with Flux-OS..."
bash run_qemu.sh &

echo "Setup complete!"
