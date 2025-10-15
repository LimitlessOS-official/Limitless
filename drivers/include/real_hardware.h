/**
 * Real Hardware Drivers Header for LimitlessOS
 * 
 * Definitions and structures for actual hardware device drivers
 * that enable LimitlessOS to run on real systems.
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#ifndef __REAL_HARDWARE_H__
#define __REAL_HARDWARE_H__

#include <stdint.h>
#include <stdbool.h>
#include "smp.h"

/* PCI Configuration Space Registers */
#define PCI_VENDOR_ID           0x00
#define PCI_DEVICE_ID           0x02
#define PCI_COMMAND             0x04
#define PCI_STATUS              0x06
#define PCI_CLASS_CODE          0x08
#define PCI_HEADER_TYPE         0x0E
#define PCI_BAR0                0x10
#define PCI_BAR1                0x14
#define PCI_BAR2                0x18
#define PCI_BAR3                0x1C
#define PCI_BAR4                0x20
#define PCI_BAR5                0x24
#define PCI_INTERRUPT_LINE      0x3C
#define PCI_INTERRUPT_PIN       0x3D

/* PCI Command Register Bits */
#define PCI_COMMAND_IO          0x0001
#define PCI_COMMAND_MEMORY      0x0002
#define PCI_COMMAND_MASTER      0x0004
#define PCI_COMMAND_SPECIAL     0x0008
#define PCI_COMMAND_INVALIDATE  0x0010
#define PCI_COMMAND_VGA_PALETTE 0x0020
#define PCI_COMMAND_PARITY      0x0040
#define PCI_COMMAND_WAIT        0x0080
#define PCI_COMMAND_SERR        0x0100
#define PCI_COMMAND_FAST_BACK   0x0200
#define PCI_COMMAND_INTX_DISABLE 0x0400

/* IRQ Flags */
#define IRQF_SHARED             0x0080

/* Physical and Virtual Address Types */
typedef uintptr_t physical_addr_t;
typedef uintptr_t virtual_addr_t;

/* PCI BAR Structure */
typedef struct pci_bar {
    physical_addr_t address;        /* Physical address */
    size_t size;                    /* BAR size */
    uint32_t flags;                 /* BAR flags */
    bool is_io;                     /* I/O space vs memory space */
    bool is_64bit;                  /* 64-bit BAR */
    bool is_prefetchable;           /* Prefetchable memory */
} pci_bar_t;

/* PCI Device Structure */
typedef struct pci_device {
    uint8_t bus;                    /* PCI bus number */
    uint8_t device;                 /* Device number */
    uint8_t function;               /* Function number */
    
    uint16_t vendor_id;             /* Vendor ID */
    uint16_t device_id;             /* Device ID */
    uint16_t subsystem_vendor_id;   /* Subsystem vendor ID */
    uint16_t subsystem_device_id;   /* Subsystem device ID */
    
    uint8_t class_code;             /* Class code */
    uint8_t subclass;               /* Subclass */
    uint8_t prog_if;                /* Programming interface */
    uint8_t revision;               /* Revision */
    
    uint8_t header_type;            /* Header type */
    uint8_t interrupt_line;         /* Interrupt line */
    uint8_t interrupt_pin;          /* Interrupt pin */
    
    pci_bar_t bars[6];              /* Base address registers */
    
    void *driver_data;              /* Driver-specific data */
    
    struct pci_device *next;        /* Next device */
} pci_device_t;

/* Intel e1000 Network Adapter */
typedef struct e1000_adapter {
    struct pci_device *pdev;        /* PCI device */
    
    void *hw_addr;                  /* Hardware registers */
    uint8_t mac_addr[6];            /* MAC address */
    
    /* Network device */
    struct {
        char *name;                 /* Interface name */
        uint32_t flags;             /* Interface flags */
        uint32_t mtu;               /* MTU size */
    } netdev;
    
    /* TX/RX rings */
    struct e1000_tx_desc *tx_ring;  /* TX descriptor ring */
    struct e1000_rx_desc *rx_ring;  /* RX descriptor ring */
    uint32_t tx_ring_size;          /* TX ring size */
    uint32_t rx_ring_size;          /* RX ring size */
    
    /* DMA addresses */
    physical_addr_t tx_ring_dma;    /* TX ring DMA address */
    physical_addr_t rx_ring_dma;    /* RX ring DMA address */
    
    spinlock_t tx_lock;             /* TX lock */
    spinlock_t rx_lock;             /* RX lock */
} e1000_adapter_t;

/* e1000 TX Descriptor */
struct e1000_tx_desc {
    uint64_t buffer_addr;           /* Buffer address */
    union {
        uint32_t data;
        struct {
            uint16_t length;        /* Data length */
            uint8_t cso;            /* Checksum offset */
            uint8_t cmd;            /* Command */
        } fields;
    } lower;
    union {
        uint32_t data;
        struct {
            uint8_t status;         /* Status */
            uint8_t css;            /* Checksum start */
            uint16_t special;       /* Special */
        } fields;
    } upper;
};

