/*
 * XHCI (eXtensible Host Controller Interface) Driver
 * USB 3.0/3.1/3.2 Controller
 * 
 * Supports SuperSpeed (5 Gbps), SuperSpeed+ (10 Gbps, 20 Gbps)
 */

#include "hal.h"
#include "pci_cfg.h"

/* XHCI Register Offsets */
#define XHCI_USBCMD     0x00  /* USB Command */
#define XHCI_USBSTS     0x04  /* USB Status */
#define XHCI_PAGESIZE   0x08  /* Page Size */
#define XHCI_DNCTRL     0x14  /* Device Notification Control */
#define XHCI_CRCR       0x18  /* Command Ring Control */
#define XHCI_DCBAAP     0x30  /* Device Context Base Address Array Pointer */
#define XHCI_CONFIG     0x38  /* Configure */
#define XHCI_PORTSC     0x400 /* Port Status and Control (base) */
#define XHCI_ERDP       0x438 /* Event Ring Dequeue Pointer */

/* XHCI Command Register Bits */
#define XHCI_USBCMD_RUN     (1 << 0)   /* Run/Stop */
#define XHCI_USBCMD_HCRST   (1 << 1)   /* Host Controller Reset */

/* XHCI Status Register Bits */
#define XHCI_USBSTS_CNR     (1 << 11)  /* Controller Not Ready */

/* XHCI Port Status and Control Bits */
#define XHCI_PORTSC_PR      (1 << 4)   /* Port Reset */
#define XHCI_PORTSC_CSC     (1 << 17)  /* Connect Status Change */
#define XHCI_PORTSC_PEC     (1 << 18)  /* Port Enabled/Disabled Change */
#define XHCI_PORTSC_WRC     (1 << 19)  /* Warm Port Reset Change */
#define XHCI_PORTSC_OCC     (1 << 20)  /* Over-current Change */
#define XHCI_PORTSC_PRC     (1 << 21)  /* Port Reset Change */

/* XHCI Command Ring Control Bits */
#define XHCI_CRCR_RCS       (1 << 0)   /* Ring Cycle State */

/* XHCI Config Register Bits */
#define XHCI_CONFIG_MaxSlotsEn_MASK     0xFF
#define XHCI_CONFIG_MaxSlotsEn_SHIFT    0

/* XHCI Helper Functions */
static uint32_t xhci_read32(xhci_controller_t* ctrl, uint32_t offset) {
    return *(volatile uint32_t*)((uint8_t*)ctrl->mmio_base + offset);
}

static void xhci_write32(xhci_controller_t* ctrl, uint32_t offset, uint32_t value) {
    *(volatile uint32_t*)((uint8_t*)ctrl->mmio_base + offset) = value;
}

static uint64_t xhci_read64(xhci_controller_t* ctrl, uint32_t offset) {
    return *(volatile uint64_t*)((uint8_t*)ctrl->mmio_base + offset);
}

static void xhci_write64(xhci_controller_t* ctrl, uint32_t offset, uint64_t value) {
    *(volatile uint64_t*)((uint8_t*)ctrl->mmio_base + offset) = value;
}

/* XHCI Ring Allocation */
static void* xhci_alloc_ring(uint32_t trb_count) {
    /* Allocate aligned memory for TRB ring */
    size_t size = trb_count * 16; /* 16 bytes per TRB */
    return hal_memory_alloc_aligned(size, 64); /* 64-byte alignment */
}

/* XHCI Device Context Base Address Array */
static void* xhci_alloc_dcbaa(void) {
    /* Allocate device context base address array (256 entries * 8 bytes) */
    return hal_memory_alloc_aligned(256 * 8, 64);
}

/* XHCI Registers (Memory-Mapped) */
typedef struct xhci_capability_regs {
    uint8_t caplength;      // Capability register length
    uint8_t reserved;
    uint16_t hciversion;    // Interface version
    uint32_t hcsparams1;    // Structural parameters 1
    uint32_t hcsparams2;    // Structural parameters 2
    uint32_t hcsparams3;    // Structural parameters 3
    uint32_t hccparams1;    // Capability parameters 1
    uint32_t dboff;         // Doorbell offset
    uint32_t rtsoff;        // Runtime registers offset
    uint32_t hccparams2;    // Capability parameters 2
} __attribute__((packed)) xhci_capability_regs_t;

typedef struct xhci_operational_regs {
    uint32_t usbcmd;        // USB command
    uint32_t usbsts;        // USB status
    uint32_t pagesize;      // Page size
    uint32_t reserved[2];
    uint32_t dnctrl;        // Device notification control
    uint64_t crcr;          // Command ring control
    uint32_t reserved2[4];
    uint64_t dcbaap;        // Device context base address array pointer
    uint32_t config;        // Configure
} __attribute__((packed)) xhci_operational_regs_t;

