/**
 * SMP (Symmetric Multi-Processing) Implementation for LimitlessOS
 * 
 * This file implements the core SMP functionality including CPU detection,
 * AP startup, IPI handling, and per-CPU data management.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "smp.h"
#include "mm/mm.h"
#include "apic.h"
#include "acpi.h"
#include "kernel.h"
#include <string.h>

/* Global SMP state */
cpu_info_t cpu_data[MAX_CPUS] __attribute__((aligned(64)));
uint32_t nr_cpus_online = 0;
uint32_t nr_cpus_possible = 0;
cpu_mask_t cpu_online_mask;
cpu_mask_t cpu_possible_mask;
uint32_t boot_cpu_id = 0;

/* NUMA topology */
numa_node_t numa_nodes[MAX_NUMA_NODES];
uint32_t nr_numa_nodes = 0;

/* CPU capabilities */
uint32_t cpu_capabilities[MAX_CPUS];

/* Trampoline code for AP startup */
extern void smp_trampoline_start(void);
extern void smp_trampoline_end(void);
extern volatile uint32_t smp_trampoline_target;

/* Boot synchronization */
static volatile uint32_t smp_boot_lock = 0;
static volatile uint32_t cpus_booted = 0;

/**
 * Initialize SMP subsystem
 */
int smp_init(void) {
    kprintf("[SMP] Initializing SMP support...\n");
    
    /* Clear CPU masks */
    cpu_mask_clear(&cpu_online_mask);
    cpu_mask_clear(&cpu_possible_mask);
    
    /* Initialize per-CPU data */
    memset(cpu_data, 0, sizeof(cpu_data));
    memset(cpu_capabilities, 0, sizeof(cpu_capabilities));
    
    /* Set up boot CPU */
    boot_cpu_id = 0;  /* Assume boot CPU is 0 */
    cpu_info_t *boot_cpu = &cpu_data[boot_cpu_id];
    
    boot_cpu->cpu_id = boot_cpu_id;
    boot_cpu->apic_id = apic_get_id();
    boot_cpu->state = CPU_ONLINE;
    boot_cpu->boot_time = get_ticks();
    
    /* Allocate stacks for boot CPU */
    boot_cpu->kernel_stack = pmm_alloc_page();
    boot_cpu->interrupt_stack = pmm_alloc_page();
    boot_cpu->exception_stack = pmm_alloc_page();
    
    if (!boot_cpu->kernel_stack || !boot_cpu->interrupt_stack || !boot_cpu->exception_stack) {
        kprintf("[SMP] Failed to allocate stacks for boot CPU\n");
        return -1;
    }
    
    /* Detect CPU capabilities */
    cpu_detect_capabilities(boot_cpu_id);
    
    /* Set boot CPU as online and possible */
    cpu_mask_set_cpu(boot_cpu_id, &cpu_online_mask);
    cpu_mask_set_cpu(boot_cpu_id, &cpu_possible_mask);
    nr_cpus_online = 1;
    nr_cpus_possible = 1;
    
    /* Detect additional CPUs from ACPI */
    smp_detect_cpus();
    
    /* Initialize NUMA topology */
    numa_init();
    
    /* Start application processors */
    smp_boot_secondary_cpus();
    
    kprintf("[SMP] SMP initialization complete\n");
    kprintf("[SMP] Boot CPU: %u, Total CPUs: %u, Online: %u\n", 
            boot_cpu_id, nr_cpus_possible, nr_cpus_online);
    
    return 0;
}

/**
 * Detect CPUs from ACPI MADT
 */
void smp_detect_cpus(void) {
    kprintf("[SMP] Detecting CPUs from ACPI...\n");
    
    /* Parse ACPI MADT to find all CPUs */
    uint32_t cpu_count = acpi_get_cpu_count();
    
    for (uint32_t i = 1; i < cpu_count && i < MAX_CPUS; i++) {
        uint32_t apic_id = acpi_get_cpu_apic_id(i);
        
        cpu_info_t *cpu = &cpu_data[i];
        cpu->cpu_id = i;
        cpu->apic_id = apic_id;
        cpu->state = CPU_OFFLINE;
        
        /* Mark as possible */
        cpu_mask_set_cpu(i, &cpu_possible_mask);
        nr_cpus_possible++;
        
        kprintf("[SMP] Found CPU %u (APIC ID %u)\n", i, apic_id);
    }
    
    kprintf("[SMP] Detected %u total CPUs\n", nr_cpus_possible);
}

/**
 * Boot secondary CPUs
 */
