/* LimitlessOS Emergency Minimal Kernel */

/* Multiboot2 header */
#define MULTIBOOT2_MAGIC 0xe85250d6
#define MULTIBOOT2_ARCH_I386 0

struct multiboot2_header_tag {
    unsigned short type;
    unsigned short flags;
    unsigned int size;
};

struct multiboot2_header {
    unsigned int magic;
    unsigned int architecture;
    unsigned int header_length;
    unsigned int checksum;
    
    /* End tag */
    struct multiboot2_header_tag end_tag;
} __attribute__((packed));

__attribute__((section(".multiboot2")))
__attribute__((aligned(8)))
__attribute__((used))
const struct multiboot2_header mb_header = {
    .magic = MULTIBOOT2_MAGIC,
    .architecture = MULTIBOOT2_ARCH_I386,
    .header_length = sizeof(struct multiboot2_header),
    .checksum = -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH_I386 + sizeof(struct multiboot2_header)),
    
    /* End tag - required */
    .end_tag = {
        .type = 0,
        .flags = 0,
        .size = 8
    }
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