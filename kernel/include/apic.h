/**
 * Advanced Programmable Interrupt Controller (APIC) Support for LimitlessOS
 * 
 * This module provides Local APIC and I/O APIC support for multi-processor
 * systems, including interrupt routing and inter-processor interrupts (IPIs).
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Local APIC Register Offsets */
#define APIC_REG_ID             0x020   /* Local APIC ID */
#define APIC_REG_VERSION        0x030   /* Local APIC Version */
#define APIC_REG_TPR            0x080   /* Task Priority Register */
#define APIC_REG_APR            0x090   /* Arbitration Priority Register */
#define APIC_REG_PPR            0x0A0   /* Processor Priority Register */
#define APIC_REG_EOI            0x0B0   /* End of Interrupt */
#define APIC_REG_RRD            0x0C0   /* Remote Read Register */
#define APIC_REG_LDR            0x0D0   /* Logical Destination Register */
#define APIC_REG_DFR            0x0E0   /* Destination Format Register */
#define APIC_REG_SIVR           0x0F0   /* Spurious Interrupt Vector Register */
#define APIC_REG_ISR            0x100   /* In-Service Register */
#define APIC_REG_TMR            0x180   /* Trigger Mode Register */
#define APIC_REG_IRR            0x200   /* Interrupt Request Register */
#define APIC_REG_ESR            0x280   /* Error Status Register */
#define APIC_REG_CMCI           0x2F0   /* Corrected Machine Check Interrupt */
#define APIC_REG_ICR_LOW        0x300   /* Interrupt Command Register (Low) */
#define APIC_REG_ICR_HIGH       0x310   /* Interrupt Command Register (High) */
#define APIC_REG_TIMER          0x320   /* Local Vector Table Timer */
#define APIC_REG_THERMAL        0x330   /* Thermal Sensor Register */
#define APIC_REG_PERF           0x340   /* Performance Counter Register */
#define APIC_REG_LINT0          0x350   /* Local Interrupt 0 */
#define APIC_REG_LINT1          0x360   /* Local Interrupt 1 */
#define APIC_REG_ERROR          0x370   /* Error Register */
#define APIC_REG_TIMER_ICR      0x380   /* Timer Initial Count Register */
#define APIC_REG_TIMER_CCR      0x390   /* Timer Current Count Register */
#define APIC_REG_TIMER_DCR      0x3E0   /* Timer Divide Configuration Register */

/* ICR Delivery Mode */
#define ICR_DELIVERY_FIXED      0x0
#define ICR_DELIVERY_LOWEST     0x1
#define ICR_DELIVERY_SMI        0x2
#define ICR_DELIVERY_NMI        0x4
#define ICR_DELIVERY_INIT       0x5
#define ICR_DELIVERY_STARTUP    0x6

/* ICR Destination Shorthand */
#define ICR_DEST_NO_SHORTHAND   0x0
#define ICR_DEST_SELF           0x1
#define ICR_DEST_ALL_INC_SELF   0x2
#define ICR_DEST_ALL_EXC_SELF   0x3

/* ICR Level */
#define ICR_LEVEL_DEASSERT      0x0
#define ICR_LEVEL_ASSERT        0x1

/* ICR Trigger Mode */
#define ICR_TRIGGER_EDGE        0x0
#define ICR_TRIGGER_LEVEL       0x1

/* Timer Modes */
#define APIC_TIMER_ONE_SHOT     0x0
#define APIC_TIMER_PERIODIC     0x1
#define APIC_TIMER_TSC_DEADLINE 0x2

/* Local Vector Table Flags */
#define LVT_MASKED              (1 << 16)
#define LVT_TRIGGER_LEVEL       (1 << 15)
#define LVT_REMOTE_IRR          (1 << 14)
#define LVT_PIN_POLARITY        (1 << 13)
#define LVT_DELIVERY_PENDING    (1 << 12)
#define LVT_DELIVERY_MODE_MASK  0x700

/* I/O APIC Registers */
#define IOAPIC_REG_ID           0x00
#define IOAPIC_REG_VERSION      0x01
#define IOAPIC_REG_ARB          0x02
#define IOAPIC_REDTBL_BASE      0x10

/* I/O APIC Redirection Table Entry Flags */
#define IOAPIC_INT_DISABLED     (1 << 16)
#define IOAPIC_INT_LEVEL        (1 << 15)
#define IOAPIC_INT_ACTIVELOW    (1 << 13)
#define IOAPIC_INT_LOGICAL      (1 << 11)

/* APIC MSR Addresses */
#define MSR_APIC_BASE           0x1B
#define MSR_X2APIC_APICID       0x802
#define MSR_X2APIC_VERSION      0x803
#define MSR_X2APIC_TPR          0x808
#define MSR_X2APIC_PPR          0x80A
#define MSR_X2APIC_EOI          0x80B
#define MSR_X2APIC_LDR          0x80D
#define MSR_X2APIC_SIVR         0x80F
#define MSR_X2APIC_ICR          0x830

/* APIC Base MSR Flags */
#define APIC_BASE_BSP           (1 << 8)   /* Bootstrap Processor */
#define APIC_BASE_X2APIC        (1 << 10)  /* x2APIC Mode */
#define APIC_BASE_GLOBAL_ENABLE (1 << 11)  /* Global Enable */

/* Maximum number of I/O APICs */
#define MAX_IOAPICS 8

