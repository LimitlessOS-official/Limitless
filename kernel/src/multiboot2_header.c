/*
 * LimitlessOS Multiboot2 Header
 * 
 * Production-grade multiboot2 header implementation in C
 * This ensures proper multiboot2 compliance and boot reliability.
 */

#include <stdint.h>

/* Multiboot2 constants */
#define MULTIBOOT2_MAGIC        0xe85250d6
#define MULTIBOOT2_ARCHITECTURE 0
#define MULTIBOOT2_HEADER_TAG_END               0
#define MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST 1
#define MULTIBOOT2_HEADER_TAG_FRAMEBUFFER       5
#define MULTIBOOT2_HEADER_TAG_CONSOLE_FLAGS     4

/* Calculate header length */
#define HEADER_LENGTH (multiboot2_header_end - multiboot2_header_start)

/* Multiboot2 header structure */
struct multiboot2_header_tag {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
} __attribute__((packed));

struct multiboot2_header_tag_framebuffer {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} __attribute__((packed));

struct multiboot2_header_tag_console_flags {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t console_flags;
} __attribute__((packed));

struct multiboot2_header_tag_info_request {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t requests[];
} __attribute__((packed));

/* Multiboot2 header - must be in first 32KB and 8-byte aligned */
__attribute__((section(".multiboot2")))
__attribute__((aligned(8)))
const struct {
    /* Required header */
    uint32_t magic;
    uint32_t architecture;
    uint32_t header_length;
    uint32_t checksum;
    
    /* Framebuffer tag */
    struct multiboot2_header_tag_framebuffer framebuffer_tag;
    
    /* Console flags tag */
    struct multiboot2_header_tag_console_flags console_tag;
    
    /* Information request tag */
    struct {
        struct multiboot2_header_tag_info_request header;
        uint32_t requests[7];
    } info_tag;
    
    /* End tag */
    struct multiboot2_header_tag end_tag;
    
} multiboot2_header = {
    /* Required multiboot2 header */
    .magic = MULTIBOOT2_MAGIC,
    .architecture = MULTIBOOT2_ARCHITECTURE,
    .header_length = sizeof(multiboot2_header),
    .checksum = -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCHITECTURE + sizeof(multiboot2_header)),
    
    /* Framebuffer tag */
    .framebuffer_tag = {
        .type = MULTIBOOT2_HEADER_TAG_FRAMEBUFFER,
        .flags = 0,
        .size = sizeof(struct multiboot2_header_tag_framebuffer),
        .width = 1024,
        .height = 768,
        .depth = 32
    },
    
    /* Console flags tag */
    .console_tag = {
        .type = MULTIBOOT2_HEADER_TAG_CONSOLE_FLAGS,
        .flags = 0,
        .size = sizeof(struct multiboot2_header_tag_console_flags),
        .console_flags = 0  /* Support both text and graphics */
    },
    
    /* Information request tag */
    .info_tag = {
        .header = {
            .type = MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST,
            .flags = 0,
            .size = sizeof(struct multiboot2_header_tag_info_request) + 7 * sizeof(uint32_t)
        },
        .requests = {
            4,  /* Basic memory info */
            6,  /* Memory map */
            8,  /* Framebuffer info */
            9,  /* ELF symbols */
            14, /* ACPI old */
            15, /* ACPI new */
            1   /* Command line */
        }
    },
    
    /* End tag */
    .end_tag = {
        .type = MULTIBOOT2_HEADER_TAG_END,
        .flags = 0,
        .size = 8
    }
};

/* Mark header boundaries for linker script */
const char multiboot2_header_start[] __attribute__((section(".multiboot2"))) = "";
const char multiboot2_header_end[] __attribute__((section(".multiboot2_end"))) = "";