/* e1000 RX Descriptor */
struct e1000_rx_desc {
    uint64_t buffer_addr;           /* Buffer address */
    uint16_t length;                /* Length */
    uint16_t checksum;              /* Checksum */
    uint8_t status;                 /* Status */
    uint8_t errors;                 /* Errors */
    uint16_t special;               /* Special */
};

/* NVMe Controller */
typedef struct nvme_ctrl {
    struct pci_device *pdev;        /* PCI device */
    void *bar;                      /* Controller registers */
    
    char model[41];                 /* Model string */
    char serial[21];                /* Serial number */
    char firmware[9];               /* Firmware revision */
    
    uint32_t nn;                    /* Number of namespaces */
    uint32_t max_qid;               /* Maximum queue ID */
    
    /* Admin queue */
    struct nvme_queue *admin_q;     /* Admin queue */
    
    /* I/O queues */
    struct nvme_queue **io_queues;  /* I/O queues */
    uint32_t queue_count;           /* Queue count */
    
    spinlock_t ctrl_lock;           /* Controller lock */
} nvme_ctrl_t;

/* NVMe Registers */
#define NVME_REG_CAP            0x00    /* Controller Capabilities */
#define NVME_REG_VS             0x08    /* Version */
#define NVME_REG_INTMS          0x0C    /* Interrupt Mask Set */
#define NVME_REG_INTMC          0x10    /* Interrupt Mask Clear */
#define NVME_REG_CC             0x14    /* Controller Configuration */
#define NVME_REG_CSTS           0x1C    /* Controller Status */
#define NVME_REG_AQA            0x24    /* Admin Queue Attributes */
#define NVME_REG_ASQ            0x28    /* Admin Submission Queue Base */
#define NVME_REG_ACQ            0x30    /* Admin Completion Queue Base */

/* Intel GPU Structure */
typedef struct intel_gpu {
    struct pci_device *pdev;        /* PCI device */
    
    void *mmio;                     /* MMIO registers */
    void *gmadr;                    /* Graphics memory */
    
    /* Framebuffer */
    struct {
        void *base;                 /* Framebuffer base */
        uint32_t width;             /* Width in pixels */
        uint32_t height;            /* Height in pixels */
        uint32_t pitch;             /* Bytes per line */
        uint32_t bpp;               /* Bits per pixel */
        uint32_t size;              /* Total size */
    } fb;
    
    spinlock_t gpu_lock;            /* GPU lock */
} intel_gpu_t;

/* AHCI Host Controller */
typedef struct ahci_host {
    struct pci_device *pdev;        /* PCI device */
    void *mmio;                     /* MMIO base */
    
    uint32_t cap;                   /* Capabilities */
    uint32_t cap2;                  /* Extended capabilities */
    uint32_t n_ports;               /* Number of ports */
    
    struct ahci_port *ports[32];    /* Port structures */
    
    spinlock_t host_lock;           /* Host lock */
} ahci_host_t;

/* AHCI Registers */
#define HOST_CAP                0x00    /* Host Capabilities */
#define HOST_CTL                0x04    /* Global Host Control */
#define HOST_IRQ_STAT           0x08    /* Interrupt Status */
#define HOST_PORTS_IMPL         0x0C    /* Ports Implemented */
#define HOST_VERSION            0x10    /* AHCI Version */

/* xHCI Host Controller */
typedef struct xhci_hcd {
    struct pci_device *pdev;        /* PCI device */
    
    void *cap_regs;                 /* Capability registers */
    void *op_regs;                  /* Operational registers */
    void *run_regs;                 /* Runtime registers */
    void *db_regs;                  /* Doorbell registers */
    
    /* Command ring */
    struct xhci_ring *cmd_ring;     /* Command ring */
    
    /* Event ring */
    struct xhci_ring *event_ring;   /* Event ring */
    
    /* Device context */
    void *dcbaa;                    /* Device Context Base Address Array */
    
    uint32_t hcs_params1;           /* Structural parameters 1 */
    uint32_t hcs_params2;           /* Structural parameters 2 */
    uint32_t hcs_params3;           /* Structural parameters 3 */
    
    spinlock_t xhci_lock;           /* xHCI lock */
} xhci_hcd_t;

/* IRQ Handler Type */
typedef int (*irq_handler_t)(int irq, void *data);

/* Function Prototypes */

/* Core Hardware */
int real_hardware_init(void);
void show_detected_hardware(void);

/* PCI Subsystem */
int pci_init(void);
void pci_scan_devices(void);
struct pci_device *pci_find_device(uint16_t vendor_id, uint16_t device_id);
uint32_t pci_read_config_dword(struct pci_device *dev, uint8_t offset);
uint16_t pci_read_config_word(struct pci_device *dev, uint8_t offset);
uint8_t pci_read_config_byte(struct pci_device *dev, uint8_t offset);
void pci_write_config_dword(struct pci_device *dev, uint8_t offset, uint32_t val);
void pci_write_config_word(struct pci_device *dev, uint8_t offset, uint16_t val);
void pci_write_config_byte(struct pci_device *dev, uint8_t offset, uint8_t val);
int pci_enable_device(struct pci_device *dev);
int pci_set_master(struct pci_device *dev);
int pci_request_irq(struct pci_device *dev, irq_handler_t handler, void *data);
const char *pci_get_class_name(uint8_t class_code);