/* I/O APIC structure */
typedef struct ioapic {
    uint32_t id;
    uint32_t version;
    uint32_t max_redirection_entries;
    uintptr_t base_address;
    uint32_t global_irq_base;
    bool enabled;
} ioapic_t;

/* Local APIC information */
typedef struct local_apic {
    uint32_t id;
    uint32_t version;
    uintptr_t base_address;
    bool x2apic_enabled;
    bool enabled;
    uint32_t spurious_vector;
    uint32_t timer_frequency;
} local_apic_t;

/* Interrupt routing entry */
typedef struct irq_route {
    uint8_t irq;            /* IRQ number */
    uint8_t ioapic_id;      /* I/O APIC handling this IRQ */
    uint8_t ioapic_pin;     /* Pin on the I/O APIC */
    uint8_t vector;         /* Interrupt vector */
    uint32_t flags;         /* Routing flags */
    bool enabled;
} irq_route_t;

/* Global APIC state */
extern local_apic_t local_apic;
extern ioapic_t ioapics[MAX_IOAPICS];
extern uint32_t num_ioapics;
extern irq_route_t irq_routing[256];

/* APIC Initialization */
int apic_init(void);
void apic_init_secondary(void);
int apic_detect_and_init(void);

/* Local APIC Functions */
uint32_t apic_get_id(void);
void apic_enable(void);
void apic_disable(void);
bool apic_is_enabled(void);
void apic_set_spurious_vector(uint32_t vector);
void apic_eoi(void);

/* Local APIC Register Access */
uint32_t apic_read(uint32_t reg);
void apic_write(uint32_t reg, uint32_t value);

/* Timer Functions */
void apic_timer_init(uint32_t frequency);
void apic_timer_set_mode(uint32_t mode);
void apic_timer_start(uint32_t initial_count);
void apic_timer_stop(void);
uint32_t apic_timer_get_count(void);
void apic_calibrate_timer(void);
static inline void apic_timer_set_periodic(uint32_t ticks) { apic_timer_start(ticks); }

/* IPI (Inter-Processor Interrupt) Functions */
void apic_send_ipi(uint32_t dest_apic_id, uint32_t vector);
void apic_send_ipi_all(uint32_t vector);
void apic_send_ipi_all_but_self(uint32_t vector);
void apic_send_init_ipi(uint32_t dest_apic_id);
void apic_send_startup_ipi(uint32_t dest_apic_id, uint32_t vector);
void apic_send_nmi_ipi(uint32_t dest_apic_id);

/* I/O APIC Functions */
int ioapic_init(void);
uint32_t ioapic_read(uint32_t ioapic_id, uint32_t reg);
void ioapic_write(uint32_t ioapic_id, uint32_t reg, uint32_t value);
void ioapic_set_irq(uint32_t irq, uint32_t vector, uint32_t dest_apic_id, uint32_t flags);
void ioapic_mask_irq(uint32_t irq);
void ioapic_unmask_irq(uint32_t irq);
bool ioapic_is_irq_masked(uint32_t irq);

/* IRQ Routing */
int apic_setup_irq_routing(void);
void apic_route_irq(uint8_t irq, uint8_t vector, uint32_t dest_apic_id);
void apic_route_legacy_irqs(void);
irq_route_t* apic_find_irq_route(uint8_t irq);

/* x2APIC Support */
bool apic_x2apic_supported(void);
void apic_enable_x2apic(void);
void apic_disable_x2apic(void);
bool apic_x2apic_enabled(void);
uint32_t apic_x2apic_read(uint32_t msr);
void apic_x2apic_write(uint32_t msr, uint32_t value);
void apic_x2apic_send_ipi(uint32_t dest_apic_id, uint32_t vector);

/* APIC Error Handling */
uint32_t apic_get_error_status(void);
void apic_clear_errors(void);
void apic_register_error_handler(void (*handler)(uint32_t error));

/* Interrupt Vector Management */
#define APIC_VECTOR_BASE        0x20    /* Base vector for APIC interrupts */
#define APIC_VECTOR_SPURIOUS    0xFF    /* Spurious interrupt vector */
#define APIC_VECTOR_TIMER       0xEF    /* APIC timer vector */
#define APIC_VECTOR_ERROR       0xEE    /* APIC error vector */
#define APIC_VECTOR_THERMAL     0xED    /* Thermal sensor vector */
#define APIC_VECTOR_PERFCTR     0xEC    /* Performance counter vector */
#define APIC_VECTOR_CMCI        0xEB    /* Corrected Machine Check vector */
#define APIC_VECTOR_IPI_BASE    0xF0    /* Base for IPI vectors */

int apic_allocate_vector(void);
void apic_free_vector(uint32_t vector);
bool apic_vector_in_use(uint32_t vector);

/* MSR helpers */
static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    __asm__ __volatile__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ __volatile__("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

/* Memory-mapped I/O helpers */
static inline uint32_t mmio_read32(uintptr_t addr) {
    return *(volatile uint32_t*)addr;
}

static inline void mmio_write32(uintptr_t addr, uint32_t value) {
    *(volatile uint32_t*)addr = value;
}

/* CPU feature detection - will be implemented in .c file */
bool cpu_has_apic(void);
bool cpu_has_x2apic(void);

/* Utility functions */
void udelay(uint32_t microseconds);