/* XHCI Controller State */
typedef struct xhci_controller {
    uint32_t pci_device_id;
    void* mmio_base;                    // MMIO base address
    xhci_capability_regs_t* cap_regs;   // Capability registers
    xhci_operational_regs_t* op_regs;   // Operational registers
    uint32_t* doorbell_array;           // Doorbell array
    
    uint32_t max_slots;                 // Max device slots
    uint32_t max_ports;                 // Max root hub ports
    
    bool initialized;
    uint32_t lock;
} xhci_controller_t;

/* Global XHCI state */
static struct {
    xhci_controller_t controllers[4];  // Support up to 4 XHCI controllers
    uint32_t controller_count;
    bool initialized;
} xhci_state = {0};

/* Initialize XHCI controller */
status_t xhci_init(uint32_t pci_bus, uint32_t pci_device, uint32_t pci_function) {
    if (xhci_state.controller_count >= 4) {
        return STATUS_NOMEM;
    }
    
    xhci_controller_t* ctrl = &xhci_state.controllers[xhci_state.controller_count];
    
    /* XHCI Controller Initialization */
    
    /* 1. Map MMIO registers via PCI BAR */
    uint32_t bar0 = pci_cfg_read32(pci_bus, pci_device, pci_function, 0x10);
    ctrl->mmio_base = (void*)(uintptr_t)(bar0 & ~0xF);
    
    /* 2. Reset controller (USBCMD.HCRST) */
    uint32_t usbcmd = xhci_read32(ctrl, XHCI_USBCMD);
    usbcmd |= XHCI_USBCMD_HCRST;
    xhci_write32(ctrl, XHCI_USBCMD, usbcmd);
    
    /* 3. Wait for reset completion (USBSTS.CNR) */
    uint32_t timeout = 1000;
    while (timeout-- && (xhci_read32(ctrl, XHCI_USBSTS) & XHCI_USBSTS_CNR)) {
        hal_timer_delay_us(1000); /* 1ms delay */
    }
    
    /* 4. Set up command ring */
    ctrl->cmd_ring = xhci_alloc_ring(64); /* 64 TRBs */
    xhci_write64(ctrl, XHCI_CRCR, (uint64_t)ctrl->cmd_ring | XHCI_CRCR_RCS);
    
    /* 5. Set up event ring */
    ctrl->event_ring = xhci_alloc_ring(256); /* 256 TRBs */
    xhci_write64(ctrl, XHCI_ERDP, (uint64_t)ctrl->event_ring);
    
    /* 6. Set up device context base address array */
    ctrl->dcbaa = xhci_alloc_dcbaa();
    xhci_write64(ctrl, XHCI_DCBAAP, (uint64_t)ctrl->dcbaa);
    
    /* 7. Configure max slots */
    uint32_t config = xhci_read32(ctrl, XHCI_CONFIG);
    config = (config & ~XHCI_CONFIG_MaxSlotsEn_MASK) | (64 << XHCI_CONFIG_MaxSlotsEn_SHIFT);
    xhci_write32(ctrl, XHCI_CONFIG, config);
    
    /* 8. Run controller (USBCMD.Run) */
    usbcmd = xhci_read32(ctrl, XHCI_USBCMD);
    usbcmd |= XHCI_USBCMD_RUN;
    xhci_write32(ctrl, XHCI_USBCMD, usbcmd);
    
    ctrl->initialized = true;
    xhci_state.controller_count++;
    
    hal_log(HAL_LOG_INFO, "XHCI", "Controller initialized at PCI %02x:%02x.%x",
            pci_bus, pci_device, pci_function);
    
    return STATUS_OK;
}

/* XHCI port reset */
status_t xhci_reset_port(xhci_controller_t* ctrl, uint8_t port) {
    if (!ctrl || !ctrl->initialized) {
        return STATUS_INVALID;
    }
    
    /* XHCI port reset sequence */
    
    /* 1. Write PORTSC.PR (Port Reset) */
    uint32_t portsc_offset = XHCI_PORTSC + (port * 0x10);
    uint32_t portsc = xhci_read32(ctrl, portsc_offset);
    portsc |= XHCI_PORTSC_PR;
    xhci_write32(ctrl, portsc_offset, portsc);
    
    /* 2. Wait for reset completion */
    uint32_t timeout = 100;
    while (timeout-- && (xhci_read32(ctrl, portsc_offset) & XHCI_PORTSC_PR)) {
        hal_timer_delay_us(1000); /* 1ms delay */
    }
    
    /* 3. Clear status change bits */
    portsc = xhci_read32(ctrl, portsc_offset);
    portsc |= XHCI_PORTSC_CSC | XHCI_PORTSC_PEC | XHCI_PORTSC_WRC | XHCI_PORTSC_OCC | XHCI_PORTSC_PRC;
    xhci_write32(ctrl, portsc_offset, portsc);
    
    hal_log(HAL_LOG_INFO, "XHCI", "Port %d reset", port);
    
    return STATUS_OK;
}

