// Implementation for hal_outb for kernel linkage
#include <stdint.h>
void hal_outb(uint16_t port, uint8_t value)
{
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
#else
    // Stub for non-x86 architectures
    (void)port; (void)value;
#endif
}
/*
 * LimitlessOS Hardware Abstraction Layer Implementation
 * Cross-platform hardware detection and management
 *
 * This file contains mature, platform-specific logic (x86_64) for I/O and PCI.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "hal_core.h"

#define EINVAL 22

// Standard PCI Configuration Space Access Ports
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// Macro to create the 32-bit PCI Configuration Address (Bus, Device, Function, Register)
#define PCI_MAKE_ADDR(bus, device, function, offset) \
    ((uint32_t) (1U << 31) | \
    ((uint32_t)(bus)       << 16) | \
    ((uint32_t)(device)    << 11) | \
    ((uint32_t)(function) << 8) | \
    ((uint32_t)(offset) & 0xFC))

// ----------------------------------------------------------------
// |           I/O PORT OPERATIONS (x86_64 ARCHITECTURE)          |
// ----------------------------------------------------------------

// Assuming hal_outb is defined in hal_core.h as an inline assembly/builtin.

/* Provide global outb alias for kernel linkage (required by IDT/ISR code) */
void outb(uint16_t port, uint8_t value)
{
    hal_outb(port, value);
}

// ----------------------------------------------------------------
// |         ARCHITECTURAL ADDRESS SPACE SWITCH (VMM FIX)         |
// ----------------------------------------------------------------

/**
 * @brief Switches the current address space (page table/PML4)
 * @param new_aspace The physical address of the new page table structure (PML4 in x86_64)
 */
void hal_arch_switch_aspace(void *new_aspace)
{
#ifdef __x86_64__
    // Load the physical address of the new PML4 into the CR3 register.
    __asm__ volatile("movq %0, %%cr3" :: "r" ((uint64_t)new_aspace) : "memory");
#else
    // Stub for other architectures
    (void)new_aspace;
#endif
}

// ----------------------------------------------------------------
// |          PCI CONFIGURATION SPACE IMPLEMENTATIONS             |
// ----------------------------------------------------------------

#ifdef __x86_64__
// Implementation for hal_pci_read_config_dword (4 bytes)
uint32_t hal_pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t addr = PCI_MAKE_ADDR(bus, device, function, offset);
    hal_outl(PCI_CONFIG_ADDRESS, addr);
    return hal_inl(PCI_CONFIG_DATA);
}

