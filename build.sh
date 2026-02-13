#!/bin/bash

# Architecture selection
ARCH="${ARCH:-x86_32}"
BOARD="${BOARD:-generic}"

# Parse arguments
if [ $# -ge 1 ]; then
    BUILD_TYPE="$1"
else
    BUILD_TYPE=""
fi

# Architecture-specific settings
case "$ARCH" in
    x86_32)
        CC="gcc"
        AS="as"
        LD="ld"
        CFLAGS="-m32 -ffreestanding -fno-stack-protector -fno-pie -I src/kernel -I src/graphics -I src/gui"
        ASFLAGS="--32"
        LDFLAGS="-m elf_i386"
        LINKER_SCRIPT="linker.ld"
        BOOT_S="src/boot/boot.s"
        KERNEL_SRC="src/kernel/kernel.c"
        INCLUDE_DIRS="-I src/kernel -I src/graphics -I src/gui"
        ;;
    x86_64)
        CC="gcc"
        AS="as"
        LD="ld"
        CFLAGS="-m64 -ffreestanding -fno-stack-protector -fno-pie -I src/kernel -I src/graphics -I src/gui"
        ASFLAGS="--64"
        LDFLAGS="-m elf_x86_64"
        LINKER_SCRIPT="linker_x86_64.ld"
        BOOT_S="src/boot/boot_x86_64.s"
        KERNEL_SRC="src/kernel/kernel_x86_64.c"
        INCLUDE_DIRS="-I src/kernel -I src/graphics -I src/gui"
        ;;
    aarch64)
        CC="aarch64-linux-gnu-gcc"
        AS="aarch64-linux-gnu-as"
        LD="aarch64-linux-gnu-ld"
        CFLAGS="-m64 -ffreestanding -fno-stack-protector -fno-pie -I src/arch/aarch64 -I src/gui"
        ASFLAGS=""
        LDFLAGS=""
        LINKER_SCRIPT="linker_aarch64_pi.ld"
        BOOT_S="src/arch/aarch64/boot.S"
        KERNEL_SRC="src/arch/aarch64/kernel.c"
        INCLUDE_DIRS="-I src/arch/aarch64 -I src/gui"
        ;;
    *)
        echo "Unknown architecture: $ARCH"
        echo "Supported: x86_32, x86_64, aarch64"
        exit 1
        ;;
esac

# Check if toolchain is available
if ! command -v $CC &> /dev/null; then
    echo "ERROR: $CC not found. Install cross-compiler toolchain."
    if [ "$ARCH" = "aarch64" ]; then
        echo "  Ubuntu/Debian: sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"
        echo "  Fedora: sudo dnf install aarch64-linux-gnu-gcc aarch64-linux-gnu-g++"
    fi
    exit 1
fi

echo "=========================================="
echo "Building Flux-OS for $ARCH (board: $BOARD)"
echo "Compiler: $CC"
echo "=========================================="

# Validate build type (optional for aarch64)
if [ -n "$BUILD_TYPE" ]; then
    if [ "$BUILD_TYPE" != "--img" ] && [ "$BUILD_TYPE" != "--iso" ] && [ "$BUILD_TYPE" != "--img.xz" ]; then
        echo "args needed"
        echo "Usage: ./build.sh [--img|--iso|--img.xz]"
        echo "  Or set ARCH environment variable and build.sh without args"
        exit 1
    fi
fi

# Bootloader assembly
echo "Assembling bootloader..."
$AS $ASFLAGS $BOOT_S -o boot.o 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: Bootloader assembly failed."
    exit 1
fi

# Architecture-specific libc
if [ "$ARCH" = "x86_32" ]; then
    echo "Building libc compatibility layer..."
    $CC $CFLAGS -c src/libc_compat.c -o libc_compat.o 2>&1
    if [ $? -ne 0 ]; then
        echo "ERROR: libc_compat.c compilation failed."
        exit 1
    fi
fi

# Kernel compilation
echo "Compiling kernel..."
$CC $CFLAGS $INCLUDE_DIRS -c $KERNEL_SRC -o kernel.o 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: Kernel compilation failed."
    exit 1
fi