/* XHCI command submission */
status_t xhci_submit_command(xhci_controller_t* ctrl, void* trb) {
    if (!ctrl || !ctrl->initialized || !trb) {
        return STATUS_INVALID;
    }
    
    /* XHCI Command submission implementation */
    if (!trb) return STATUS_INVALID;
    
    /* 1. Write TRB to command ring */
    xhci_trb_t* cmd_trb = (xhci_trb_t*)ctrl->cmd_ring;
    memcpy(&cmd_trb[ctrl->cmd_ring_enqueue], trb, sizeof(xhci_trb_t));
    ctrl->cmd_ring_enqueue = (ctrl->cmd_ring_enqueue + 1) % XHCI_CMD_RING_SIZE;
    
    /* 2. Ring doorbell (DB[0]) */
    xhci_write32(ctrl, XHCI_DB_OFFSET, 0);
    
    /* 3. Wait for command completion event */
    uint32_t timeout = 1000;
    while (timeout-- && !xhci_check_event_completion(ctrl)) {
        hal_timer_delay_us(1000);
    }
    
    return timeout > 0 ? STATUS_OK : STATUS_TIMEOUT;
    
    return STATUS_OK;
}

/* XHCI transfer submission */
status_t xhci_submit_transfer(xhci_controller_t* ctrl, uint8_t slot_id,
                              uint8_t endpoint, void* buffer, uint32_t length) {
    if (!ctrl || !ctrl->initialized) {
        return STATUS_INVALID;
    }
    
    /* XHCI Transfer submission implementation */
    if (!ctrl || !transfer || !ctrl->initialized) {
        return STATUS_INVALID;
    }
    
    /* Set up Transfer Request Block (TRB) */
    xhci_trb_t trb = {0};
    trb.parameter = (uint64_t)transfer->buffer;
    trb.status = transfer->length | (1 << 22); /* Interrupt on completion */
    trb.control = transfer->type | (1 << 0);   /* Cycle bit */
    
    /* Add to appropriate transfer ring */
    return xhci_submit_command(ctrl, &trb);
    /* 1. Set up Transfer TRB */
    /* 2. Write to transfer ring */
    /* 3. Ring doorbell (DB[slot_id]) */
    
    return STATUS_OK;
}

/* XHCI interrupt handler */
void xhci_interrupt_handler(xhci_controller_t* ctrl) {
    if (!ctrl || !ctrl->initialized) {
        return;
    }
    
    /* XHCI Interrupt handling implementation */
    if (!ctrl || !ctrl->initialized) {
        return STATUS_INVALID;
    }
    
    /* Read interrupt status */
    uint32_t usbsts = xhci_read32(ctrl, XHCI_USBSTS);
    if (!(usbsts & XHCI_USBSTS_EINT)) {
        return STATUS_OK; /* No interrupt pending */
    }
    
    /* Process event ring */
    xhci_process_event_ring(ctrl);
    
    /* Clear interrupt status */
    xhci_write32(ctrl, XHCI_USBSTS, XHCI_USBSTS_EINT);
    
    return STATUS_OK;
    /* - Check USBSTS register */
    /* - Process event ring */
    /* - Handle completion events */
    /* - Handle port change events */
    
    hal_log(HAL_LOG_DEBUG, "XHCI", "Interrupt handled");
}

/* Get XHCI controller by index */
xhci_controller_t* xhci_get_controller(uint32_t index) {
    if (index >= xhci_state.controller_count) {
        return NULL;
    }
    return &xhci_state.controllers[index];
}

/* XHCI subsystem initialization */
status_t xhci_subsystem_init(void) {
    if (xhci_state.initialized) {
        return STATUS_EXISTS;
    }
    
    xhci_state.controller_count = 0;
    xhci_state.initialized = true;
    
    /* Enumerate XHCI controllers via PCI */
    hal_log(HAL_LOG_INFO, "XHCI", "Enumerating XHCI controllers...");
    
    uint32_t controller_count = 0;
    for (uint32_t bus = 0; bus < 256 && controller_count < 4; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            for (uint32_t func = 0; func < 8; func++) {
                uint32_t class_info = pci_cfg_read32(bus, slot, func, 0x08);
                uint8_t class_code = (class_info >> 24) & 0xFF;
                uint8_t subclass = (class_info >> 16) & 0xFF;
                uint8_t prog_if = (class_info >> 8) & 0xFF;
                
                if (class_code == 0x0C && subclass == 0x03 && prog_if == 0x30) {
                    /* Found XHCI controller */
                    if (xhci_init(bus, slot, func) == STATUS_OK) {
                        controller_count++;
                    }
                }
            }
        }
    }
    
    hal_log(HAL_LOG_INFO, "XHCI", "Initialized %u XHCI controller(s)", controller_count);
    /* - Scan PCI for class 0x0C (Serial Bus) subclass 0x03 (USB) prog-if 0x30 (XHCI) */
    /* - Initialize each controller found */
    
    hal_log(HAL_LOG_INFO, "XHCI", "XHCI subsystem initialized");
    
    return STATUS_OK;
}
