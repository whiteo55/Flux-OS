# TODO: Raspberry Pi 500 (AArch64) Support

## Phase 1: Architecture Foundation
- [x] Plan and get approval
- [x] Create `src/arch/aarch64/` directory structure
- [x] Create linker script `linker_aarch64_pi.ld`
- [x] Create ARM64 bootloader `src/arch/aarch64/boot.S`

## Phase 2: Kernel Core (ARM64)
- [x] Create `src/arch/aarch64/kernel.c` - Entry point
- [x] Create `src/arch/aarch64/mmu.c` - MMU setup
- [x] Create `src/arch/aarch64/gic.c` - GICv2 interrupt controller
- [x] Create `src/arch/aarch64/timer.c` - ARM generic timer

## Phase 3: Raspberry Pi Hardware Support
- [x] Create `src/arch/aarch64/mailbox.h` - Mailbox definitions
- [x] Create `src/arch/aarch64/mailbox.c` - Mailbox protocol
- [x] Create `src/arch/aarch64/fb.h` - Framebuffer header
- [x] Create `src/arch/aarch64/fb.c` - Framebuffer driver
- [ ] Create `src/arch/aarch64/gpio.c` - GPIO (for future use)
- [ ] Create `src/arch/aarch64/board.c` - Board-specific init

## Phase 4: Build System Integration
- [ ] Update `build.sh` with `--arch aarch64` support
- [ ] Add cross-compilation toolchain detection (aarch64-linux-gnu-*)
- [ ] Add output naming convention for multi-arch

## Phase 5: Common Kernel Integration
- [ ] Create arch abstraction layer in `src/kernel/`
- [ ] Adapt `src/kernel/kernel.c` for multi-arch
- [ ] Create `src/arch/common/` for shared code

## Phase 6: Testing & Documentation
- [ ] Document build process for Pi
- [ ] Test compilation for aarch64
- [ ] Create SD card image creation instructions
