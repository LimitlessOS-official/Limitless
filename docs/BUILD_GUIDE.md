# LimitlessOS Production Build Guide

## Quick Start

### Building the Production Kernel

```bash
# Clean and build production kernel
make -f Makefile.production clean
make -f Makefile.production production

# The kernel binary will be at:
# build/production/limitless_production.elf
```

### Creating Bootable ISO (requires xorriso)

```bash
# Install xorriso (if not already installed)
sudo apt-get install xorriso  # Debian/Ubuntu
sudo yum install xorriso      # RedHat/CentOS
brew install xorriso          # macOS

# Build ISO
make -f Makefile.production iso

# The ISO will be at:
# dist/production/LimitlessOS-Production-v2.0.0.iso
```

### Testing in QEMU

```bash
# Test kernel directly
qemu-system-i386 -kernel build/production/limitless_production.elf -m 512M

# Test ISO (if available)
qemu-system-i386 -cdrom dist/production/LimitlessOS-Production-v2.0.0.iso -m 512M
```

## Build Requirements

### Essential Tools
- GCC with 32-bit support (`gcc-multilib` on Ubuntu)
- GNU Make
- GNU Assembler (as)
- GNU Linker (ld)

### Optional Tools for ISO Creation
- GRUB tools (`grub-common`, `grub-pc-bin`)
- xorriso (for ISO generation)

### Testing Tools
- QEMU (`qemu-system-i386` or `qemu-system-x86_64`)
- VirtualBox (alternative to QEMU)

## Project Structure

```
LimitlessOS/
├── kernel/
│   ├── boot/
│   │   └── multiboot2_entry.asm      # Boot assembly code
│   ├── src/
│   │   ├── production_kernel.c       # Main production kernel
│   │   └── mm/
│   │       └── pmm_simple.c          # Physical memory manager
│   ├── include/
│   │   └── mm/
│   │       └── pmm_simple.h          # PMM header
│   └── simple_linker.ld              # Linker script
├── bootloader/
│   └── grub/
│       └── grub_production.cfg       # GRUB configuration
├── servers/                          # Userspace servers
│   ├── drivers/
│   │   └── usb_server.c              # USB host controller driver
│   ├── fs/
│   │   └── vfs_server.c              # Virtual file system server
│   └── services/
│       └── power_mgmt_daemon.c       # Power management daemon
├── docs/
│   ├── HYBRID_KERNEL_ARCHITECTURE.md # Architecture documentation
│   ├── PRODUCTION_README.md          # Production features
│   └── BUILD_GUIDE.md                # This file
├── Makefile.production               # Production build system
└── README.md                         # Main README

```

## Build Targets

### `make -f Makefile.production all`
Default target. Builds the production kernel.

### `make -f Makefile.production production`
Explicitly builds the production kernel binary.

### `make -f Makefile.production iso`
Creates a bootable ISO image (requires xorriso).

### `make -f Makefile.production clean`
Removes all build artifacts.

### `make -f Makefile.production help`
Shows help message with available targets.

## Kernel Features

### Implemented ✅
- **Multiboot2 Boot**: Compatible with GRUB and other bootloaders
- **VGA Text Console**: Color terminal output with scrolling
- **Physical Memory Manager**: Buddy allocator for page frames
- **Clean Architecture**: No dependencies on Linux headers
- **Hybrid Kernel Design**: Microkernel core with userspace services

### In Progress 🚧
- Virtual Memory Manager (VMM)
- Process scheduler
- IPC subsystem
- System call interface

### Planned 📋
- Complete memory management (COW, demand paging)
- Process management (fork, exec, exit)
- Thread support
- Signal handling
- VFS implementation
- Device driver framework
- Network stack

## Kernel Output

When booted, the production kernel displays:

```
===============================================================================
        LimitlessOS - Production-Grade Hybrid Kernel v2.0
===============================================================================

[BOOT] Starting production kernel initialization...

[PHASE 1] Memory Management Initialization
[PMM] Initializing production physical memory manager...
[PMM] Memory start: 0x0000000000200000, size: 16 MB
[PMM] Total pages: 4096, Free pages: 3840
[PMM] Initialization complete

[PHASE 2] Process Management Initialization
[SCHED] Initializing production scheduler...
[SCHED] Scheduler ready

[PHASE 3] IPC Subsystem Initialization
[IPC] Message passing IPC ready

[PHASE 4] Device Driver Framework
[DRIVER] Loading core drivers...
[DRIVER] Timer driver initialized
[DRIVER] Interrupt controller initialized
[DRIVER] Console driver initialized

===============================================================================
        Kernel Initialization Complete - System Ready
===============================================================================

Hybrid Kernel Architecture:
  - Microkernel core for critical operations
  - Userspace drivers for fault isolation
  - Message-based IPC for communication
  - Production-grade memory management
  - POSIX-compliant system interface

System Status: RUNNING
Waiting for init process...
```

## Troubleshooting

### "undefined reference to `kernel_main`"
Make sure the C kernel function is named `kernel_main` to match the assembly entry point.

### "xorriso not found"
Install xorriso package for ISO creation, or test the kernel directly with QEMU.

### Warnings about missing .note.GNU-stack
This is normal and can be ignored. It's just informational.

### Build fails with "gcc: command not found"
Install GCC: `sudo apt-get install build-essential gcc-multilib`

### "No such file or directory" for includes
Make sure you're building from the project root directory.

## Advanced Build Options

### Custom Compiler Flags

```bash
# Build with debugging symbols
CFLAGS="-g -O0" make -f Makefile.production

# Build with optimization
CFLAGS="-O3 -march=native" make -f Makefile.production

# Build with verbose output
make -f Makefile.production V=1
```

### Cross-Compilation

```bash
# For 64-bit target (future)
ARCH=x86_64 make -f Makefile.production

# For ARM target (future)
ARCH=arm make -f Makefile.production
```

## Testing

### Unit Tests (Planned)
```bash
make -f Makefile.production test
```

### Integration Tests (Planned)
```bash
make -f Makefile.production integration-test
```

### Running in QEMU

```bash
# Basic test
qemu-system-i386 -kernel build/production/limitless_production.elf

# With more memory
qemu-system-i386 -kernel build/production/limitless_production.elf -m 1G

# With serial output
qemu-system-i386 -kernel build/production/limitless_production.elf -serial stdio

# With debugging
qemu-system-i386 -kernel build/production/limitless_production.elf -s -S
# In another terminal:
gdb build/production/limitless_production.elf
(gdb) target remote :1234
(gdb) continue
```

## Contributing

When contributing to the production kernel:

1. Follow the existing code style
2. Add proper error handling
3. Include comments for complex logic
4. Test your changes in QEMU
5. Update documentation as needed
6. Keep code modular and clean

## License

LimitlessOS is released under the MIT License.

## Support

For issues, questions, or contributions:
- Open an issue on GitHub
- Read the documentation in `docs/`
- Check the examples in `servers/`

---

**Version**: 2.0.0 | **Build System**: GNU Make | **License**: MIT
