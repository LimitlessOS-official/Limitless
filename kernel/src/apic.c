/**
 * Advanced Programmable Interrupt Controller (APIC) Implementation
 * 
 * This file provides the implementation of Local APIC and I/O APIC support
 * for multi-processor systems.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "apic.h"
#include "acpi.h"
#include "kernel.h"
#include <string.h>

/* Global APIC state */
local_apic_t local_apic;
ioapic_t ioapics[MAX_IOAPICS];
uint32_t num_ioapics = 0;
irq_route_t irq_routing[256];

/* Vector allocation bitmap */
static uint32_t vector_bitmap[8] = {0};  /* 256 bits for vectors 0-255 */

/* Default Local APIC base address */
static uintptr_t apic_base = 0xFEE00000;

/**
 * Initialize APIC subsystem
 */
int apic_init(void) {
    kprintf("[APIC] Initializing APIC subsystem...\n");
    
    /* Check if APIC is supported */
    if (!cpu_has_apic()) {
        kprintf("[APIC] APIC not supported by CPU\n");
        return -1;
    }
    
    /* Get APIC base address from MSR */
    uint64_t apic_base_msr = read_msr(MSR_APIC_BASE);
    apic_base = apic_base_msr & 0xFFFFF000;
    
    kprintf("[APIC] APIC base address: 0x%08X\n", (uint32_t)apic_base);
    
    /* Check if this is the BSP */
    bool is_bsp = (apic_base_msr & APIC_BASE_BSP) != 0;
    kprintf("[APIC] %s processor detected\n", is_bsp ? "Bootstrap" : "Application");
    
    /* Initialize Local APIC */
    memset(&local_apic, 0, sizeof(local_apic));
    local_apic.base_address = apic_base;
    local_apic.enabled = false;
    
    /* Enable Local APIC */
    apic_enable();
    
    /* Get Local APIC ID and version */
    local_apic.id = apic_get_id();
    local_apic.version = apic_read(APIC_REG_VERSION);
    
    kprintf("[APIC] Local APIC ID: %u, Version: 0x%02X\n", 
            local_apic.id, local_apic.version & 0xFF);
    
    /* Set spurious interrupt vector */
    apic_set_spurious_vector(APIC_VECTOR_SPURIOUS);
    
    /* Initialize vector allocation bitmap */
    memset(vector_bitmap, 0, sizeof(vector_bitmap));
    
    /* Mark system vectors as used */
    for (int i = 0; i < 0x20; i++) {
        vector_bitmap[i / 32] |= (1 << (i % 32));
    }
    
    /* Mark APIC vectors as used */
    vector_bitmap[APIC_VECTOR_SPURIOUS / 32] |= (1 << (APIC_VECTOR_SPURIOUS % 32));
    vector_bitmap[APIC_VECTOR_TIMER / 32] |= (1 << (APIC_VECTOR_TIMER % 32));
    vector_bitmap[APIC_VECTOR_ERROR / 32] |= (1 << (APIC_VECTOR_ERROR % 32));
    
    /* Initialize I/O APICs if this is the BSP */
    if (is_bsp) {
        ioapic_init();
        apic_setup_irq_routing();
    }
    
    /* Set up Local Vector Table */
    apic_setup_lvt_timer(APIC_VECTOR_TIMER, APIC_TIMER_ONE_SHOT);
    apic_setup_lvt_error(APIC_VECTOR_ERROR);
    
    /* Mask LINT0 and LINT1 for now */
    apic_mask_lvt_entry(APIC_REG_LINT0);
    apic_mask_lvt_entry(APIC_REG_LINT1);
    
    kprintf("[APIC] APIC initialization complete\n");
    return 0;
}

/**
 * Initialize APIC on secondary processor
 */
void apic_init_secondary(void) {
    /* Enable Local APIC */
    apic_enable();
    
    /* Set spurious interrupt vector */
    apic_set_spurious_vector(APIC_VECTOR_SPURIOUS);
    
    /* Set up Local Vector Table */
    apic_setup_lvt_timer(APIC_VECTOR_TIMER, APIC_TIMER_ONE_SHOT);
    apic_setup_lvt_error(APIC_VECTOR_ERROR);
    
    /* Mask LINT0 and LINT1 */
    apic_mask_lvt_entry(APIC_REG_LINT0);
    apic_mask_lvt_entry(APIC_REG_LINT1);
    
    kprintf("[APIC] Secondary processor APIC initialized\n");
}

/**
 * Get Local APIC ID
 */
uint32_t apic_get_id(void) {
    if (local_apic.x2apic_enabled) {
        return apic_x2apic_read(MSR_X2APIC_APICID);
    } else {
        return (apic_read(APIC_REG_ID) >> 24) & 0xFF;
    }
}

