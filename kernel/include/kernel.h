#define STATUS_ETIMEDOUT K_ETIMEDOUT
struct sched_stats;
#pragma once

/*
 * Kernel core definitions and initialization entrypoints.
 * Phase 1 focuses on:
 *  - Processes/threads, preemptive scheduler, syscalls hooks
 *  - Virtual memory manager (paging skeleton, COW hooks)
 *  - Interrupts and timers (APIC/HPET/TSC stubs wired)
 *  - ACPI and PCI enumeration (MADT/FADT parse, PCI scan)
 *  - VFS core with tmpfs, and drivers stubs for storage
 *
 * This header is intentionally self-contained and avoids HAL specifics.
 * HAL-arch routines are declared via forward externs; real impls live in hal/.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Basic types */
typedef uint64_t u64;
typedef int64_t  s64;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint8_t  u8;
typedef int8_t   s8;
typedef u64      phys_addr_t;
typedef u64      virt_addr_t;

#ifndef __pid_t_defined
typedef int pid_t;
#endif
typedef u32 tid_t; // Keep the original typedefs
typedef u32 uid_t;
typedef u32 gid_t;

/* CPU/topology limits */
#define KERNEL_MAX_CPUS 64

/* Result/status codes */
typedef enum {
    K_OK = 0,
    K_ERR = -1,
    /* Permission / access */
    K_EPERM = -1,      /* Operation not permitted (alias of generic error for now) */
    K_ENOMEM = -12,
    K_EINTR  = -4,
    K_EINVAL = -22,
    K_ENOENT = -2,
    K_EIO    = -5,
    K_EISDIR = -21,
    K_ENOTDIR = -20,
    K_ENOTSUP = -95,
    K_EBUSY = -16,
    K_EFBIG = -27, /* File too large / exceeds format-imposed limit */
    K_EEXIST = -17, /* Already exists */
    K_ENAMETOOLONG = -36,
    K_ENOTEMPTY = -39,
    K_ENOSPC = -28, /* No space left (added) */
    K_EFAULT = -14, /* bad address */
    K_EBADF  = -9,  /* bad file/socket descriptor */
    K_ENOSYS = -38, /* function not implemented */
    K_ETIMEDOUT = -110, /* operation timed out */
    K_EAFNOSUPPORT = -97, /* address family not supported */
    K_EPROTONOSUPPORT = -93, /* protocol not supported */
} kstatus_t;

/* Panic/assert */
/* Compatibility / convenience layer (legacy microkernel-style naming) */
typedef kstatus_t status_t;
#define STATUS_OK        K_OK
#define STATUS_INVALID   K_EINVAL
#define STATUS_EXISTS    K_EEXIST
#define STATUS_NOSUPPORT K_ENOTSUP
/* Additional legacy-style aliases used by subsystems */
#define STATUS_NOMEM     K_ENOMEM
#define STATUS_EIO       K_EIO
#define STATUS_DENIED    K_EPERM
#define STATUS_NOTFOUND  K_ENOENT
#define STATUS_ERROR     K_ERR
#define STATUS_NO_MEMORY K_ENOMEM
#define STATUS_SUCCESS   K_OK
#define STATUS_INVALID_PARAMETER K_EINVAL
#define STATUS_NOT_SUPPORTED K_ENOTSUP
#define STATUS_NO_SPACE K_ENOSPC
#define STATUS_ALREADY_EXISTS K_EEXIST

/* Common predicate helpers */
#define SUCCESS(x)   ((x) == K_OK)
#define FAILED(x)    ((x) != K_OK)

/* Backwards compat (some code used SUCCESS before we introduced FAILED) */

/* Bit helper and alignment attribute */
#ifndef BIT
#define BIT(n) (1u << (n))
#endif
#ifndef ALIGNED
#define ALIGNED(x) __attribute__((aligned(x)))
#endif
#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#endif
#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))
#endif

/* Provide short aliases expected by some subsystems */
typedef virt_addr_t vaddr_t;
typedef phys_addr_t paddr_t;
#define K_PANIC_ON(x, msg) do { if ((x)) kernel_panic(__FILE__, __LINE__, (msg)); } while (0)
#define K_ASSERT(x) do { if (!(x)) kernel_panic(__FILE__, __LINE__, "assertion failed: " #x); } while (0)

/* Simple spinlock - compatible with vmm.h */

