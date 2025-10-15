#include "kernel.h"
#include "pci.h"
#include "pci_cfg.h"

#define PCI_CONF_ADDRESS 0xCF8
#define PCI_CONF_DATA    0xCFC

static inline void outl(u16 port, u32 val) {
#if defined(__x86_64__)
    __asm__ volatile("outl %0,%1" :: "a"(val), "Nd"(port));
#else
# error "CF8/CFC only implemented for x86_64"
#endif
}
static inline u32 inl(u16 port) {
    u32 v;
#if defined(__x86_64__)
    __asm__ volatile("inl %1,%0" : "=a"(v) : "Nd"(port));
#else
# error "CF8/CFC only implemented for x86_64"
#endif
    return v;
}

static inline u32 pci_cfg_addr(const pci_device_t* d, u16 off) {
    return 0x80000000u
         | ((u32)d->bus << 16)
         | ((u32)d->slot << 11)
         | ((u32)d->func << 8)
         | (off & 0xFC);
}

u32 pci_cfg_read32(const pci_device_t* d, u16 off) {
    outl(PCI_CONF_ADDRESS, pci_cfg_addr(d, off));
    return inl(PCI_CONF_DATA);
}
u16 pci_cfg_read16(const pci_device_t* d, u16 off) {
    u32 v = pci_cfg_read32(d, off & ~3u);
    return (u16)((v >> ((off & 2u) * 8u)) & 0xFFFFu);
}
u8 pci_cfg_read8(const pci_device_t* d, u16 off) {
    u32 v = pci_cfg_read32(d, off & ~3u);
    return (u8)((v >> ((off & 3u) * 8u)) & 0xFFu);
}
void pci_cfg_read(const pci_device_t* d, u16 off, void* buf, u32 len) {
    u8* out = (u8*)buf;
    for (u32 i = 0; i < len; i++) out[i] = pci_cfg_read8(d, (u16)(off + i));
}

u8 pci_cap_first(const pci_device_t* d) {
    u16 status = pci_cfg_read16(d, 0x06);
    if (!(status & (1u << 4))) return 0;
    return pci_cfg_read8(d, 0x34);
}
u8 pci_cap_next(const pci_device_t* d, u8 off) {
    if (!off) return 0;
    return pci_cfg_read8(d, (u16)(off + 1));
}