/**
 * Enable Local APIC
 */
void apic_enable(void) {
    /* Enable APIC via MSR */
    uint64_t apic_base_msr = read_msr(MSR_APIC_BASE);
    apic_base_msr |= APIC_BASE_GLOBAL_ENABLE;
    write_msr(MSR_APIC_BASE, apic_base_msr);
    
    /* Enable APIC via spurious interrupt vector register */
    uint32_t spurious = apic_read(APIC_REG_SIVR);
    spurious |= (1 << 8);  /* APIC Software Enable */
    apic_write(APIC_REG_SIVR, spurious);
    
    local_apic.enabled = true;
    kprintf("[APIC] Local APIC enabled\n");
}

/**
 * Disable Local APIC
 */
void apic_disable(void) {
    /* Disable APIC via spurious interrupt vector register */
    uint32_t spurious = apic_read(APIC_REG_SIVR);
    spurious &= ~(1 << 8);  /* Clear APIC Software Enable */
    apic_write(APIC_REG_SIVR, spurious);
    
    local_apic.enabled = false;
    kprintf("[APIC] Local APIC disabled\n");
}

/**
 * Check if Local APIC is enabled
 */
bool apic_is_enabled(void) {
    return local_apic.enabled;
}

/**
 * Set spurious interrupt vector
 */
void apic_set_spurious_vector(uint32_t vector) {
    uint32_t spurious = apic_read(APIC_REG_SIVR);
    spurious = (spurious & 0xFFFFFF00) | vector;
    spurious |= (1 << 8);  /* Enable bit */
    apic_write(APIC_REG_SIVR, spurious);
    
    local_apic.spurious_vector = vector;
}

/**
 * Send End of Interrupt
 */
void apic_eoi(void) {
    if (local_apic.x2apic_enabled) {
        apic_x2apic_write(MSR_X2APIC_EOI, 0);
    } else {
        apic_write(APIC_REG_EOI, 0);
    }
}

/**
 * Read Local APIC register
 */
uint32_t apic_read(uint32_t reg) {
    return mmio_read32(apic_base + reg);
}

/**
 * Write Local APIC register
 */
void apic_write(uint32_t reg, uint32_t value) {
    mmio_write32(apic_base + reg, value);
}

/**
 * Send IPI to specific APIC ID
 */
void apic_send_ipi(uint32_t dest_apic_id, uint32_t vector) {
    if (local_apic.x2apic_enabled) {
        apic_x2apic_send_ipi(dest_apic_id, vector);
        return;
    }
    
    /* Wait for any previous IPI to complete */
    while (apic_read(APIC_REG_ICR_LOW) & (1 << 12)) {
        /* Delivery status bit is set, wait */
    }
    
    /* Set destination */
    apic_write(APIC_REG_ICR_HIGH, dest_apic_id << 24);
    
    /* Send IPI */
    uint32_t icr_low = vector | 
                      (ICR_DELIVERY_FIXED << 8) |
                      (ICR_DEST_NO_SHORTHAND << 18) |
                      (ICR_LEVEL_ASSERT << 14) |
                      (ICR_TRIGGER_EDGE << 15);
    
    apic_write(APIC_REG_ICR_LOW, icr_low);
}

/**
 * Send INIT IPI
 */
void apic_send_init_ipi(uint32_t dest_apic_id) {
    /* Wait for any previous IPI to complete */
    while (apic_read(APIC_REG_ICR_LOW) & (1 << 12)) {
        /* Delivery status bit is set, wait */
    }
    
    /* Set destination */
    apic_write(APIC_REG_ICR_HIGH, dest_apic_id << 24);
    
    /* Send INIT IPI */
    uint32_t icr_low = (ICR_DELIVERY_INIT << 8) |
                      (ICR_DEST_NO_SHORTHAND << 18) |
                      (ICR_LEVEL_ASSERT << 14) |
                      (ICR_TRIGGER_EDGE << 15);
    
    apic_write(APIC_REG_ICR_LOW, icr_low);
}

/**
 * Send STARTUP IPI
 */
void apic_send_startup_ipi(uint32_t dest_apic_id, uint32_t vector) {
    /* Wait for any previous IPI to complete */
    while (apic_read(APIC_REG_ICR_LOW) & (1 << 12)) {
        /* Delivery status bit is set, wait */
    }
    
    /* Set destination */
    apic_write(APIC_REG_ICR_HIGH, dest_apic_id << 24);
    
    /* Send STARTUP IPI */
    uint32_t icr_low = vector |
                      (ICR_DELIVERY_STARTUP << 8) |
                      (ICR_DEST_NO_SHORTHAND << 18) |
                      (ICR_LEVEL_ASSERT << 14) |
                      (ICR_TRIGGER_EDGE << 15);
    
    apic_write(APIC_REG_ICR_LOW, icr_low);
}