void smp_boot_secondary_cpus(void) {
    kprintf("[SMP] Starting secondary CPUs...\n");
    
    /* Set up trampoline code in low memory */
    void *trampoline = (void*)0x8000;  /* Standard location */
    size_t trampoline_size = (uintptr_t)smp_trampoline_end - (uintptr_t)smp_trampoline_start;
    
    memcpy(trampoline, smp_trampoline_start, trampoline_size);
    
    /* Boot each secondary CPU */
    for (uint32_t cpu_id = 1; cpu_id < nr_cpus_possible; cpu_id++) {
        if (cpu_id == boot_cpu_id) continue;
        
        if (smp_boot_cpu(cpu_id) == 0) {
            kprintf("[SMP] Successfully started CPU %u\n", cpu_id);
        } else {
            kprintf("[SMP] Failed to start CPU %u\n", cpu_id);
        }
    }
    
    kprintf("[SMP] Secondary CPU startup complete\n");
}

/**
 * Boot a specific CPU
 */
int smp_boot_cpu(uint32_t cpu_id) {
    if (cpu_id >= MAX_CPUS || cpu_id == boot_cpu_id) {
        return -1;
    }
    
    cpu_info_t *cpu = &cpu_data[cpu_id];
    
    /* Allocate stacks */
    cpu->kernel_stack = pmm_alloc_page();
    cpu->interrupt_stack = pmm_alloc_page();
    cpu->exception_stack = pmm_alloc_page();
    
    if (!cpu->kernel_stack || !cpu->interrupt_stack || !cpu->exception_stack) {
        kprintf("[SMP] Failed to allocate stacks for CPU %u\n", cpu_id);
        return -1;
    }
    
    /* Set up trampoline target */
    smp_trampoline_target = (uint32_t)smp_init_secondary;
    
    /* Mark CPU as booting */
    cpu->state = CPU_BOOTING;
    
    /* Send INIT IPI */
    apic_send_init_ipi(cpu->apic_id);
    
    /* Wait 10ms */
    udelay(10000);
    
    /* Send STARTUP IPI */
    apic_send_startup_ipi(cpu->apic_id, 0x08);  /* Trampoline at 0x8000 */
    
    /* Wait for CPU to come online */
    uint32_t timeout = 1000;  /* 1 second timeout */
    while (timeout > 0 && cpu->state == CPU_BOOTING) {
        udelay(1000);  /* 1ms */
        timeout--;
    }
    
    if (cpu->state != CPU_ONLINE) {
        kprintf("[SMP] CPU %u failed to come online\n", cpu_id);
        return -1;
    }
    
    /* Add to online mask */
    cpu_mask_set_cpu(cpu_id, &cpu_online_mask);
    atomic_inc((volatile int*)&nr_cpus_online);
    
    return 0;
}

/**
 * Secondary CPU initialization (called by each AP)
 */
void smp_init_secondary(void) {
    uint32_t cpu_id = apic_get_id();  /* Get our APIC ID */
    
    /* Find our CPU data structure */
    cpu_info_t *cpu = NULL;
    for (uint32_t i = 0; i < MAX_CPUS; i++) {
        if (cpu_data[i].apic_id == cpu_id) {
            cpu = &cpu_data[i];
            break;
        }
    }
    
    if (!cpu) {
        /* This shouldn't happen */
        while (1) asm volatile("hlt");
    }
    
    /* Set up our stack */
    uint32_t stack_top = (uint32_t)cpu->kernel_stack + PAGE_SIZE - 16;
    asm volatile("movl %0, %%esp" :: "r"(stack_top));
    
    /* Initialize local APIC */
    apic_init_secondary();
    
    /* Load GDT and IDT */
    /* TODO: Set up per-CPU GDT/IDT */
    
    /* Enable interrupts */
    asm volatile("sti");
    
    /* Detect CPU capabilities */
    cpu_detect_capabilities(cpu->cpu_id);
    
    /* Mark as online */
    cpu->state = CPU_ONLINE;
    cpu->boot_time = get_ticks();
    
    /* Initialize per-CPU scheduler */
    /* TODO: Initialize per-CPU run queue */
    
    kprintf("[SMP] CPU %u online (APIC %u)\n", cpu->cpu_id, cpu->apic_id);
    
    /* Enter idle loop */
    smp_enter_idle();
}

/**
 * Get current processor ID
 */
uint32_t smp_processor_id(void) {
    if (!smp_enabled()) {
        return 0;
    }
    
    uint32_t apic_id = apic_get_id();
    
    /* Find CPU by APIC ID */
    for (uint32_t i = 0; i < MAX_CPUS; i++) {
        if (cpu_data[i].apic_id == apic_id && cpu_data[i].state != CPU_OFFLINE) {
            return i;
        }
    }
    
    return 0;  /* Default to boot CPU */
}