# Architecture-specific compilation
case "$ARCH" in
    x86_32)
        echo "Compiling graphics..."
        $CC $CFLAGS -c src/graphics/gfx.c -o gfx.o 2>&1
        if [ $? -ne 0 ]; then
            echo "ERROR: Graphics compilation failed."
            exit 1
        fi

        echo "Compiling GUI..."
        $CC $CFLAGS -c src/gui/desktop.c -o gui_desktop.o 2>&1
        $CC $CFLAGS -c src/gui/mouse.c -o gui_mouse.o 2>&1
        $CC $CFLAGS -c src/gui/keyboard.c -o gui_keyboard.o 2>&1
        $CC $CFLAGS -c src/gui/window.c -o gui_window.o 2>&1
        $CC $CFLAGS -c src/gui/button.c -o gui_button.o 2>&1
        $CC $CFLAGS -c src/gui/string.c -o gui_string.o 2>&1
        
        OBJECTS="boot.o kernel.o gfx.o gui_desktop.o gui_mouse.o gui_keyboard.o gui_window.o gui_button.o gui_string.o libc_compat.o"
        ;;
    aarch64)
        echo "Compiling arch-specific drivers..."
        $CC $CFLAGS -c src/arch/aarch64/mailbox.c -o mailbox.o 2>&1
        $CC $CFLAGS -c src/arch/aarch64/fb.c -o fb.o 2>&1
        $CC $CFLAGS -c src/arch/aarch64/timer.c -o timer.o 2>&1
        $CC $CFLAGS -c src/arch/aarch64/gic.c -o gic.o 2>&1
        $CC $CFLAGS -c src/arch/aarch64/mmu.c -o mmu.o 2>&1
        
        echo "Compiling GUI..."
        $CC $CFLAGS -I src/gui -c src/gui/desktop.c -o gui_desktop.o 2>&1
        $CC $CFLAGS -I src/gui -c src/gui/mouse.c -o gui_mouse.o 2>&1
        $CC $CFLAGS -I src/gui -c src/gui/keyboard.c -o gui_keyboard.o 2>&1
        $CC $CFLAGS -I src/gui -c src/gui/window.c -o gui_window.o 2>&1
        $CC $CFLAGS -I src/gui -c src/gui/button.c -o gui_button.o 2>&1
        $CC $CFLAGS -I src/gui -c src/gui/string.c -o gui_string.o 2>&1
        
        OBJECTS="boot.o kernel.o mailbox.o fb.o timer.o gic.o mmu.o gui_desktop.o gui_mouse.o gui_keyboard.o gui_window.o gui_button.o gui_string.o"
        ;;
esac

# Linking
echo "Linking kernel..."
$LD $LDFLAGS -T $LINKER_SCRIPT -o flux-kernel $OBJECTS 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi

echo "Linking complete"

# x86: Check Multiboot compliance
if [ "$ARCH" = "x86_32" ]; then
    echo "Checking Multiboot compliance..."
    grub-file --is-x86-multiboot flux-kernel 2>&1
    if [ $? -ne 0 ]; then
        echo "WARNING: Kernel is not Multiboot compliant."
    fi
fi

# Output filename based on architecture
case "$ARCH" in
    x86_32|x86_64)
        KERNEL_NAME="flux-kernel"
        ;;
    aarch64)
        KERNEL_NAME="kernel8.img"
        ;;
esac

# Build output based on type
if [ -n "$BUILD_TYPE" ]; then
    case "$BUILD_TYPE" in
        --img)
            echo "Creating disk image..."
            cp flux-kernel ${KERNEL_NAME}
            echo "Created: ${KERNEL_NAME}"
            ;;
        --iso)
            if [ "$ARCH" = "x86_32" ]; then
                echo "Creating ISO..."
                mkdir -p isodir/boot/grub
                cp flux-kernel isodir/boot/flux-kernel
                cat > isodir/boot/grub/grub.cfg << 'CFGEOF'
set timeout=0
set default=0
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
                echo "Created: flux-os.iso"
            else
                echo "ERROR: ISO creation only supported for x86_32"
                exit 1
            fi
            ;;
        --img.xz)
            echo "Compressing kernel image..."
            xz -k -f flux-kernel
            if [ $? -ne 0 ]; then
                echo "ERROR: xz compression failed."
                exit 1
            fi
            cp flux-kernel.xz ${KERNEL_NAME}.xz
            echo "Created: ${KERNEL_NAME}.xz"
            ;;
    esac
fi

# Clean up object files
rm -f boot.o kernel.o *.o flux-kernel flux-kernel.xz
if [ "$ARCH" = "x86_32" ]; then
    rm -rf isodir
fi

echo ""
echo "========================================"
echo "Build Complete for $ARCH!"
echo "========================================"
echo ""
echo "To build for aarch64 (Raspberry Pi):"
echo "  ARCH=aarch64 ./build.sh --img"
echo ""