/**
 * Initialize I/O APICs
 */
int ioapic_init(void) {
    kprintf("[IOAPIC] Initializing I/O APICs...\n");
    
    /* Clear I/O APIC structures */
    memset(ioapics, 0, sizeof(ioapics));
    num_ioapics = 0;
    
    /* Detect I/O APICs from ACPI */
    /* This is a simplified detection - real implementation would parse MADT */
    
    /* For now, assume one I/O APIC at standard address */
    ioapics[0].id = 0;
    ioapics[0].base_address = 0xFEC00000;  /* Standard I/O APIC address */
    ioapics[0].global_irq_base = 0;
    ioapics[0].enabled = true;
    
    /* Read I/O APIC version and capabilities */
    uint32_t version = ioapic_read(0, IOAPIC_REG_VERSION);
    ioapics[0].version = version & 0xFF;
    ioapics[0].max_redirection_entries = ((version >> 16) & 0xFF) + 1;
    
    num_ioapics = 1;
    
    kprintf("[IOAPIC] I/O APIC 0: Version 0x%02X, %u entries\n",
            ioapics[0].version, ioapics[0].max_redirection_entries);
    
    /* Mask all interrupts initially */
    for (uint32_t i = 0; i < ioapics[0].max_redirection_entries; i++) {
        ioapic_write(0, IOAPIC_REDTBL_BASE + i * 2, IOAPIC_INT_DISABLED);
        ioapic_write(0, IOAPIC_REDTBL_BASE + i * 2 + 1, 0);
    }
    
    return 0;
}

/**
 * Read I/O APIC register
 */
uint32_t ioapic_read(uint32_t ioapic_id, uint32_t reg) {
    if (ioapic_id >= num_ioapics || !ioapics[ioapic_id].enabled) {
        return 0;
    }
    
    uintptr_t base = ioapics[ioapic_id].base_address;
    
    /* Write register index */
    mmio_write32(base, reg & 0xFF);
    
    /* Read data */
    return mmio_read32(base + 0x10);
}

/**
 * Write I/O APIC register
 */
void ioapic_write(uint32_t ioapic_id, uint32_t reg, uint32_t value) {
    if (ioapic_id >= num_ioapics || !ioapics[ioapic_id].enabled) {
        return;
    }
    
    uintptr_t base = ioapics[ioapic_id].base_address;
    
    /* Write register index */
    mmio_write32(base, reg & 0xFF);
    
    /* Write data */
    mmio_write32(base + 0x10, value);
}

/**
 * Set up IRQ routing
 */
int apic_setup_irq_routing(void) {
    kprintf("[APIC] Setting up IRQ routing...\n");
    
    /* Clear routing table */
    memset(irq_routing, 0, sizeof(irq_routing));
    
    /* Set up legacy IRQ routing */
    apic_route_legacy_irqs();
    
    return 0;
}

/**
 * Route legacy IRQs (0-15) to I/O APIC
 */
void apic_route_legacy_irqs(void) {
    for (uint8_t irq = 0; irq < 16; irq++) {
        uint8_t vector = APIC_VECTOR_BASE + irq;
        
        /* Route to BSP (APIC ID 0) for now */
        apic_route_irq(irq, vector, 0);
        
        kprintf("[APIC] IRQ %u -> Vector 0x%02X -> CPU 0\n", irq, vector);
    }
}

/**
 * Route specific IRQ
 */
void apic_route_irq(uint8_t irq, uint8_t vector, uint32_t dest_apic_id) {
    /* Find I/O APIC that handles this IRQ */
    uint32_t ioapic_id = 0;  /* For now, assume I/O APIC 0 handles all */
    uint32_t pin = irq;      /* 1:1 mapping for legacy IRQs */
    
    /* Set up routing entry */
    irq_routing[irq].irq = irq;
    irq_routing[irq].ioapic_id = ioapic_id;
    irq_routing[irq].ioapic_pin = pin;
    irq_routing[irq].vector = vector;
    irq_routing[irq].flags = 0;
    irq_routing[irq].enabled = true;
    
    /* Configure I/O APIC redirection entry */
    uint32_t low = vector;
    uint32_t high = dest_apic_id << 24;
    
    ioapic_write(ioapic_id, IOAPIC_REDTBL_BASE + pin * 2, low | IOAPIC_INT_DISABLED);
    ioapic_write(ioapic_id, IOAPIC_REDTBL_BASE + pin * 2 + 1, high);
    
    /* Enable the interrupt */
    ioapic_write(ioapic_id, IOAPIC_REDTBL_BASE + pin * 2, low);
}