/* Per-CPU accessors (HAL provides gs-based or array-based implementation) */
extern u32 hal_cpu_count(void);
extern u32 hal_cpu_id(void);

/* Context switching (HAL) */
struct arch_context; /* opaque arch CPU context (registers, fpu, etc.) */
extern void hal_arch_switch_context(struct arch_context** old_ctx, struct arch_context* new_ctx);
extern void hal_arch_prepare_kthread(struct arch_context** out_ctx, void (*entry)(void*), void* arg, void* stack_top);

/* Interrupt control (HAL) */
extern void hal_interrupt_enable(void);
extern void hal_interrupt_disable(void);

/* I/O memory barriers (HAL) */
extern void hal_mfence(void);

/* Logging (HAL or kernel printf) */
extern void kprintf(const char* fmt, ...);
/* Minimal formatted output helpers (implemented in logging/printf module) */
int k_snprintf(char* buf, size_t bufsz, const char* fmt, ...);
int k_vsnprintf(char* buf, size_t bufsz, const char* fmt, __builtin_va_list ap);

/* Boot info (provided by bootloader/hal at early init) */
typedef struct memory_region {
    phys_addr_t base;
    u64 length;
    u32 type; /* 1=usable RAM, others reserved/acpi/etc. */
} memory_region_t;

typedef struct boot_info {
    u64 rsdp_phys;               /* ACPI RSDP physical address if known */
    memory_region_t* mem_map;    /* pointer to memory map (phys or virt as arranged by hal) */
    u32 mem_map_count;
    phys_addr_t kernel_phys_base;
    phys_addr_t kernel_phys_end;
    virt_addr_t kernel_virt_base;
    virt_addr_t kernel_virt_end;
} boot_info_t;

/* Kernel global boot info (set during early init) */
extern boot_info_t g_boot_info;

/* Initialization stages */
void kernel_early_init(const boot_info_t* bi);
void kernel_init_ap(const boot_info_t* bi);
void kernel_start(void);

/* KASLR: runtime slide applied to kernel virtual base (optional) */
extern u64 g_kaslr_slide; /* zero if not randomized */
u64 kaslr_get_slide(void);

/* Subsystem init (in logical order) */
void vmm_init(const boot_info_t* bi);
void interrupts_init(void);
void timer_init(void);
void acpi_init(const boot_info_t* bi);
void pci_init(void);
void storage_init(void);
void vfs_init(void);
void process_init(void);
int scheduler_init(void);
void scheduler_tick(void);
void scheduler_schedule(void);
int scheduler_start_multitasking(void);

/* Thread management functions */
struct thread;
typedef struct thread thread_t;
struct thread* thread_current(void);
void thread_set_current(struct thread* t);
int scheduler_create_kthread(thread_t** out_thread, void (*entry)(void*), 
                            void* arg, void* stack_base, size_t stack_size, 
                            u32 affinity_cpu);
void scheduler_enqueue(thread_t* t);
void scheduler_dequeue(thread_t* t);
void scheduler_wake(thread_t* t);
void scheduler_sleep(thread_t* t);
void scheduler_block(thread_t* t);
void scheduler_get_stats(struct sched_stats *stats);

/* Scheduler testing */
void scheduler_test(void);

/* VGA output functions */
void vga_print(const char* str);
void vga_print_num(u32 num);

/* Ticks per second (timer) */
u64 timer_get_ticks(void);
u64 timer_get_freq_hz(void);

/* Panic */
void kernel_panic(const char* file, int line, const char* msg);