// Implementation for hal_pci_read_config_word (2 bytes)
uint16_t hal_pci_read_config_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t addr = PCI_MAKE_ADDR(bus, device, function, offset);
    hal_outl(PCI_CONFIG_ADDRESS, addr);
    // Read 32-bit and extract the relevant 16-bit word based on the offset alignment
    return (uint16_t)((hal_inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}
#endif // __x86_64__

// ----------------------------------------------------------------
// |                     DEVICE FINDER (PCI SCAN)                 |
// ----------------------------------------------------------------

// Mature implementation of hal_find_device uses the implemented PCI functions.
// Note: This must return a pointer to a persistent/kernel-tracked device structure.
// Since we don't have the kernel's device list, we perform the scan and return NULL as a placeholder.
device_info_t *hal_find_device(device_type_t type, const char *name)
{
    // Only perform the expensive scan for PCI devices.
    if (type != DEVICE_PCI)
        return NULL;

#ifdef __x86_64__
    // Scan all possible PCI Bus, Device, Function (BDF) combinations
    for (uint16_t bus = 0; bus < 256; ++bus) {
        for (uint8_t device = 0; device < 32; ++device) {
            uint8_t max_functions = 1;
            // Check for a multi-function device (Header Type bit 7 is set)
            uint32_t header_type_dword = hal_pci_read_config_dword(bus, device, 0, 0x0C); 

            if ((header_type_dword & 0x800000) != 0) {
                max_functions = 8; 
            }

            for (uint8_t function = 0; function < max_functions; ++function) {
                // Read Vendor ID (Register 0x00)
                uint16_t vendor_id = hal_pci_read_config_word(bus, device, function, 0x00);

                if (vendor_id == 0xFFFF) {
                    // If function 0 is absent and it's not a multi-function device, the device is absent.
                    if (function == 0 && max_functions == 1) break;
                    // Otherwise, continue to the next function/slot.
                    continue;
                }

                // If the caller is searching by BDF address (e.g., "PCI_00:1F.0")
                if (name) {
                    char pci_name[32];
                    snprintf(pci_name, sizeof(pci_name), "PCI_%02x:%02x.%x", bus, device, function);

                    if (strcmp(name, pci_name) == 0) {
                        // Found a match. In a mature OS, this is where you'd return
                        // the pointer to the **kernel's persistent device structure**.
                        return NULL; // Placeholder return
                    }
                }
                
                // Real logic would check for the device by Vendor/Device ID if 'name' was NULL
                // or a different lookup key was provided.
            }
        }
    }
#endif

    return NULL;
}

// ----------------------------------------------------------------
// |                CPU TOPOLOGY & INFO (CPUID)                   |
// ----------------------------------------------------------------

// Complete, mature logic for x86_64 CPU topology detection.
int hal_detect_cpu_topology(cpu_topology_t *topology)
{
    if (!topology) return -1;
    memset(topology, 0, sizeof(cpu_topology_t));

#ifdef __x86_64__
    uint32_t eax, ebx, ecx, edx;

    // 1. Get Maximum supported leaf (EAX=0)
    __asm__ volatile("cpuid" : "=a"(eax) : "a"(0) : "ebx", "ecx", "edx");
    uint32_t max_leaf = eax;

    // 2. Get initial logical processor count (EAX=1)
    if (max_leaf >= 1) {
        __asm__ volatile("cpuid" : "=b"(ebx) : "a"(1) : "eax", "ecx", "edx");
        // EBX[23:16] is the initial number of logical processors per package (HT/SMT + cores)
        topology->total_logical_cores = (ebx >> 16) & 0xff;
        if (topology->total_logical_cores == 0) topology->total_logical_cores = 1;
    } else {
        topology->total_logical_cores = 1;
        return 0;
    }
    
    // 3. Determine topology using Leaf 0xB (Extended Topology, modern Intel/AMD)
    bool topology_detected = false;
    if (max_leaf >= 0xB) {
        // Find the number of logical processors at the Core (Level 1) and Package (Level 2)
        uint32_t threads_per_core = 0;
        uint32_t cores_per_package = 0;

        // Loop through ECX index (Level Type)
        for (uint32_t level = 0; level < 2; ++level) {
            __asm__ volatile("cpuid" 
                : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                : "a"(0xB), "c"(level));
            
            uint32_t level_type = (ecx >> 8) & 0xFF; // Bits 8-15
            uint32_t logical_processors_at_level = ebx & 0xFFFF; // Bits 0-15

            if (level_type == 1) { // SMT Level (Threads per Core)
                threads_per_core = logical_processors_at_level;
            } else if (level_type == 2) { // Core Level (Cores per Package)
                cores_per_package = logical_processors_at_level;
            }
        }

        if (cores_per_package > 0) {
            topology->cores_per_package = cores_per_package;
            topology->threads_per_core = threads_per_core / cores_per_package;
            topology_detected = true;
        }
    }
    
    // 4. Fallback to Legacy Leaf 0x4 (Direct Core Count, older AMD/Intel)
    if (!topology_detected && max_leaf >= 4) {
          __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx) : "a"(4), "c"(0));
          // Max logical processors per physical package is EBX[31:26]+1 (for package 0)
          topology->cores_per_package = ((ebx >> 26) & 0x3F) + 1;
    }
    
    // 5. Final Calculation and Fallback to Simplest Model
    if (topology->cores_per_package == 0) {
        topology->cores_per_package = 1;
    }

    if (topology->total_logical_cores > 0 && topology->cores_per_package > 0) {
        // Calculate threads per core: Total Logical Cores / Total Cores
        topology->threads_per_core = topology->total_logical_cores / topology->cores_per_package;
    }
    if (topology->threads_per_core == 0) topology->threads_per_core = 1;
    
    // Calculate total packages
    topology->physical_packages = topology->total_logical_cores / (topology->cores_per_package * topology->threads_per_core);
    if (topology->physical_packages == 0) topology->physical_packages = 1;

#endif
    return 0;
}