/**
 * Get CPU data for specific CPU
 */
cpu_info_t* smp_cpu_data(uint32_t cpu_id) {
    if (cpu_id >= MAX_CPUS) {
        return NULL;
    }
    return &cpu_data[cpu_id];
}

/**
 * Check if CPU is online
 */
bool smp_cpu_online(uint32_t cpu_id) {
    return cpu_mask_test_cpu(cpu_id, &cpu_online_mask);
}

/**
 * Set CPU state
 */
void smp_set_cpu_state(uint32_t cpu_id, cpu_state_t state) {
    if (cpu_id >= MAX_CPUS) return;
    
    cpu_data[cpu_id].state = state;
    
    if (state == CPU_ONLINE) {
        cpu_mask_set_cpu(cpu_id, &cpu_online_mask);
    } else {
        cpu_mask_clear_cpu(cpu_id, &cpu_online_mask);
    }
}

/**
 * Send IPI to specific CPU
 */
void smp_send_ipi(uint32_t cpu_id, uint32_t ipi_type) {
    if (cpu_id >= MAX_CPUS || !smp_cpu_online(cpu_id)) {
        return;
    }
    
    cpu_info_t *cpu = &cpu_data[cpu_id];
    
    /* Set pending IPI */
    atomic_inc((volatile int*)&cpu->ipi_pending);
    cpu->ipi_pending |= ipi_type;
    
    /* Send hardware IPI */
    apic_send_ipi(cpu->apic_id, 0xF0 + ipi_type);  /* IPI vector base at 0xF0 */
}

/**
 * Send IPI to CPUs in mask
 */
void smp_send_ipi_mask(const cpu_mask_t *mask, uint32_t ipi_type) {
    uint32_t cpu;
    for_each_cpu_in_mask(cpu, mask) {
        if (cpu != smp_processor_id()) {
            smp_send_ipi(cpu, ipi_type);
        }
    }
}

/**
 * Send IPI to all CPUs
 */
void smp_send_ipi_all(uint32_t ipi_type) {
    smp_send_ipi_mask(&cpu_online_mask, ipi_type);
}

/**
 * Send IPI to all CPUs except self
 */
void smp_send_ipi_all_but_self(uint32_t ipi_type) {
    uint32_t current_cpu = smp_processor_id();
    uint32_t cpu;
    
    for_each_online_cpu(cpu) {
        if (cpu != current_cpu) {
            smp_send_ipi(cpu, ipi_type);
        }
    }
}

/**
 * CPU mask operations
 */
void cpu_mask_clear(cpu_mask_t *mask) {
    memset(mask, 0, sizeof(cpu_mask_t));
}

void cpu_mask_set_cpu(uint32_t cpu, cpu_mask_t *mask) {
    if (cpu >= MAX_CPUS) return;
    
    uint32_t word = cpu / (sizeof(unsigned long) * 8);
    uint32_t bit = cpu % (sizeof(unsigned long) * 8);
    
    mask->bits[word] |= (1UL << bit);
}

void cpu_mask_clear_cpu(uint32_t cpu, cpu_mask_t *mask) {
    if (cpu >= MAX_CPUS) return;
    
    uint32_t word = cpu / (sizeof(unsigned long) * 8);
    uint32_t bit = cpu % (sizeof(unsigned long) * 8);
    
    mask->bits[word] &= ~(1UL << bit);
}

bool cpu_mask_test_cpu(uint32_t cpu, const cpu_mask_t *mask) {
    if (cpu >= MAX_CPUS) return false;
    
    uint32_t word = cpu / (sizeof(unsigned long) * 8);
    uint32_t bit = cpu % (sizeof(unsigned long) * 8);
    
    return (mask->bits[word] & (1UL << bit)) != 0;
}

uint32_t cpu_mask_first(const cpu_mask_t *mask) {
    for (uint32_t i = 0; i < MAX_CPUS; i++) {
        if (cpu_mask_test_cpu(i, mask)) {
            return i;
        }
    }
    return MAX_CPUS;
}

uint32_t cpu_mask_next(uint32_t cpu, const cpu_mask_t *mask) {
    for (uint32_t i = cpu + 1; i < MAX_CPUS; i++) {
        if (cpu_mask_test_cpu(i, mask)) {
            return i;
        }
    }
    return MAX_CPUS;
}

uint32_t cpu_mask_weight(const cpu_mask_t *mask) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_CPUS; i++) {
        if (cpu_mask_test_cpu(i, mask)) {
            count++;
        }
    }
    return count;
}