/**
 * Mask IRQ
 */
void ioapic_mask_irq(uint32_t irq) {
    irq_route_t *route = apic_find_irq_route(irq);
    if (!route) return;
    
    uint32_t reg = IOAPIC_REDTBL_BASE + route->ioapic_pin * 2;
    uint32_t value = ioapic_read(route->ioapic_id, reg);
    value |= IOAPIC_INT_DISABLED;
    ioapic_write(route->ioapic_id, reg, value);
}

/**
 * Unmask IRQ
 */
void ioapic_unmask_irq(uint32_t irq) {
    irq_route_t *route = apic_find_irq_route(irq);
    if (!route) return;
    
    uint32_t reg = IOAPIC_REDTBL_BASE + route->ioapic_pin * 2;
    uint32_t value = ioapic_read(route->ioapic_id, reg);
    value &= ~IOAPIC_INT_DISABLED;
    ioapic_write(route->ioapic_id, reg, value);
}

/**
 * Find IRQ routing entry
 */
irq_route_t* apic_find_irq_route(uint8_t irq) {
    if (irq >= 256 || !irq_routing[irq].enabled) {
        return NULL;
    }
    return &irq_routing[irq];
}

/**
 * Set up Local Vector Table entry
 */
void apic_setup_lvt_timer(uint32_t vector, uint32_t mode) {
    uint32_t lvt = vector | (mode << 17);
    apic_write(APIC_REG_TIMER, lvt);
}

void apic_setup_lvt_error(uint32_t vector) {
    apic_write(APIC_REG_ERROR, vector);
}

/**
 * Mask LVT entry
 */
void apic_mask_lvt_entry(uint32_t reg) {
    uint32_t value = apic_read(reg);
    value |= LVT_MASKED;
    apic_write(reg, value);
}

/**
 * Unmask LVT entry
 */
void apic_unmask_lvt_entry(uint32_t reg) {
    uint32_t value = apic_read(reg);
    value &= ~LVT_MASKED;
    apic_write(reg, value);
}

/**
 * Timer functions
 */
void apic_timer_init(uint32_t frequency) {
    /* Set timer divide register */
    apic_write(APIC_REG_TIMER_DCR, 0x03);  /* Divide by 16 */
    
    /* Set up timer LVT entry */
    apic_setup_lvt_timer(APIC_VECTOR_TIMER, APIC_TIMER_PERIODIC);
    
    local_apic.timer_frequency = frequency;
}

void apic_timer_start(uint32_t initial_count) {
    apic_write(APIC_REG_TIMER_ICR, initial_count);
}

void apic_timer_stop(void) {
    apic_write(APIC_REG_TIMER_ICR, 0);
}

/**
 * Vector allocation
 */
int apic_allocate_vector(void) {
    for (int i = APIC_VECTOR_BASE; i < 256; i++) {
        int word = i / 32;
        int bit = i % 32;
        
        if (!(vector_bitmap[word] & (1 << bit))) {
            vector_bitmap[word] |= (1 << bit);
            return i;
        }
    }
    return -1;  /* No free vector */
}

void apic_free_vector(uint32_t vector) {
    if (vector >= 256) return;
    
    int word = vector / 32;
    int bit = vector % 32;
    
    vector_bitmap[word] &= ~(1 << bit);
}

bool apic_vector_in_use(uint32_t vector) {
    if (vector >= 256) return false;
    
    int word = vector / 32;
    int bit = vector % 32;
    
    return (vector_bitmap[word] & (1 << bit)) != 0;
}

/**
 * CPU feature detection
 */
bool cpu_has_apic(void) {
    uint32_t eax, ebx, ecx, edx;
    __asm__ __volatile__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    return (edx & (1 << 9)) != 0;
}

bool cpu_has_x2apic(void) {
    uint32_t eax, ebx, ecx, edx;
    __asm__ __volatile__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    return (ecx & (1 << 21)) != 0;
}

/**
 * x2APIC support stubs
 */
bool apic_x2apic_supported(void) {
    return cpu_has_x2apic();
}

uint32_t apic_x2apic_read(uint32_t msr) {
    return (uint32_t)read_msr(msr);
}

void apic_x2apic_write(uint32_t msr, uint32_t value) {
    write_msr(msr, value);
}

void apic_x2apic_send_ipi(uint32_t dest_apic_id, uint32_t vector) {
    uint64_t icr = ((uint64_t)dest_apic_id << 32) | vector;
    write_msr(MSR_X2APIC_ICR, icr);
}

/**
 * Utility function
 */
void udelay(uint32_t microseconds) {
    /* Simple delay loop - should be calibrated */
    for (volatile uint32_t i = 0; i < microseconds * 1000; i++) {
        /* Busy wait */
    }
}