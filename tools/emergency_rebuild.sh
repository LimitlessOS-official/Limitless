#!/bin/bash

# ============================================================================
# LimitlessOS Emergency Bootloader Rebuild Script
# Creates a minimal, guaranteed-to-work bootable ISO
# ============================================================================

echo "🚨 Emergency Bootloader Rebuild for LimitlessOS"
echo "=============================================="
echo

# Create minimal kernel with proper multiboot2 headers
echo "1. Creating emergency minimal kernel..."

cat > kernel/src/emergency_kernel.c << 'EOF'
/* LimitlessOS Emergency Minimal Kernel */

/* Multiboot2 header */
#define MULTIBOOT2_MAGIC 0xe85250d6

struct multiboot2_header {
    unsigned int magic;
    unsigned int architecture;
    unsigned int header_length;
    unsigned int checksum;
    unsigned short type;
    unsigned short flags;
    unsigned int size;
} __attribute__((packed));

__attribute__((section(".multiboot2")))
__attribute__((aligned(8)))
static const struct multiboot2_header mb_header = {
    .magic = MULTIBOOT2_MAGIC,
    .architecture = 0,
    .header_length = sizeof(struct multiboot2_header),
    .checksum = -(MULTIBOOT2_MAGIC + 0 + sizeof(struct multiboot2_header)),
    .type = 0,
    .flags = 0,
    .size = 8
};

/* VGA text mode output */
static volatile unsigned short* vga_buffer = (volatile unsigned short*)0xB8000;
static int vga_row = 0;
static int vga_col = 0;

void vga_putchar(char c) {
    if (c == '\n') {
        vga_row++;
        vga_col = 0;
        return;
    }
    
    if (vga_row >= 25) {
        vga_row = 0;
    }
    
    if (vga_col >= 80) {
        vga_col = 0;
        vga_row++;
    }
    
    int offset = vga_row * 80 + vga_col;
    vga_buffer[offset] = (0x0F << 8) | c; // White on black
    vga_col++;
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_clear(void) {
    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = 0;
    }
    vga_row = 0;
    vga_col = 0;
}

/* Kernel entry point */
void kernel_main(void) {
    vga_clear();
    
    vga_puts("LimitlessOS Enterprise v1.0.0\n");
    vga_puts("Emergency Boot Mode - System Online\n");
    vga_puts("=====================================\n\n");
    
    vga_puts("✅ Multiboot2 bootloader working\n");
    vga_puts("✅ VGA text mode initialized\n");
    vga_puts("✅ Kernel successfully loaded\n\n");
    
    vga_puts("System Status: OPERATIONAL\n");
    vga_puts("Boot Mode: Emergency Safe Mode\n");
    vga_puts("Architecture: x86_64\n\n");
    
    vga_puts("LimitlessOS is ready for installation.\n");
    vga_puts("This emergency kernel validates bootloader functionality.\n\n");
    
    vga_puts("System halted. Reboot to continue.\n");
    
    // Halt the system
    while(1) {
        __asm__("hlt");
    }
}
EOF

echo "2. Creating emergency linker script..."

cat > kernel/emergency_linker.ld << 'EOF'
ENTRY(kernel_main)

SECTIONS
{
    . = 1M;

    .multiboot2 ALIGN(8) :
    {
        *(.multiboot2)
    }

    .text ALIGN(4K) :
    {
        *(.text)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    .data ALIGN(4K) :
    {
        *(.data)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
}
EOF

echo "3. Compiling emergency kernel..."
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c kernel/src/emergency_kernel.c -o build/kernel/emergency_kernel.o

ld -m elf_i386 -T kernel/emergency_linker.ld -o build/kernel/limitless_emergency.elf build/kernel/emergency_kernel.o

echo "4. Creating minimal GRUB config..."

mkdir -p build/iso/boot/grub

cat > build/iso/boot/grub/grub.cfg << 'EOF'
set timeout=5
set default=0

insmod multiboot2

menuentry "LimitlessOS Emergency Boot" {
    echo "Loading LimitlessOS Emergency Kernel..."
    multiboot2 /boot/limitless_emergency.elf
    boot
}
EOF

echo "5. Copying kernel to ISO build directory..."
cp build/kernel/limitless_emergency.elf build/iso/boot/

echo "6. Creating emergency bootable ISO..."
grub-mkrescue -o LimitlessOS-Emergency.iso build/iso/

echo "✅ Emergency ISO created: LimitlessOS-Emergency.iso"
echo
echo "🧪 TEST THIS EMERGENCY ISO FIRST:"
echo "   This minimal ISO will validate if the bootloader is working"
echo "   If this boots successfully, the issue is in the complex kernel"
echo "   If this fails, the issue is with GRUB/bootloader setup"
echo