bool cpu_mask_empty(const cpu_mask_t *mask) {
    for (size_t i = 0; i < sizeof(mask->bits) / sizeof(mask->bits[0]); i++) {
        if (mask->bits[i] != 0) {
            return false;
        }
    }
    return true;
}

/**
 * Spinlock implementation
 */
void spin_lock_init(spinlock_t *lock) {
    lock->slock = 0;
    lock->owner_cpu = 0;
    lock->owner_pc = NULL;
}

void spin_lock(spinlock_t *lock) {
    while (1) {
        /* Try to acquire lock */
        if (!__sync_lock_test_and_set(&lock->slock, 1)) {
            lock->owner_cpu = smp_processor_id();
            lock->owner_pc = __builtin_return_address(0);
            break;
        }
        
        /* Spin with pause instruction for better performance */
        while (lock->slock) {
            asm volatile("pause");
        }
    }
}

void spin_unlock(spinlock_t *lock) {
    lock->owner_cpu = 0;
    lock->owner_pc = NULL;
    __sync_lock_release(&lock->slock);
}

bool spin_trylock(spinlock_t *lock) {
    if (!__sync_lock_test_and_set(&lock->slock, 1)) {
        lock->owner_cpu = smp_processor_id();
        lock->owner_pc = __builtin_return_address(0);
        return true;
    }
    return false;
}

/**
 * Detect CPU topology from CPUID
 */
void smp_detect_topology(void) {
    kprintf("[SMP] Detecting CPU topology...\n");
    
    for (uint32_t cpu = 0; cpu < nr_cpus_possible; cpu++) {
        cpu_info_t *cpu_info = &cpu_data[cpu];
        
        /* Use CPUID to detect topology */
        uint32_t eax, ebx, ecx, edx;
        
        /* Get topology info from CPUID.0Bh */
        eax = 0x0B;
        ecx = 0;
        asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                     : "a"(eax), "c"(ecx));
        
        if (eax != 0) {
            /* Extended topology enumeration supported */
            cpu_info->topology.thread_id = cpu_info->apic_id & ((1 << (eax & 0x1F)) - 1);
            
            ecx = 1;
            asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                         : "a"(0x0B), "c"(ecx));
            
            cpu_info->topology.core_id = (cpu_info->apic_id >> (eax & 0x1F)) & 
                                        ((1 << ((eax & 0x1F) - (cpu_info->topology.thread_id ? 1 : 0))) - 1);
            
            cpu_info->topology.package_id = cpu_info->apic_id >> (eax & 0x1F);
        } else {
            /* Fall back to basic detection */
            cpu_info->topology.thread_id = 0;
            cpu_info->topology.core_id = cpu;
            cpu_info->topology.package_id = 0;
        }
        
        /* Detect if SMT is enabled */
        eax = 1;
        asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
        cpu_info->topology.is_smt = (edx & (1 << 28)) && (ebx >> 16) > 1;
        
        kprintf("[SMP] CPU %u: Package %u, Core %u, Thread %u, SMT %s\n",
                cpu, cpu_info->topology.package_id, cpu_info->topology.core_id,
                cpu_info->topology.thread_id, cpu_info->topology.is_smt ? "yes" : "no");
    }
}

/**
 * CPU capability detection
 */
void cpu_detect_capabilities(uint32_t cpu_id) {
    if (cpu_id >= MAX_CPUS) return;
    
    uint32_t eax, ebx, ecx, edx;
    uint32_t caps = 0;
    
    /* Get basic features */
    eax = 1;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
    
    if (edx & (1 << 0))  caps |= CPU_CAP_FPU;
    if (edx & (1 << 1))  caps |= CPU_CAP_VME;
    if (edx & (1 << 2))  caps |= CPU_CAP_DE;
    if (edx & (1 << 3))  caps |= CPU_CAP_PSE;
    if (edx & (1 << 4))  caps |= CPU_CAP_TSC;
    if (edx & (1 << 5))  caps |= CPU_CAP_MSR;
    if (edx & (1 << 6))  caps |= CPU_CAP_PAE;
    if (edx & (1 << 7))  caps |= CPU_CAP_MCE;
    if (edx & (1 << 8))  caps |= CPU_CAP_CX8;
    if (edx & (1 << 9))  caps |= CPU_CAP_APIC;
    if (edx & (1 << 11)) caps |= CPU_CAP_SEP;
    if (edx & (1 << 12)) caps |= CPU_CAP_MTRR;
    if (edx & (1 << 13)) caps |= CPU_CAP_PGE;
    if (edx & (1 << 14)) caps |= CPU_CAP_MCA;
    if (edx & (1 << 15)) caps |= CPU_CAP_CMOV;
    if (edx & (1 << 23)) caps |= CPU_CAP_MMX;
    if (edx & (1 << 24)) caps |= CPU_CAP_FXSR;
    if (edx & (1 << 25)) caps |= CPU_CAP_XMM;
    if (edx & (1 << 26)) caps |= CPU_CAP_XMM2;
    
    cpu_capabilities[cpu_id] = caps;
    
    /* Detect cache sizes */
    eax = 0x80000006;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
    
    cpu_data[cpu_id].cache_line_size = ecx & 0xFF;
    cpu_data[cpu_id].l2_cache_size = (ecx >> 16) * 1024;  /* KB to bytes */
    
    /* Get TSC frequency if possible */
    /* This is CPU-specific and may require additional detection */
    cpu_data[cpu_id].tsc_freq = 0;  /* Will be calibrated later */
}

