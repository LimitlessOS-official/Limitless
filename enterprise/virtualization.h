#ifndef LIMITLESS_VIRTUALIZATION_H
#define LIMITLESS_VIRTUALIZATION_H

int virtualization_kvm_start(const char* vm_name);
int virtualization_qemu_start(const char* vm_name);
int virtualization_virtualbox_start(const char* vm_name);
int virtualization_vmware_start(const char* vm_name);

#endif // LIMITLESS_VIRTUALIZATION_H
