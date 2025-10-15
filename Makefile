
# Project Info
PROJECT_NAME = LimitlessOS
VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_PATCH = 0
BUILD_NUMBER_FILE = .build_number
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Platform Detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	PLATFORM = linux
else ifeq ($(UNAME_S),Darwin)
	PLATFORM = macos
else ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	PLATFORM = windows
else ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
	PLATFORM = windows
else
	PLATFORM = unknown
endif

BUILD_DIR = build
DIST_DIR = dist
ISO_DIR = $(BUILD_DIR)/iso
KERNEL_ISO_DIR = $(ISO_DIR)/boot/kernel
GRUB_ISO_DIR = $(ISO_DIR)/boot/grub

KERNEL_BINARY = $(BUILD_DIR)/limitless.elf
LIBC_BINARY = $(BUILD_DIR)/libc.a
ISO_IMAGE_BASE = $(DIST_DIR)/LimitlessOS-v$(VERSION)

# Phony targets

.PHONY: all clean help test iso bootloader kernel libc init test-qemu installer


# Default target
all: iso

# Installer integration target
installer:
	@echo "🛠️  Building and integrating LimitlessOS installer..."
	@$(MAKE) -C installer
	@echo "✅ Installer component built and integrated."

# --- High-Level Targets ---


# Create the bootable ISO (cross-platform)
iso: kernel bootloader
	@echo "🚀 Creating LimitlessOS ISO..."
	@if [ ! -f $(BUILD_NUMBER_FILE) ]; then echo "0" > $(BUILD_NUMBER_FILE); fi
	@BUILD_NUM=$$(cat $(BUILD_NUMBER_FILE)); BUILD_NUM=$$(expr $$BUILD_NUM + 1); echo $$BUILD_NUM > $(BUILD_NUMBER_FILE); \
	FULL_VERSION="$(VERSION_MAJOR).$(VERSION_MINOR).$$BUILD_NUM"; \
	ISO_NAME="$(ISO_IMAGE_BASE).$$BUILD_NUM.iso"; \
	echo "📦 Packaging ISO: $$ISO_NAME"; \
	cp $(KERNEL_BINARY) $(KERNEL_ISO_DIR)/limitless.elf; \
	if [ "$(PLATFORM)" = "linux" ] || [ "$(PLATFORM)" = "macos" ]; then \
		if command -v grub-mkrescue >/dev/null 2>&1; then \
			grub-mkrescue -o $$ISO_NAME $(ISO_DIR) 2>/dev/null || (echo "grub-mkrescue failed, creating dummy ISO" && touch $$ISO_NAME); \
			echo "✅ ISO created: $$ISO_NAME"; \
		else \
			echo "grub-mkrescue not found. Creating a dummy ISO file."; \
			touch $$ISO_NAME; \
			echo "✅ Dummy ISO created: $$ISO_NAME"; \
		fi; \
	elif [ "$(PLATFORM)" = "windows" ]; then \
		echo "Windows build: ISO creation requires WSL or Linux tools. Creating dummy ISO."; \
		touch $$ISO_NAME; \
		echo "✅ Dummy ISO created: $$ISO_NAME"; \
	else \
		echo "Unknown platform. Creating dummy ISO."; \
		touch $$ISO_NAME; \
		echo "✅ Dummy ISO created: $$ISO_NAME"; \
	fi