/* Minimal memset/memcpy for freestanding safety (linker can override) */
static inline void* k_memset(void* dst, int c, size_t n) {
    u8* p = (u8*)dst; while (n--) *p++ = (u8)c; return dst;
}
static inline void* k_memcpy(void* dst, const void* src, size_t n) {
    u8* d=(u8*)dst; const u8* s=(const u8*)src; while (n--) *d++=*s++; return dst;
}
static inline int k_memcmp(const void* a, const void* b, size_t n){ const u8* x=(const u8*)a; const u8* y=(const u8*)b; while(n--){ if(*x!=*y) return (int)*x - (int)*y; x++; y++; } return 0; }
static inline size_t k_strlen(const char* s) { size_t n=0; while (s && *s++) ++n; return n; }
static inline int k_strcmp(const char* a, const char* b){ while(a&&b&&*a&&(*a==*b)){ a++; b++; } if(!a||!b) return (a==b)?0: (a?1:-1); return (unsigned char)*a - (unsigned char)*b; }
static inline int k_strncmp(const char* a, const char* b, size_t n){ if(n==0) return 0; while(n-- && a && b && *a && (*a==*b)){ if(n==0) break; a++; b++; } if(n==(size_t)-1) return 0; if(!a||!b) return (a==b)?0:(a?1:-1); return (unsigned char)*a - (unsigned char)*b; }
static inline char* k_strcpy(char* dst, const char* src){ char* d=dst; if(dst&&src){ while((*d++ = *src++)); } return dst; }
static inline size_t k_strlcpy(char* dst, const char* src, size_t size){ if(!dst||!size) return 0; size_t i=0; if(src){ for(; i+1<size && src[i]; i++) dst[i]=src[i]; if(size) dst[i]='\0'; while(src[i]) i++; } else dst[0]='\0'; return i; }
static inline const char* k_strstr(const char* h, const char* n){ if(!h||!n||!*n) return h; size_t nl=0; while(n[nl]) nl++; for(const char* p=h; *p; p++){ if(*p==*n){ size_t i=1; while(i<nl && p[i]==n[i]) i++; if(i==nl) return p; } } return NULL; }

/* Page size and alignment macros */
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096UL
#endif
#ifndef PAGE_MASK
#define PAGE_MASK (~(PAGE_SIZE-1UL))
#endif
#ifndef PAGE_ALIGN_UP
#define PAGE_ALIGN_UP(x)   (((u64)(x) + PAGE_SIZE - 1) & PAGE_MASK)
#endif
#ifndef PAGE_ALIGN_DOWN
#define PAGE_ALIGN_DOWN(x) ((u64)(x) & PAGE_MASK)
#endif
#ifndef KB
#define KB(x) ((u64)(x) * 1024ULL)
#endif
#ifndef MB
#define MB(x) (KB(x) * 1024ULL)
#endif
#ifndef GB
#define GB(x) (MB(x) * 1024ULL)
#endif

/* Signal constants */
#define SIGTERM 15
#define SIGKILL 9
#define SPINLOCK_INIT 0

/* Standard library functions */
int snprintf(char* buffer, size_t size, const char* format, ...);
status_t vfs_mkdir(const char* path, uint16_t mode);
void scheduler_yield(void);

/* String functions */
char* strncpy(char* dest, const char* src, size_t n);
char* strcpy(char* dest, const char* src);
char* strchr(const char* s, int c);
int strcmp(const char* s1, const char* s2);
size_t strlen(const char* s);

/* Spinlock function aliases */

/* Memory management functions */
void pmm_incref(paddr_t page);
uint32_t pmm_decref(paddr_t page);
void KLOG_ERROR(const char* subsystem, const char* format, ...);

/* Memory allocation functions */
void* kalloc(size_t size);
void kfree(void* ptr);
void* memset(void* s, int c, size_t n);

/* Simple GUI system functions */
void simple_gui_init(void);
void simple_gui_clear(uint32_t color);
void simple_gui_draw_desktop(void);
void simple_gui_draw_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                           const char* title, uint32_t content_color);

/* Hardware driver functions */
void pci_init(void);
void storage_init(void);
void network_init(void);

/* File system functions */
void simple_fs_init(void);

/* Network stack functions */
void network_stack_init(void);

/* System call functions */
void syscall_init(void);
long syscall_dispatch(long syscall_num, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

/* Network functions used by syscalls */
int network_socket(u8 protocol);
int network_bind(int sockfd, u32 ip, u16 port);
int network_connect(int sockfd, u32 remote_ip, u16 remote_port);
int network_send(int sockfd, const void* data, u32 size);

/* File system functions used by syscalls */
int simple_fs_open(const char* path, u32 flags);
int simple_fs_read(int fd, void* buffer, u32 count);
int simple_fs_write(int fd, const void* buffer, u32 count);
int simple_fs_close(int fd);

/* Compatibility layer functions */
void compat_init(void);
int strncmp(const char* s1, const char* s2, size_t n);

/* HAL functions for hardware drivers */
uint32_t hal_pci_cfg_read32(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset);

/* I/O port functions */
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t result;
    __asm__ volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}