/* Device Probing */
int intel_e1000_probe(struct pci_device *dev);
int rtl8139_probe(struct pci_device *dev);
int rtl8169_probe(struct pci_device *dev);
int nvme_probe(struct pci_device *dev);
int ahci_probe(struct pci_device *dev);
int ehci_probe(struct pci_device *dev);
int xhci_probe(struct pci_device *dev);
int intel_gpu_probe(struct pci_device *dev);
int nvidia_gpu_probe(struct pci_device *dev);
int amd_gpu_probe(struct pci_device *dev);
int intel_hda_probe(struct pci_device *dev);
int realtek_audio_probe(struct pci_device *dev);

/* Hardware Initialization */
void init_storage_controllers(void);
void init_network_controllers(void);
void init_usb_controllers(void);
void init_graphics_controllers(void);
void init_audio_controllers(void);

/* Intel e1000 Functions */
int e1000_reset_hw(struct e1000_adapter *adapter);
int e1000_read_mac_addr(struct e1000_adapter *adapter);
int e1000_setup_rings(struct e1000_adapter *adapter);
void e1000_free_rings(struct e1000_adapter *adapter);
void e1000_configure(struct e1000_adapter *adapter);
int e1000_interrupt(int irq, void *data);

/* NVMe Functions */
int nvme_reset_ctrl(struct nvme_ctrl *ctrl);
int nvme_setup_admin_queue(struct nvme_ctrl *ctrl);
void nvme_free_admin_queue(struct nvme_ctrl *ctrl);
int nvme_enable_ctrl(struct nvme_ctrl *ctrl);
int nvme_disable_ctrl(struct nvme_ctrl *ctrl);
int nvme_identify_ctrl(struct nvme_ctrl *ctrl);
void nvme_setup_io_queues(struct nvme_ctrl *ctrl);
void nvme_scan_namespaces(struct nvme_ctrl *ctrl);

/* Intel GPU Functions */
int intel_gpu_init_display(struct intel_gpu *gpu);
int intel_gpu_setup_framebuffer(struct intel_gpu *gpu);

/* AHCI Functions */
void ahci_enable(struct ahci_host *host);
void ahci_init_port(struct ahci_host *host, int port);
int ahci_interrupt(int irq, void *data);

/* xHCI Functions */
int xhci_reset(struct xhci_hcd *xhci);
int xhci_mem_init(struct xhci_hcd *xhci);
void xhci_run(struct xhci_hcd *xhci);
int xhci_interrupt(int irq, void *data);

/* Memory Management */
void *ioremap(physical_addr_t phys_addr, size_t size);
void iounmap(volatile void *addr);
virtual_addr_t allocate_kernel_virtual_memory(size_t size);
void free_kernel_virtual_memory(virtual_addr_t addr, size_t size);
int map_physical_memory(virtual_addr_t virt, physical_addr_t phys, size_t size, uint32_t flags);
void unmap_kernel_memory(virtual_addr_t addr);

/* Hardware Register Access */
uint32_t readl(volatile void *addr);
void writel(uint32_t val, volatile void *addr);
uint16_t readw(volatile void *addr);
void writew(uint16_t val, volatile void *addr);
uint8_t readb(volatile void *addr);
void writeb(uint8_t val, volatile void *addr);

/* Interrupt Management */
int request_irq(int irq, irq_handler_t handler, uint32_t flags, 
               const char *name, void *data);
void free_irq(int irq, void *data);

/* Device Registration */
int register_netdev(struct net_device *dev);
int register_block_device(void *ctrl);
int register_framebuffer(void *fb);

/* Page Flags */
#define PAGE_PRESENT            0x001
#define PAGE_WRITABLE           0x002
#define PAGE_USER               0x004
#define PAGE_WRITETHROUGH       0x008
#define PAGE_NOCACHE            0x010
#define PAGE_ACCESSED           0x020
#define PAGE_DIRTY              0x040
#define PAGE_LARGE              0x080
#define PAGE_GLOBAL             0x100

/* PCI Class Codes */
#define PCI_CLASS_STORAGE       0x01
#define PCI_CLASS_NETWORK       0x02
#define PCI_CLASS_DISPLAY       0x03
#define PCI_CLASS_MULTIMEDIA    0x04
#define PCI_CLASS_MEMORY        0x05
#define PCI_CLASS_BRIDGE        0x06
#define PCI_CLASS_SERIAL        0x0C

/* Common Hardware Vendor IDs */
#define PCI_VENDOR_INTEL        0x8086
#define PCI_VENDOR_AMD          0x1022
#define PCI_VENDOR_NVIDIA       0x10DE
#define PCI_VENDOR_REALTEK      0x10EC
#define PCI_VENDOR_SAMSUNG      0x144D
#define PCI_VENDOR_SANDISK      0x15B7

#endif /* __REAL_HARDWARE_H__ */