/**
 * Check if CPU has capability
 */
bool cpu_has_capability(uint32_t cpu_id, uint32_t capability) {
    if (cpu_id >= MAX_CPUS) return false;
    return (cpu_capabilities[cpu_id] & capability) != 0;
}

/**
 * NUMA initialization
 */
void numa_init(void) {
    kprintf("[NUMA] Initializing NUMA topology...\n");
    
    /* For now, create single NUMA node containing all CPUs */
    numa_nodes[0].node_id = 0;
    cpu_mask_clear(&numa_nodes[0].cpu_mask);
    
    for (uint32_t cpu = 0; cpu < nr_cpus_possible; cpu++) {
        cpu_mask_set_cpu(cpu, &numa_nodes[0].cpu_mask);
        cpu_data[cpu].topology.numa_node = 0;
    }
    
    numa_nodes[0].memory_size = 0;  /* Will be set by memory manager */
    numa_nodes[0].free_memory = 0;
    
    /* Set distances (0 to self, 10 is standard local distance) */
    numa_nodes[0].distance[0] = 10;
    
    nr_numa_nodes = 1;
    
    kprintf("[NUMA] Single NUMA node topology initialized\n");
}

/**
 * Enter idle state
 */
void smp_enter_idle(void) {
    uint32_t cpu_id = smp_processor_id();
    cpu_data[cpu_id].state = CPU_IDLE;
    
    while (cpu_data[cpu_id].state == CPU_IDLE) {
        /* Check for pending IPIs */
        if (cpu_data[cpu_id].ipi_pending) {
            /* Handle IPIs */
            /* TODO: Implement IPI handlers */
        }
        
        /* Enter low-power state */
        asm volatile("hlt");
    }
}

/**
 * Exit idle state
 */
void smp_exit_idle(void) {
    uint32_t cpu_id = smp_processor_id();
    if (cpu_data[cpu_id].state == CPU_IDLE) {
        cpu_data[cpu_id].state = CPU_ONLINE;
    }
}

/**
 * CPU relax instruction for spinloops
 */
void smp_cpu_relax(void) {
    asm volatile("pause" ::: "memory");
}

/**
 * Debug functions
 */
void smp_dump_cpu_info(uint32_t cpu_id) {
    if (cpu_id >= MAX_CPUS) return;
    
    cpu_info_t *cpu = &cpu_data[cpu_id];
    
    kprintf("CPU %u:\n", cpu_id);
    kprintf("  APIC ID: %u\n", cpu->apic_id);
    kprintf("  State: %d\n", cpu->state);
    kprintf("  Package: %u, Core: %u, Thread: %u\n",
            cpu->topology.package_id, cpu->topology.core_id, cpu->topology.thread_id);
    kprintf("  NUMA Node: %u\n", cpu->topology.numa_node);
    kprintf("  Capabilities: 0x%08X\n", cpu_capabilities[cpu_id]);
    kprintf("  Cache Line Size: %u bytes\n", cpu->cache_line_size);
    kprintf("  L2 Cache: %u KB\n", cpu->l2_cache_size / 1024);
}

void smp_dump_all_cpus(void) {
    kprintf("[SMP] CPU Information:\n");
    kprintf("Total CPUs: %u, Online: %u\n", nr_cpus_possible, nr_cpus_online);
    
    for (uint32_t cpu = 0; cpu < nr_cpus_possible; cpu++) {
        smp_dump_cpu_info(cpu);
    }
}

/* Check if SMP is enabled */
bool smp_enabled(void) {
    return nr_cpus_possible > 1;
}