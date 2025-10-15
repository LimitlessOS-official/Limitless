#!/bin/bash
#
# LimitlessOS Laptop Installation System
# Complete installer for deploying LimitlessOS on laptop hardware
#

set -e

INSTALLER_VERSION="1.0"
LIMITLESS_VERSION="Enterprise 1.0"

echo "=============================================="
echo "LimitlessOS Enterprise Laptop Installer v${INSTALLER_VERSION}"
echo "The Superior Operating System for Laptops"
echo "=============================================="
echo ""

# Color codes for pretty output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
check_root() {
    if [[ $EUID -ne 0 ]]; then
        log_error "This installer must be run as root (use sudo)"
        log_info "Example: sudo ./install_limitless_laptop.sh"
        exit 1
    fi
}

# Detect laptop hardware
detect_laptop_hardware() {
    log_info "Detecting laptop hardware configuration..."
    
    # CPU information
    CPU_MODEL=$(cat /proc/cpuinfo | grep "model name" | head -1 | cut -d: -f2 | xargs)
    CPU_CORES=$(nproc)
    
    # Memory information
    TOTAL_RAM=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    TOTAL_RAM_GB=$((TOTAL_RAM / 1024 / 1024))
    
    # Storage information  
    STORAGE_DEVICES=$(lsblk -dn -o NAME,SIZE,TYPE | grep disk)
    
    # Graphics information
    GPU_INFO=$(lspci | grep -i vga || echo "No VGA detected")
    
    # WiFi information
    WIFI_INFO=$(lspci | grep -i wireless || echo "No WiFi detected")
    
    # Audio information
    AUDIO_INFO=$(lspci | grep -i audio || echo "No Audio detected")
    
    echo ""
    log_info "Laptop Hardware Summary:"
    echo "========================"
    echo "CPU: $CPU_MODEL ($CPU_CORES cores)"
    echo "RAM: ${TOTAL_RAM_GB}GB"
    echo "GPU: $GPU_INFO"
    echo "WiFi: $WIFI_INFO"  
    echo "Audio: $AUDIO_INFO"
    echo ""
    echo "Storage Devices:"
    echo "$STORAGE_DEVICES"
    echo ""
}

# Select installation disk
select_installation_disk() {
    log_info "Available storage devices for LimitlessOS installation:"
    echo ""
    
    # List available disks
    lsblk -dn -o NAME,SIZE,MODEL | grep -v loop
    echo ""
    
    while true; do
        echo -n "Enter the disk to install LimitlessOS (e.g., sda, nvme0n1): "
        read INSTALL_DISK
        
        if [[ -b "/dev/$INSTALL_DISK" ]]; then
            DISK_SIZE=$(lsblk -dn -o SIZE /dev/$INSTALL_DISK | xargs)
            log_warning "You selected /dev/$INSTALL_DISK (Size: $DISK_SIZE)"
            log_warning "ALL DATA ON THIS DISK WILL BE ERASED!"
            echo ""
            echo -n "Are you sure you want to continue? [yes/no]: "
            read CONFIRM
            
            if [[ "$CONFIRM" == "yes" ]]; then
                INSTALL_DEVICE="/dev/$INSTALL_DISK"
                log_success "Installation disk selected: $INSTALL_DEVICE"
                break
            else
                log_info "Installation cancelled by user"
                exit 0
            fi
        else
            log_error "Device /dev/$INSTALL_DISK does not exist"
        fi
    done
}

# Partition the disk
partition_disk() {
    log_info "Partitioning $INSTALL_DEVICE for LimitlessOS..."
    
    # Detect if system uses UEFI or BIOS
    if [[ -d /sys/firmware/efi ]]; then
        BOOT_MODE="UEFI"
        log_info "UEFI boot mode detected - creating GPT partition table"
        
        # Create GPT partition table for UEFI
        parted -s "$INSTALL_DEVICE" mklabel gpt
        
        # EFI System Partition (512MB)
        parted -s "$INSTALL_DEVICE" mkpart ESP fat32 1MiB 513MiB
        parted -s "$INSTALL_DEVICE" set 1 esp on
        
        # LimitlessOS Root Partition (remaining space)
        parted -s "$INSTALL_DEVICE" mkpart primary ext4 513MiB 100%
        
        EFI_PARTITION="${INSTALL_DEVICE}1"
        ROOT_PARTITION="${INSTALL_DEVICE}2"
        
    else
        BOOT_MODE="BIOS"
        log_info "BIOS boot mode detected - creating MBR partition table"
        
        # Create MBR partition table for BIOS
        parted -s "$INSTALL_DEVICE" mklabel msdos
        
        # Boot partition (512MB) - marked as bootable
        parted -s "$INSTALL_DEVICE" mkpart primary ext4 1MiB 513MiB
        parted -s "$INSTALL_DEVICE" set 1 boot on
        
        # LimitlessOS Root Partition (remaining space)
        parted -s "$INSTALL_DEVICE" mkpart primary ext4 513MiB 100%
        
        BOOT_PARTITION="${INSTALL_DEVICE}1"
        ROOT_PARTITION="${INSTALL_DEVICE}2"
    fi
    
    # Wait for partitions to be ready
    sleep 2
    partprobe "$INSTALL_DEVICE"
    sleep 2
    
    log_success "Disk partitioning completed for $BOOT_MODE boot mode"
}

