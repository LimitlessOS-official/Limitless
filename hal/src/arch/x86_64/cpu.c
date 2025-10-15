/*
 * x86_64 CPU Management
 */

#include "hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Forward declarations for VMM functions */
extern void* vmm_kmalloc(size_t size, size_t alignment);
extern void vmm_kfree(void* ptr, size_t size);
/* k_memset defined in kernel.h */

static uint32_t detected_cpu_count = 1;
static cpu_info_t cpu_info_cache = {0};

/* CPUID instruction wrapper */
static inline void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(0));
}

/* Read timestamp counter */
static inline uint64_t rdtsc(void) {
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

/* Detect CPU features */
static void detect_cpu_features(cpu_info_t* info) {
    uint32_t eax, ebx, ecx, edx;

    /* Get vendor string */
    cpuid(0, &eax, &ebx, &ecx, &edx);
    *(uint32_t*)(info->vendor + 0) = ebx;
    *(uint32_t*)(info->vendor + 4) = edx;
    *(uint32_t*)(info->vendor + 8) = ecx;
    info->vendor[12] = '\0';

    /* Get processor info and features */
    cpuid(1, &eax, &ebx, &ecx, &edx);

    info->stepping = eax & 0xF;
    info->model_id = (eax >> 4) & 0xF;
    info->family = (eax >> 8) & 0xF;

    /* Extended model and family */
    if (info->family == 0xF) {
        info->family += (eax >> 20) & 0xFF;
    }
    if (info->family == 0x6 || info->family == 0xF) {
        info->model_id += ((eax >> 16) & 0xF) << 4;
    }

    /* Feature flags */
    info->has_sse = (edx & (1 << 25)) != 0;
    info->has_sse2 = (edx & (1 << 26)) != 0;
    info->has_avx = (ecx & (1 << 28)) != 0;

    /* Check extended features */
    cpuid(7, &eax, &ebx, &ecx, &edx);
    info->has_avx2 = (ebx & (1 << 5)) != 0;
    info->has_avx512 = (ebx & (1 << 16)) != 0;

    /* Get processor brand string */
    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    if (eax >= 0x80000004) {
        uint32_t* brand = (uint32_t*)info->model;
        cpuid(0x80000002, &brand[0], &brand[1], &brand[2], &brand[3]);
        cpuid(0x80000003, &brand[4], &brand[5], &brand[6], &brand[7]);
        cpuid(0x80000004, &brand[8], &brand[9], &brand[10], &brand[11]);
        info->model[63] = '\0';

        /* Trim leading spaces */
        char* p = info->model;
        while (*p == ' ') p++;
        if (p != info->model) {
            int i = 0;
            while (p[i]) {
                info->model[i] = p[i];
                i++;
            }
            info->model[i] = '\0';
        }
    }

    /* CPU topology - simplified for now */
    info->core_count = 1;
    info->thread_count = 1;

    /* Estimate frequency (simplified - read from MSR in production) */
    info->frequency_mhz = 2400;  // Default estimate
}

/* Initialize CPU subsystem */
status_t hal_cpu_init(void) {
    /* Detect primary CPU */
    detect_cpu_features(&cpu_info_cache);
    cpu_info_cache.id = 0;

    /* Detect number of CPUs (simplified - use ACPI/MP tables in production) */
    detected_cpu_count = 1;

    return STATUS_OK;
}

/* Get number of CPUs */
uint32_t hal_cpu_count(void) {
    return detected_cpu_count;
}

/* Get CPU information */
status_t hal_cpu_info(uint32_t cpu_id, cpu_info_t* info) {
    if (!info || cpu_id >= detected_cpu_count) {
        return STATUS_INVALID;
    }

    *info = cpu_info_cache;
    info->id = cpu_id;

    return STATUS_OK;
}

/* Enable interrupts */
void hal_cpu_enable_interrupts(void) {
    __asm__ volatile("sti");
}

/* Disable interrupts */
void hal_cpu_disable_interrupts(void) {
    __asm__ volatile("cli");
}

/* Halt CPU */
void hal_cpu_halt(void) {
    __asm__ volatile("hlt");
}

/* Read timestamp counter */
uint64_t hal_cpu_read_timestamp(void) {
    return rdtsc();
}

/* Get current CPU ID */
u32 hal_cpu_id(void) {
    /* Simplified - return 0 for single CPU */
    return 0;
}

/* Interrupt control functions */
void hal_interrupt_enable(void) {
    __asm__ volatile("sti");
}

void hal_interrupt_disable(void) {
    __asm__ volatile("cli");
}

/* Basic interrupt initialization */
void hal_idt_init(void) {
    /* Simplified IDT setup - just clear interrupts for now */
    hal_interrupt_disable();
}

void hal_apic_init(void) {
    /* Simplified APIC init - no-op for now */
}

void hal_ioapic_init(void) {
    /* Simplified IO-APIC init - no-op for now */
}

void hal_enable_irq(int irq) {
    /* Simplified IRQ enable */
    (void)irq;
}

void hal_eoi(int irq) {
    /* Simplified end-of-interrupt */
    (void)irq;
}

/* Context switching */
void hal_arch_switch_context(arch_context_t** old_ctx, arch_context_t* new_ctx) {
    if (!new_ctx) {
        return; /* Nothing to switch to */
    }
    
    /* If old_ctx is NULL, this is the first switch to a new context */
    if (!old_ctx || !*old_ctx) {
        /* Just restore new context */
        __asm__ volatile (
            "movq %0, %%cr3\n"          /* Load page table */
            "movq %1, %%rsp\n"          /* Load stack pointer */
            "movq %2, %%rbp\n"          /* Load base pointer */
            "movq %3, %%rax\n"          /* Load rax */
            "movq %4, %%rbx\n"          /* Load rbx */
            "movq %5, %%rcx\n"          /* Load rcx */
            "movq %6, %%rdx\n"          /* Load rdx */
            "movq %7, %%rsi\n"          /* Load rsi */
            "movq %8, %%rdi\n"          /* Load rdi */
            "movq %9, %%r8\n"           /* Load r8 */
            "movq %10, %%r9\n"          /* Load r9 */
            "movq %11, %%r10\n"         /* Load r10 */
            "movq %12, %%r11\n"         /* Load r11 */
            "movq %13, %%r12\n"         /* Load r12 */
            "movq %14, %%r13\n"         /* Load r13 */
            "movq %15, %%r14\n"         /* Load r14 */
            "movq %16, %%r15\n"         /* Load r15 */
            "pushq %17\n"               /* Push rflags */
            "popfq\n"                   /* Restore rflags */
            "jmpq *%18\n"               /* Jump to rip */
            :
            : "m" (new_ctx->cr3), "m" (new_ctx->rsp), "m" (new_ctx->rbp),
              "m" (new_ctx->rax), "m" (new_ctx->rbx), "m" (new_ctx->rcx),
              "m" (new_ctx->rdx), "m" (new_ctx->rsi), "m" (new_ctx->rdi),
              "m" (new_ctx->r8), "m" (new_ctx->r9), "m" (new_ctx->r10),
              "m" (new_ctx->r11), "m" (new_ctx->r12), "m" (new_ctx->r13),
              "m" (new_ctx->r14), "m" (new_ctx->r15), "m" (new_ctx->rflags),
              "m" (new_ctx->rip)
            : "memory"
        );
        return;
    }
    
    /* Full context switch: save current, restore new */
    arch_context_t* old = *old_ctx;
    if (!old) {
        /* Allocate context structure for current thread */
        old = (arch_context_t*)vmm_kmalloc(sizeof(arch_context_t), 16);
        if (!old) return;
        *old_ctx = old;
    }
    
    /* Save current context and switch to new one */
    __asm__ volatile (
        /* Save current context */
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        "movq %%r8, %8\n"
        "movq %%r9, %9\n"
        "movq %%r10, %10\n"
        "movq %%r11, %11\n"
        "movq %%r12, %12\n"
        "movq %%r13, %13\n"
        "movq %%r14, %14\n"
        "movq %%r15, %15\n"
        "pushfq\n"
        "popq %16\n"                    /* Save rflags */
        "movq %%cr3, %%rax\n"
        "movq %%rax, %17\n"            /* Save cr3 */
        "leaq 1f(%%rip), %%rax\n"
        "movq %%rax, %18\n"            /* Save return address */
        
        /* Restore new context */
        "movq %19, %%cr3\n"            /* Load new page table */
        "movq %20, %%rsp\n"            /* Load new stack */
        "movq %21, %%rbp\n"            /* Load new base pointer */
        "movq %22, %%rax\n"            /* Load new rax */
        "movq %23, %%rbx\n"            /* Load new rbx */
        "movq %24, %%rcx\n"            /* Load new rcx */
        "movq %25, %%rdx\n"            /* Load new rdx */
        "movq %26, %%rsi\n"            /* Load new rsi */
        "movq %27, %%rdi\n"            /* Load new rdi */
        "movq %28, %%r8\n"             /* Load new r8 */
        "movq %29, %%r9\n"             /* Load new r9 */
        "movq %30, %%r10\n"            /* Load new r10 */
        "movq %31, %%r11\n"            /* Load new r11 */
        "movq %32, %%r12\n"            /* Load new r12 */
        "movq %33, %%r13\n"            /* Load new r13 */
        "movq %34, %%r14\n"            /* Load new r14 */
        "movq %35, %%r15\n"            /* Load new r15 */
        "pushq %36\n"                  /* Push new rflags */
        "popfq\n"                      /* Restore rflags */
        "jmpq *%37\n"                  /* Jump to new rip */
        "1:\n"                         /* Return point for old context */
        : "=m" (old->rax), "=m" (old->rbx), "=m" (old->rcx), "=m" (old->rdx),
          "=m" (old->rsi), "=m" (old->rdi), "=m" (old->rbp), "=m" (old->rsp),
          "=m" (old->r8), "=m" (old->r9), "=m" (old->r10), "=m" (old->r11),
          "=m" (old->r12), "=m" (old->r13), "=m" (old->r14), "=m" (old->r15),
          "=m" (old->rflags), "=m" (old->cr3), "=m" (old->rip)
        : "m" (new_ctx->cr3), "m" (new_ctx->rsp), "m" (new_ctx->rbp),
          "m" (new_ctx->rax), "m" (new_ctx->rbx), "m" (new_ctx->rcx),
          "m" (new_ctx->rdx), "m" (new_ctx->rsi), "m" (new_ctx->rdi),
          "m" (new_ctx->r8), "m" (new_ctx->r9), "m" (new_ctx->r10),
          "m" (new_ctx->r11), "m" (new_ctx->r12), "m" (new_ctx->r13),
          "m" (new_ctx->r14), "m" (new_ctx->r15), "m" (new_ctx->rflags),
          "m" (new_ctx->rip)
        : "memory", "rax"
    );
}

void hal_arch_prepare_kthread(arch_context_t** out_ctx, void (*entry)(void*), void* arg, void* stack_top) {
    if (!out_ctx || !entry || !stack_top) {
        return;
    }
    
    /* Allocate context structure */
    arch_context_t* ctx = (arch_context_t*)vmm_kmalloc(sizeof(arch_context_t), 16);
    if (!ctx) {
        *out_ctx = NULL;
        return;
    }
    
    /* Initialize context for kernel thread */
    k_memset(ctx, 0, sizeof(*ctx));
    
    /* Set up registers */
    ctx->rdi = (uint64_t)arg;           /* First argument in RDI */
    ctx->rsp = (uint64_t)stack_top - 8; /* Stack pointer (leave space for return address) */
    ctx->rbp = ctx->rsp;                /* Base pointer */
    ctx->rip = (uint64_t)entry;         /* Instruction pointer */
    ctx->rflags = 0x202;                /* Enable interrupts flag */
    
    /* Use current page table for kernel threads */
    uint64_t current_cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r" (current_cr3));
    ctx->cr3 = current_cr3;
    
    /* Put a dummy return address on stack */
    uint64_t* stack = (uint64_t*)ctx->rsp;
    *stack = 0;  /* Kernel threads shouldn't return */
    
    *out_ctx = ctx;
}

/* Memory mapping */
void hal_early_map_physical(uint64_t phys, uint64_t virt, uint64_t size, uint64_t flags) {
    /* Simplified memory mapping - identity mapping assumed */
    (void)phys;
    (void)virt;
    (void)size;
    (void)flags;
}

/* Additional timer functions */
void hal_timer_set_periodic(uint64_t hz) {
    /* Simplified timer setup */
    (void)hz;
}

uint64_t hal_timer_ticks(void) {
    /* Return timestamp as ticks */
    return hal_cpu_read_timestamp();
}
