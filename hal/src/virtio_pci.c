#include "kernel.h"
#include "virtio_pci.h"

int virtio_find_caps(const pci_device_t* dev,
                     virtio_pci_common_cfg_t** out_common,
                     void** out_notify, u32* out_notify_mul,
                     void** out_devcfg) {
    *out_common = NULL; *out_notify = NULL; *out_notify_mul = 0; *out_devcfg = NULL;

    u8 off = pci_cap_first(dev);
    while (off) {
        u8 vndr = pci_cfg_read8(dev, off + 0);
        if (vndr == 0x09) {
            virtio_pci_cap_t cap;
            pci_cfg_read(dev, off, &cap, sizeof(cap));
            if (cap.cfg_type == VIRTIO_PCI_CAP_COMMON_CFG) {
                *out_common = (virtio_pci_common_cfg_t*)virtio_ioremap(dev, cap.bar, cap.offset, cap.length);
            } else if (cap.cfg_type == VIRTIO_PCI_CAP_DEVICE_CFG) {
                *out_devcfg = virtio_ioremap(dev, cap.bar, cap.offset, cap.length);
            } else if (cap.cfg_type == VIRTIO_PCI_CAP_NOTIFY_CFG) {
                virtio_pci_notify_cap_t ncap;
                pci_cfg_read(dev, off, &ncap, sizeof(ncap));
                *out_notify = virtio_ioremap(dev, ncap.cap.bar, ncap.cap.offset, ncap.cap.length);
                *out_notify_mul = ncap.notify_off_multiplier;
            }
        }
        off = pci_cap_next(dev, off);
    }
    return (*out_common && *out_notify && *out_devcfg) ? 0 : K_ENOENT;
}