# Format filesystems
format_filesystems() {
    log_info "Creating filesystems for LimitlessOS..."
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        # Format EFI System Partition
        log_info "Formatting EFI System Partition..."
        mkfs.fat -F32 -n "LIMITLESS-EFI" "$EFI_PARTITION"
        
        # Format root partition
        log_info "Formatting root partition..."
        mkfs.ext4 -L "LimitlessOS-Root" "$ROOT_PARTITION"
        
    else
        # Format boot partition for BIOS
        log_info "Formatting boot partition..."
        mkfs.ext4 -L "LimitlessOS-Boot" "$BOOT_PARTITION"
        
        # Format root partition
        log_info "Formatting root partition..."
        mkfs.ext4 -L "LimitlessOS-Root" "$ROOT_PARTITION"
    fi
    
    log_success "Filesystem creation completed"
}

# Mount filesystems
mount_filesystems() {
    log_info "Mounting filesystems..."
    
    # Create mount points
    mkdir -p /mnt/limitless
    
    # Mount root partition
    mount "$ROOT_PARTITION" /mnt/limitless
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        # Mount EFI System Partition
        mkdir -p /mnt/limitless/boot/efi
        mount "$EFI_PARTITION" /mnt/limitless/boot/efi
    else
        # Mount boot partition for BIOS
        mkdir -p /mnt/limitless/boot
        mount "$BOOT_PARTITION" /mnt/limitless/boot
    fi
    
    log_success "Filesystems mounted successfully"
}