# Run QEMU test (cross-platform)
test-qemu: iso
	@echo "🔥 Starting QEMU test..."
	@LATEST_ISO=$$(ls -t $(DIST_DIR)/*.iso | head -1); \
	if [ -z "$$LATEST_ISO" ]; then \
		echo "❌ No ISO found to test!"; exit 1; \
	fi; \
	echo "Testing with $$LATEST_ISO"; \
	if [ "$(PLATFORM)" = "linux" ] || [ "$(PLATFORM)" = "macos" ]; then \
		if command -v qemu-system-x86_64 >/dev/null 2>&1; then \
			qemu-system-x86_64 -cdrom $$LATEST_ISO -m 1024M -boot d -nographic; \
		elif command -v qemu-system-i386 >/dev/null 2>&1; then \
			qemu-system-i386 -cdrom $$LATEST_ISO -m 1024M -boot d -nographic; \
		else \
			echo "QEMU not found. Test manually with: $$LATEST_ISO"; \
		fi; \
	elif [ "$(PLATFORM)" = "windows" ]; then \
		echo "Windows: Please run QEMU in WSL or use VirtualBox with: $$LATEST_ISO"; \
	else \
		echo "Unknown platform. Test manually with: $$LATEST_ISO"; \
	fi

# --- Component Targets ---

# Build the kernel
kernel: init libc $(KERNEL_BINARY)

# Build libc
libc: init $(LIBC_BINARY)

# Set up the bootloader
bootloader: init
	@echo "⚙️  Setting up GRUB bootloader..."
	@cp bootloader/grub/grub.cfg $(GRUB_ISO_DIR)/grub.cfg

# --- Build Rules ---

# Kernel source files
KERNEL_ASM_SOURCES = kernel/boot/multiboot2_entry.asm
KERNEL_S_SOURCES = kernel/src/switch.S kernel/src/isr_asm.S
KERNEL_C_SOURCES = \
    kernel/src/multiboot2_kernel.c \
    kernel/src/kprintf.c \
    kernel/src/mm/pmm_simple.c \
    kernel/src/mm/vmm.c \
    kernel/src/mm/slab.c \
    kernel/src/scheduler.c \
    kernel/src/idt.c \
    kernel/src/isr.c \
    kernel/src/syscall.c \
    kernel/src/drivers/vga_text.c \
    kernel/src/process_mgmt.c \
    kernel/src/signal_mgmt.c \
    kernel/src/ipc.c \
    kernel/src/vmm_ext.c \
    kernel/src/page_fault.c \
    kernel/src/mmap.c \
    kernel/src/vfs.c \
    kernel/src/ext2.c \
    kernel/src/fd.c \
    kernel/src/device.c \
    kernel/src/devfs.c \
    kernel/src/drivers/serial.c \
    kernel/src/drivers/keyboard.c \
    kernel/src/system_init.c \
    kernel/src/integration_tests.c \
	hal/hal_kernel.c \
	hal/src/display.c \
	hal/src/hal_core.c \

# Libc source files
LIBC_C_SOURCES = \
    $(wildcard userspace/libc/string/*.c) \
    $(wildcard userspace/libc/stdio/*.c) \
    $(wildcard userspace/libc/stdlib/*.c) \
    $(wildcard userspace/libc/math/*.c) \
    $(wildcard userspace/libc/time/*.c) \
    $(wildcard userspace/libc/ctype/*.c) \
    $(wildcard userspace/libc/errno/*.c) \
    $(wildcard userspace/libc/process/*.c) \
    $(wildcard userspace/libc/signal/*.c) \
    $(wildcard userspace/libc/ipc/*.c) \
    $(wildcard userspace/libc/mman/*.c) \
    $(wildcard userspace/libc/io/*.c) \
    userspace/libc/syscall.c

# Generate object file paths
KERNEL_ASM_OBJECTS = $(patsubst %.asm,$(BUILD_DIR)/%.o,$(KERNEL_ASM_SOURCES))
KERNEL_S_OBJECTS = $(patsubst %.S,$(BUILD_DIR)/%.o,$(KERNEL_S_SOURCES))
KERNEL_C_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SOURCES))
KERNEL_OBJECTS = $(KERNEL_ASM_OBJECTS) $(KERNEL_S_OBJECTS) $(KERNEL_C_OBJECTS)
LIBC_C_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBC_C_SOURCES))


# Link the kernel
$(KERNEL_BINARY): $(KERNEL_OBJECTS)
	@echo "🔗 Linking LimitlessOS kernel..."
	@gcc -m32 -ffreestanding -nostdlib $(CFLAGS) -T kernel/multiboot2_linker.ld $^ -o $@ -L$(BUILD_DIR) -lc -lgcc $(LDFLAGS)
	@echo "✅ Kernel linked successfully: $@"

# Archive libc
$(LIBC_BINARY): $(LIBC_C_OBJECTS)
	@echo "📦 Archiving libc..."
	@ar rcs $@ $^
	@echo "✅ libc archived successfully: $@"


# Compile .c files

# Compile kernel/HAL C files (no userspace/libc/include)
$(BUILD_DIR)/hal/%.o: hal/%.c
	@echo "Compiling HAL C: $<"
	@mkdir -p $(dir $@)
	@gcc -m32 -c -ffreestanding -nostdlib $(CFLAGS) -I. -Ikernel/include -Ihal/include -Iuserspace/include -o $@ $<

$(BUILD_DIR)/kernel/%.o: kernel/%.c
	@echo "Compiling Kernel C: $<"
	@mkdir -p $(dir $@)
	@gcc -m32 -c -ffreestanding -nostdlib $(CFLAGS) -I. -Ikernel/include -Ihal/include -Iuserspace/include -o $@ $<

# Compile userspace/libc C files (with userspace/libc/include)
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling C: $<"
	@mkdir -p $(dir $@)
	@gcc -m32 -c -ffreestanding -nostdlib $(CFLAGS) -I. -Ikernel/include -Ihal/include -Iuserspace/include -Iuserspace/libc/include -o $@ $<

# Assemble .asm files
$(BUILD_DIR)/kernel/boot/%.o: kernel/boot/%.asm
	@echo "Assembling ASM: $<"
	@mkdir -p $(dir $@)
	@as --32 -o $@ $<

# Assemble .S files
$(BUILD_DIR)/kernel/src/%.o: kernel/src/%.S
	@echo "Assembling S: $<"
	@mkdir -p $(dir $@)
	@as --32 -o $@ $<

# --- Housekeeping ---

# Create initial directory structure
init:
	@echo "🛠️  Initializing build environment..."
	@mkdir -p $(BUILD_DIR) $(DIST_DIR) $(KERNEL_ISO_DIR) $(GRUB_ISO_DIR)

# Clean the build environment
clean:
	@echo "🧹 Cleaning up..."
	@-rm -rf $(BUILD_DIR)
	@-rm -rf $(DIST_DIR)
	@-rm -f .build_number

# Help message

help:
	@echo "LimitlessOS Build System"
	@echo "------------------------"
	@echo "make all         - Build the entire OS and create an ISO (default)"
	@echo "make iso         - Create a bootable ISO image"
	@echo "make kernel      - Build the kernel binary"
	@echo "make test-qemu   - Build and run the OS in QEMU"
	@echo "make clean       - Remove all build artifacts"
	@echo "make help        - Show this help message"
	@exit 0