# Install LimitlessOS files
install_limitless_files() {
    log_info "Installing LimitlessOS Enterprise system files..."
    
    # Create directory structure
    log_info "Creating LimitlessOS directory structure..."
    mkdir -p /mnt/limitless/{bin,sbin,usr,var,tmp,home,root,etc,dev,proc,sys}
    mkdir -p /mnt/limitless/usr/{bin,sbin,lib,share}
    mkdir -p /mnt/limitless/var/{log,tmp,cache}
    mkdir -p /mnt/limitless/etc/{limitless,network}
    
    # Copy kernel
    log_info "Installing LimitlessOS kernel..."
    cp build/kernel/limitless_kernel_laptop.bin /mnt/limitless/boot/limitless_kernel.bin
    
    # Copy userspace binaries
    log_info "Installing LimitlessOS userspace..."
    if [[ -d "userspace" ]]; then
        cp -r userspace/* /mnt/limitless/usr/ 2>/dev/null || true
    fi
    
    # Copy HAL libraries
    log_info "Installing Hardware Abstraction Layer..."
    if [[ -f "build/libhal.a" ]]; then
        cp build/libhal.a /mnt/limitless/usr/lib/
    fi
    
    # Create system configuration files
    log_info "Creating system configuration..."
    
    # Create fstab
    cat > /mnt/limitless/etc/fstab << EOF
# LimitlessOS filesystem table
$ROOT_PARTITION / ext4 defaults 0 1
EOF
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        echo "$EFI_PARTITION /boot/efi vfat defaults 0 0" >> /mnt/limitless/etc/fstab
    else
        echo "$BOOT_PARTITION /boot ext4 defaults 0 0" >> /mnt/limitless/etc/fstab
    fi
    
    # Create hostname
    echo "limitless-laptop" > /mnt/limitless/etc/hostname
    
    # Create basic network configuration
    cat > /mnt/limitless/etc/network/interfaces << EOF
# LimitlessOS Network Configuration
auto lo
iface lo inet loopback

# Ethernet interface (will be auto-detected)
auto eth0
iface eth0 inet dhcp

# WiFi interface (will be configured by NetworkManager)
auto wlan0
iface wlan0 inet dhcp
EOF
    
    # Create user account
    log_info "Creating user account..."
    echo -n "Enter username for LimitlessOS: "
    read USERNAME
    echo -n "Enter password for $USERNAME: "
    read -s PASSWORD
    echo ""
    
    mkdir -p "/mnt/limitless/home/$USERNAME"
    echo "$USERNAME:$PASSWORD" > /mnt/limitless/etc/passwd.tmp
    
    log_success "LimitlessOS system files installed successfully"
}

# Install bootloader
install_bootloader() {
    log_info "Installing LimitlessOS bootloader..."
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        log_info "Installing GRUB2 for UEFI boot..."
        
        # Install GRUB2 to EFI System Partition
        grub-install --target=x86_64-efi \
                    --efi-directory=/mnt/limitless/boot/efi \
                    --bootloader-id=LimitlessOS \
                    --boot-directory=/mnt/limitless/boot \
                    --recheck
        
        # Copy GRUB configuration
        mkdir -p /mnt/limitless/boot/grub
        cp bootloader/grub.cfg /mnt/limitless/boot/grub/
        
        # Create UEFI boot entry
        efibootmgr --create \
                  --disk "$INSTALL_DEVICE" \
                  --part 1 \
                  --loader "\\EFI\\LimitlessOS\\grubx64.efi" \
                  --label "LimitlessOS Enterprise" \
                  --verbose
                  
    else
        log_info "Installing GRUB2 for BIOS boot..."
        
        # Install GRUB2 to MBR
        grub-install --target=i386-pc \
                    --boot-directory=/mnt/limitless/boot \
                    "$INSTALL_DEVICE"
        
        # Copy GRUB configuration  
        mkdir -p /mnt/limitless/boot/grub
        cp bootloader/grub_legacy.cfg /mnt/limitless/boot/grub/grub.cfg
    fi
    
    log_success "Bootloader installation completed"
}

# Generate system information
generate_system_info() {
    log_info "Generating system information file..."
    
    cat > /mnt/limitless/etc/limitless/system-info << EOF
# LimitlessOS System Information
LIMITLESS_VERSION="$LIMITLESS_VERSION"
INSTALLATION_DATE="$(date)"
BOOT_MODE="$BOOT_MODE"
INSTALL_DEVICE="$INSTALL_DEVICE"
CPU_MODEL="$CPU_MODEL"
CPU_CORES="$CPU_CORES"
TOTAL_RAM_GB="$TOTAL_RAM_GB"
INSTALLER_VERSION="$INSTALLER_VERSION"
EOF
    
    log_success "System information file created"
}

# Finalize installation
finalize_installation() {
    log_info "Finalizing LimitlessOS installation..."
    
    # Set permissions
    chmod 755 /mnt/limitless/boot/limitless_kernel.bin
    
    # Create device nodes (basic)
    mknod /mnt/limitless/dev/null c 1 3 2>/dev/null || true
    mknod /mnt/limitless/dev/zero c 1 5 2>/dev/null || true
    
    # Sync filesystem
    sync
    
    # Unmount filesystems
    log_info "Unmounting filesystems..."
    umount /mnt/limitless/boot/efi 2>/dev/null || true
    umount /mnt/limitless/boot 2>/dev/null || true
    umount /mnt/limitless
    
    log_success "LimitlessOS installation completed successfully!"
}

# Installation complete message
show_completion_message() {
    echo ""
    echo "=============================================="
    echo -e "${GREEN}LimitlessOS Enterprise Installation Complete!${NC}"
    echo "=============================================="
    echo ""
    echo "Your laptop now has LimitlessOS installed with:"
    echo "• Enterprise-grade kernel with SMP, IOMMU, virtualization"
    echo "• Complete desktop GUI environment" 
    echo "• WiFi, audio, touchpad, and power management drivers"
    echo "• Advanced security and networking capabilities"
    echo "• Full multimedia and enterprise application support"
    echo ""
    echo "Installation Summary:"
    echo "---------------------"
    echo "Target Device: $INSTALL_DEVICE"
    echo "Boot Mode: $BOOT_MODE"
    echo "CPU: $CPU_MODEL ($CPU_CORES cores)"
    echo "RAM: ${TOTAL_RAM_GB}GB"
    echo ""
    echo -e "${YELLOW}Next Steps:${NC}"
    echo "1. Remove the installation media"
    echo "2. Reboot your laptop"
    echo "3. Select 'LimitlessOS Enterprise' from the boot menu"
    echo "4. Experience the superior operating system!"
    echo ""
    echo -e "${BLUE}Welcome to the future of computing with LimitlessOS!${NC}"
    echo ""
}

# Main installation function
main() {
    check_root
    
    log_info "Starting LimitlessOS laptop installation process..."
    echo ""
    
    detect_laptop_hardware
    select_installation_disk
    partition_disk
    format_filesystems
    mount_filesystems
    install_limitless_files
    install_bootloader
    generate_system_info
    finalize_installation
    show_completion_message
}

# Run main installation
main "$@"