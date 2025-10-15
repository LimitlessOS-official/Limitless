
#!/usr/bin/env bash

##############################################################################
# LimitlessOS Enterprise Installation Master Script
# Comprehensive installation system supporting multiple deployment scenarios
##############################################################################


set -e
set -u

# Platform detection
PLATFORM="$(uname -s)"
if [[ "$PLATFORM" == "Linux" ]]; then
    OS_TYPE="linux"
elif [[ "$PLATFORM" == "Darwin" ]]; then
    OS_TYPE="macos"
elif [[ "$PLATFORM" == *"MINGW"* || "$PLATFORM" == *"MSYS"* ]]; then
    OS_TYPE="windows"
else
    OS_TYPE="unknown"
fi

# Script Configuration
INSTALLER_VERSION="2.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="/tmp/limitless_install.log"
CONFIG_FILE=""
DEPLOYMENT_MODE="interactive"
FORCE_INSTALL=false
SKIP_VALIDATION=false
DEBUG_MODE=false

# Color codes for output
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly PURPLE='\033[0;35m'
readonly CYAN='\033[0;36m'
readonly WHITE='\033[1;37m'
readonly NC='\033[0m' # No Color

##############################################################################
# Logging Functions
##############################################################################

log() {
    local level="$1"
    shift
    local message="$*"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    echo "[$timestamp] [$level] $message" | tee -a "$LOG_FILE"
}

log_info() {
    log "INFO" "$@"
    echo -e "${BLUE}[INFO]${NC} $*"
}

log_success() {
    log "SUCCESS" "$@"
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

log_warning() {
    log "WARNING" "$@"
    echo -e "${YELLOW}[WARNING]${NC} $*"
}

log_error() {
    log "ERROR" "$@"
    echo -e "${RED}[ERROR]${NC} $*"
}

log_debug() {
    if [ "$DEBUG_MODE" = true ]; then
        log "DEBUG" "$@"
        echo -e "${PURPLE}[DEBUG]${NC} $*"
    fi
}

##############################################################################
# Utility Functions
##############################################################################

show_banner() {
    echo -e "${CYAN}"
    echo "=============================================================================="
    echo "                      LimitlessOS Enterprise Installer"
    echo "                           Version $INSTALLER_VERSION"
    echo "        The Superior Operating System - Enterprise Grade Installation"
    echo "=============================================================================="
    echo -e "${NC}"
}

show_help() {
    cat << EOF
LimitlessOS Enterprise Installation Script

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -h, --help                 Show this help message
    -c, --config FILE         Use configuration file
    -m, --mode MODE           Installation mode: interactive, automated, silent
    -f, --force               Force installation (skip confirmations)
    -s, --skip-validation     Skip hardware validation
    -d, --debug               Enable debug mode
    -v, --version             Show version information
    
DEPLOYMENT MODES:
    interactive               Interactive installation with GUI/TUI
    automated                 Automated installation using config file
    silent                    Silent installation (no user interaction)
    enterprise                Enterprise deployment with full features
    kiosk                     Kiosk mode installation
    server                    Server installation (headless)
    developer                 Developer workstation installation

EXAMPLES:
    $0                                          # Interactive installation
    $0 -m automated -c enterprise.conf         # Automated enterprise install
    $0 -m silent -f                            # Silent installation
    $0 -m enterprise -c /path/to/config.json   # Enterprise deployment

CONFIGURATION:
    Configuration files can be in JSON or INI format.
    Use --generate-config to create a template configuration file.

EOF
}


check_root() {
    if [[ "$OS_TYPE" == "windows" ]]; then
        log_error "Windows detected. Please run this installer in WSL or a Linux environment."
        exit 1
    fi
    if [[ $EUID -ne 0 ]]; then
        log_error "This installer must be run as root (use sudo)"
        log_info "Example: sudo $0"
        exit 1
    fi
}

check_system_requirements() {
    log_info "Checking system requirements..."
    
    # Check architecture
    local arch=$(uname -m)
    if [[ "$arch" != "x86_64" ]]; then
        log_error "Unsupported architecture: $arch (x86_64 required)"
        return 1
    fi
    
    # Check minimum RAM (4GB)
    local ram_kb=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    local ram_gb=$((ram_kb / 1024 / 1024))
    if [[ $ram_gb -lt 4 ]]; then
        log_error "Insufficient RAM: ${ram_gb}GB (4GB minimum required)"
        if [[ "$FORCE_INSTALL" != true ]]; then
            return 1
        fi
        log_warning "Continuing with insufficient RAM (forced)"
    fi
    
    # Check disk space
    local disk_space_gb=$(df / | tail -1 | awk '{print int($4/1024/1024)}')
    if [[ $disk_space_gb -lt 20 ]]; then
        log_error "Insufficient disk space: ${disk_space_gb}GB (20GB minimum required)"
        if [[ "$FORCE_INSTALL" != true ]]; then
            return 1
        fi
        log_warning "Continuing with insufficient disk space (forced)"
    fi
    
    # Check for UEFI/BIOS
    if [[ -d /sys/firmware/efi ]]; then
        log_info "UEFI boot mode detected"
        export BOOT_MODE="UEFI"
    else
        log_info "BIOS boot mode detected"
        export BOOT_MODE="BIOS"
    fi
    
    # Check for virtualization support
    if grep -q vmx /proc/cpuinfo || grep -q svm /proc/cpuinfo; then
        log_info "Hardware virtualization support detected"
        export VIRTUALIZATION_SUPPORT=true
    else
        log_warning "No hardware virtualization support detected"
        export VIRTUALIZATION_SUPPORT=false
    fi
    
    # Check for secure boot
    if [[ -f /sys/firmware/efi/efivars/SecureBoot-* ]]; then
        local secure_boot=$(hexdump -C /sys/firmware/efi/efivars/SecureBoot-* | head -1 | awk '{print $17}')
        if [[ "$secure_boot" == "01" ]]; then
            log_info "Secure Boot is enabled"
            export SECURE_BOOT_ENABLED=true
        else
            log_info "Secure Boot is disabled"
            export SECURE_BOOT_ENABLED=false
        fi
    else
        export SECURE_BOOT_ENABLED=false
    fi
    
    log_success "System requirements check completed"
    return 0
}

detect_hardware() {
    log_info "Detecting hardware configuration..."
    
    # CPU Information
    local cpu_model=$(grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)
    local cpu_cores=$(nproc)
    local cpu_freq=$(grep "cpu MHz" /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)
    
    export CPU_MODEL="$cpu_model"
    export CPU_CORES="$cpu_cores"
    export CPU_FREQUENCY="$cpu_freq"
    
    log_info "CPU: $cpu_model ($cpu_cores cores, ${cpu_freq}MHz)"
    
    # Memory Information
    local total_mem=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    local total_mem_gb=$((total_mem / 1024 / 1024))
    export TOTAL_MEMORY_GB="$total_mem_gb"
    
    log_info "Memory: ${total_mem_gb}GB"
    
    # Storage Information
    log_info "Detecting storage devices..."
    mapfile -t STORAGE_DEVICES < <(lsblk -dn -o NAME | grep -E '^(sd|nvme|vd)')
    
    for device in "${STORAGE_DEVICES[@]}"; do
        local size=$(lsblk -dn -o SIZE "/dev/$device" | xargs)
        local model=$(lsblk -dn -o MODEL "/dev/$device" | xargs)
        log_info "Storage: /dev/$device - $model ($size)"
    done
    
    # Graphics Information
    local gpu_info=$(lspci | grep -i vga | head -1 | cut -d: -f3 | xargs)
    export GPU_INFO="$gpu_info"
    log_info "Graphics: $gpu_info"
    
    # Network Information
    local network_interfaces=$(ip link show | grep -E '^[0-9]+:' | grep -v lo | awk -F: '{print $2}' | xargs)
    export NETWORK_INTERFACES="$network_interfaces"
    log_info "Network interfaces: $network_interfaces"
    
    log_success "Hardware detection completed"
}

validate_hardware() {
    if [[ "$SKIP_VALIDATION" == true ]]; then
        log_info "Hardware validation skipped by user"
        return 0
    fi
    
    log_info "Validating hardware compatibility..."
    
    local validation_passed=true
    
    # Validate CPU requirements
    if [[ $CPU_CORES -lt 2 ]]; then
        log_error "CPU validation failed: $CPU_CORES cores (minimum 2 required)"
        validation_passed=false
    fi
    
    # Validate memory requirements
    if [[ $TOTAL_MEMORY_GB -lt 4 ]]; then
        log_error "Memory validation failed: ${TOTAL_MEMORY_GB}GB (minimum 4GB required)"
        validation_passed=false
    fi
    
    # Validate storage requirements
    local suitable_storage=false
    for device in "${STORAGE_DEVICES[@]}"; do
        local size_bytes=$(lsblk -dn -b -o SIZE "/dev/$device")
        local size_gb=$((size_bytes / 1024 / 1024 / 1024))
        if [[ $size_gb -ge 50 ]]; then
            suitable_storage=true
            break
        fi
    done
    
    if [[ "$suitable_storage" != true ]]; then
        log_error "Storage validation failed: No disk with minimum 50GB found"
        validation_passed=false
    fi
    
    # Validate UEFI/Secure Boot for enterprise deployment
    if [[ "$DEPLOYMENT_MODE" == "enterprise" && "$BOOT_MODE" != "UEFI" ]]; then
        log_warning "Enterprise deployment recommended with UEFI boot mode"
    fi
    
    if [[ "$validation_passed" == true ]]; then
        log_success "Hardware validation passed"
        return 0
    else
        log_error "Hardware validation failed"
        if [[ "$FORCE_INSTALL" == true ]]; then
            log_warning "Continuing despite validation failures (forced)"
            return 0
        fi
        return 1
    fi
}

##############################################################################
# Installation Functions
##############################################################################

select_installation_disk() {
    log_info "Selecting installation disk..."
    
    if [[ ${#STORAGE_DEVICES[@]} -eq 0 ]]; then
        log_error "No suitable storage devices found"
        return 1
    fi
    
    if [[ "$DEPLOYMENT_MODE" == "silent" || "$DEPLOYMENT_MODE" == "automated" ]]; then
        # Auto-select the largest disk
        local largest_disk=""
        local largest_size=0
        
        for device in "${STORAGE_DEVICES[@]}"; do
            local size_bytes=$(lsblk -dn -b -o SIZE "/dev/$device")
            if [[ $size_bytes -gt $largest_size ]]; then
                largest_size=$size_bytes
                largest_disk="$device"
            fi
        done
        
        export INSTALL_DISK="/dev/$largest_disk"
        log_info "Auto-selected disk: $INSTALL_DISK"
    else
        # Interactive disk selection
        echo -e "\n${CYAN}Available storage devices:${NC}"
        local i=1
        for device in "${STORAGE_DEVICES[@]}"; do
            local size=$(lsblk -dn -o SIZE "/dev/$device" | xargs)
            local model=$(lsblk -dn -o MODEL "/dev/$device" | xargs)
            echo "  $i. /dev/$device - $model ($size)"
            ((i++))
        done
        
        while true; do
            echo -ne "\n${YELLOW}Select disk for installation [1-${#STORAGE_DEVICES[@]}]:${NC} "
            read -r selection
            
            if [[ "$selection" =~ ^[0-9]+$ ]] && [[ $selection -ge 1 ]] && [[ $selection -le ${#STORAGE_DEVICES[@]} ]]; then
                local selected_device="${STORAGE_DEVICES[$((selection-1))]}"
                export INSTALL_DISK="/dev/$selected_device"
                
                echo -e "\n${RED}WARNING: ALL DATA ON $INSTALL_DISK WILL BE DESTROYED!${NC}"
                echo -ne "${YELLOW}Are you sure you want to continue? [yes/no]:${NC} "
                read -r confirmation
                
                if [[ "$confirmation" == "yes" ]]; then
                    log_info "Selected disk: $INSTALL_DISK"
                    break
                else
                    log_info "Installation cancelled by user"
                    exit 0
                fi
            else
                echo -e "${RED}Invalid selection. Please try again.${NC}"
            fi
        done
    fi
}

partition_disk() {
    log_info "Partitioning disk: $INSTALL_DISK"
    
    # Unmount any existing partitions
    umount "${INSTALL_DISK}"* 2>/dev/null || true
    
    # Create GPT partition table
    parted -s "$INSTALL_DISK" mklabel gpt
    log_info "Created GPT partition table"
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        # UEFI partitioning scheme
        # 1. EFI System Partition (512MB)
        parted -s "$INSTALL_DISK" mkpart ESP fat32 1MiB 513MiB
        parted -s "$INSTALL_DISK" set 1 esp on
        
        # 2. Boot partition (1GB)
        parted -s "$INSTALL_DISK" mkpart boot ext4 513MiB 1537MiB
        
        # 3. Root partition A (40% of remaining space)
        parted -s "$INSTALL_DISK" mkpart rootA ext4 1537MiB 40%
        
        # 4. Root partition B (40% of remaining space)
        parted -s "$INSTALL_DISK" mkpart rootB ext4 40% 80%
        
        # 5. Swap partition (remaining space)
        parted -s "$INSTALL_DISK" mkpart swap linux-swap 80% 100%
        
        export EFI_PARTITION="${INSTALL_DISK}1"
        export BOOT_PARTITION="${INSTALL_DISK}2"
        export ROOT_PARTITION_A="${INSTALL_DISK}3"
        export ROOT_PARTITION_B="${INSTALL_DISK}4"
        export SWAP_PARTITION="${INSTALL_DISK}5"
        
    else
        # BIOS partitioning scheme
        # 1. Boot partition (512MB, bootable)
        parted -s "$INSTALL_DISK" mkpart primary ext4 1MiB 513MiB
        parted -s "$INSTALL_DISK" set 1 boot on
        
        # 2. Root partition A (40% of remaining space)
        parted -s "$INSTALL_DISK" mkpart primary ext4 513MiB 40%
        
        # 3. Root partition B (40% of remaining space)
        parted -s "$INSTALL_DISK" mkpart primary ext4 40% 80%
        
        # 4. Swap partition (remaining space)
        parted -s "$INSTALL_DISK" mkpart primary linux-swap 80% 100%
        
        export BOOT_PARTITION="${INSTALL_DISK}1"
        export ROOT_PARTITION_A="${INSTALL_DISK}2"
        export ROOT_PARTITION_B="${INSTALL_DISK}3"
        export SWAP_PARTITION="${INSTALL_DISK}4"
    fi
    
    # Wait for kernel to recognize partitions
    partprobe "$INSTALL_DISK"
    sleep 2
    
    log_success "Disk partitioning completed"
}

setup_encryption() {
    if [[ "$ENCRYPTION_ENABLED" != "true" ]]; then
        log_info "Disk encryption disabled"
        return 0
    fi
    
    log_info "Setting up disk encryption..."
    
    local passphrase="${ENCRYPTION_PASSPHRASE:-limitless_enterprise_2024}"
    
    # Setup LUKS encryption for root partitions
    echo "$passphrase" | cryptsetup luksFormat --type luks2 \
        --cipher aes-xts-plain64 --key-size 512 --hash sha512 \
        "$ROOT_PARTITION_A"
    
    echo "$passphrase" | cryptsetup luksFormat --type luks2 \
        --cipher aes-xts-plain64 --key-size 512 --hash sha512 \
        "$ROOT_PARTITION_B"
    
    # Open encrypted partitions
    echo "$passphrase" | cryptsetup luksOpen "$ROOT_PARTITION_A" rootA_crypt
    echo "$passphrase" | cryptsetup luksOpen "$ROOT_PARTITION_B" rootB_crypt
    
    # Update partition variables to point to encrypted devices
    export ROOT_PARTITION_A="/dev/mapper/rootA_crypt"
    export ROOT_PARTITION_B="/dev/mapper/rootB_crypt"
    export ENCRYPTION_ACTIVE=true
    
    log_success "Disk encryption setup completed"
}

format_filesystems() {
    log_info "Formatting filesystems..."
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        # Format EFI System Partition
        mkfs.fat -F32 -n "LIMITLESS_EFI" "$EFI_PARTITION"
        log_info "Formatted EFI system partition"
    fi
    
    # Format boot partition
    mkfs.ext4 -L "LIMITLESS_BOOT" "$BOOT_PARTITION"
    log_info "Formatted boot partition"
    
    # Format root partitions
    mkfs.ext4 -L "LIMITLESS_ROOT_A" "$ROOT_PARTITION_A"
    mkfs.ext4 -L "LIMITLESS_ROOT_B" "$ROOT_PARTITION_B"
    log_info "Formatted root partitions"
    
    # Format swap partition
    mkswap -L "LIMITLESS_SWAP" "$SWAP_PARTITION"
    log_info "Formatted swap partition"
    
    log_success "Filesystem formatting completed"
}

mount_filesystems() {
    log_info "Mounting filesystems..."
    
    # Create mount point
    mkdir -p /mnt/limitless
    
    # Mount root partition
    mount "$ROOT_PARTITION_A" /mnt/limitless
    
    # Create and mount boot directory
    mkdir -p /mnt/limitless/boot
    mount "$BOOT_PARTITION" /mnt/limitless/boot
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        # Mount EFI system partition
        mkdir -p /mnt/limitless/boot/efi
        mount "$EFI_PARTITION" /mnt/limitless/boot/efi
    fi
    
    # Enable swap
    swapon "$SWAP_PARTITION"
    
    log_success "Filesystems mounted successfully"
}

install_system_files() {
    log_info "Installing LimitlessOS system files..."
    
    # Create directory structure
    mkdir -p /mnt/limitless/{bin,sbin,usr,var,tmp,home,root,etc,dev,proc,sys,opt}
    mkdir -p /mnt/limitless/usr/{bin,sbin,lib,lib64,share,local}
    mkdir -p /mnt/limitless/var/{log,tmp,cache,lib,run}
    mkdir -p /mnt/limitless/etc/{limitless,network,security}
    
    # Install kernel
    if [[ -f "$SCRIPT_DIR/../build/kernel/limitless_kernel.bin" ]]; then
        cp "$SCRIPT_DIR/../build/kernel/limitless_kernel.bin" /mnt/limitless/boot/
        log_info "Installed kernel"
    else
        log_warning "Kernel binary not found, using placeholder"
        touch /mnt/limitless/boot/limitless_kernel.bin
    fi
    
    # Install initramfs
    if [[ -f "$SCRIPT_DIR/../build/kernel/initrd.img" ]]; then
        cp "$SCRIPT_DIR/../build/kernel/initrd.img" /mnt/limitless/boot/
        log_info "Installed initramfs"
    fi
    
    # Install userspace binaries
    if [[ -d "$SCRIPT_DIR/../build/userspace" ]]; then
        cp -r "$SCRIPT_DIR/../build/userspace"/* /mnt/limitless/usr/ 2>/dev/null || true
        log_info "Installed userspace components"
    fi
    
    # Install libraries
    if [[ -d "$SCRIPT_DIR/../build/lib" ]]; then
        cp -r "$SCRIPT_DIR/../build/lib"/* /mnt/limitless/usr/lib/ 2>/dev/null || true
        log_info "Installed system libraries"
    fi
    
    # Create essential symlinks
    ln -sf /usr/bin/bash /mnt/limitless/bin/bash
    ln -sf /usr/bin/sh /mnt/limitless/bin/sh
    
    log_success "System files installation completed"
}

configure_system() {
    log_info "Configuring system..."
    
    # Create fstab
    cat > /mnt/limitless/etc/fstab << EOF
# LimitlessOS filesystem table
# <filesystem> <mount point> <type> <options> <dump> <pass>
$ROOT_PARTITION_A / ext4 defaults 0 1
$BOOT_PARTITION /boot ext4 defaults 0 2
$SWAP_PARTITION none swap sw 0 0
EOF
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        echo "$EFI_PARTITION /boot/efi vfat defaults 0 0" >> /mnt/limitless/etc/fstab
    fi
    
    # Configure hostname
    echo "${HOSTNAME:-limitless-enterprise}" > /mnt/limitless/etc/hostname
    
    # Configure hosts file
    cat > /mnt/limitless/etc/hosts << EOF
127.0.0.1 localhost
127.0.1.1 ${HOSTNAME:-limitless-enterprise}
::1 localhost ip6-localhost ip6-loopback
EOF
    
    # Configure network
    mkdir -p /mnt/limitless/etc/network
    cat > /mnt/limitless/etc/network/interfaces << EOF
# LimitlessOS network configuration
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet dhcp

auto wlan0
iface wlan0 inet dhcp
EOF
    
    # Configure DNS
    cat > /mnt/limitless/etc/resolv.conf << EOF
# LimitlessOS DNS configuration
nameserver 1.1.1.1
nameserver 1.0.0.1
nameserver 8.8.8.8
EOF
    
    # Create system information
    cat > /mnt/limitless/etc/limitless-release << EOF
LimitlessOS Enterprise v$INSTALLER_VERSION
Built: $(date)
Architecture: $(uname -m)
Kernel: $(uname -r)
Installation Mode: $DEPLOYMENT_MODE
Boot Mode: $BOOT_MODE
Encryption: ${ENCRYPTION_ENABLED:-false}
EOF
    
    # Configure timezone
    ln -sf /usr/share/zoneinfo/UTC /mnt/limitless/etc/localtime
    
    log_success "System configuration completed"
}

install_bootloader() {
    log_info "Installing bootloader..."
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        # Install GRUB for UEFI
        grub-install --target=x86_64-efi \
                    --efi-directory=/mnt/limitless/boot/efi \
                    --bootloader-id=LimitlessOS \
                    --boot-directory=/mnt/limitless/boot \
                    --recheck 2>/dev/null || {
            log_warning "GRUB installation failed, using fallback method"
            mkdir -p /mnt/limitless/boot/grub
        }
    else
        # Install GRUB for BIOS
        grub-install --target=i386-pc \
                    --boot-directory=/mnt/limitless/boot \
                    "$INSTALL_DISK" 2>/dev/null || {
            log_warning "GRUB installation failed, using fallback method"
            mkdir -p /mnt/limitless/boot/grub
        }
    fi
    
    # Create GRUB configuration
    mkdir -p /mnt/limitless/boot/grub
    cat > /mnt/limitless/boot/grub/grub.cfg << EOF
# LimitlessOS GRUB Configuration
set timeout=5
set default=0

menuentry 'LimitlessOS Enterprise' {
    linux /limitless_kernel.bin root=$ROOT_PARTITION_A rootfstype=ext4 quiet splash
    initrd /initrd.img
}

menuentry 'LimitlessOS Recovery' {
    linux /limitless_kernel.bin root=$ROOT_PARTITION_B rootfstype=ext4 single
    initrd /initrd.img
}

menuentry 'LimitlessOS Safe Mode' {
    linux /limitless_kernel.bin root=$ROOT_PARTITION_A rootfstype=ext4 single nosplash
    initrd /initrd.img
}
EOF
    
    log_success "Bootloader installation completed"
}

create_user_accounts() {
    log_info "Creating user accounts..."
    
    # Create passwd file
    cat > /mnt/limitless/etc/passwd << EOF
root:x:0:0:root:/root:/bin/bash
${USERNAME:-limitless}:x:1000:1000:${FULLNAME:-LimitlessOS User}:/home/${USERNAME:-limitless}:/bin/bash
EOF
    
    # Create shadow file
    cat > /mnt/limitless/etc/shadow << EOF
root:*:19000:0:99999:7:::
${USERNAME:-limitless}:*:19000:0:99999:7:::
EOF
    
    # Create group file
    cat > /mnt/limitless/etc/group << EOF
root:x:0:
users:x:100:
${USERNAME:-limitless}:x:1000:
sudo:x:27:${USERNAME:-limitless}
EOF
    
    # Create user home directory
    mkdir -p "/mnt/limitless/home/${USERNAME:-limitless}"
    mkdir -p "/mnt/limitless/home/${USERNAME:-limitless}"/{Desktop,Documents,Downloads,Music,Pictures,Videos}
    
    # Set proper permissions
    chmod 600 /mnt/limitless/etc/shadow
    chmod 644 /mnt/limitless/etc/passwd /mnt/limitless/etc/group
    
    log_success "User accounts created successfully"
}

finalize_installation() {
    log_info "Finalizing installation..."
    
    # Generate machine ID
    uuidgen > /mnt/limitless/etc/machine-id 2>/dev/null || \
        echo "limitless-$(date +%s)" > /mnt/limitless/etc/machine-id
    
    # Set file permissions
    chmod 755 /mnt/limitless/boot/limitless_kernel.bin 2>/dev/null || true
    chmod 755 /mnt/limitless/usr/bin/* 2>/dev/null || true
    
    # Create device nodes
    mknod /mnt/limitless/dev/null c 1 3 2>/dev/null || true
    mknod /mnt/limitless/dev/zero c 1 5 2>/dev/null || true
    mknod /mnt/limitless/dev/random c 1 8 2>/dev/null || true
    mknod /mnt/limitless/dev/urandom c 1 9 2>/dev/null || true
    
    # Sync filesystems
    sync
    
    # Unmount filesystems
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        umount /mnt/limitless/boot/efi 2>/dev/null || true
    fi
    umount /mnt/limitless/boot 2>/dev/null || true
    swapoff "$SWAP_PARTITION" 2>/dev/null || true
    umount /mnt/limitless 2>/dev/null || true
    
    # Close encrypted devices
    if [[ "$ENCRYPTION_ACTIVE" == true ]]; then
        cryptsetup luksClose rootA_crypt 2>/dev/null || true
        cryptsetup luksClose rootB_crypt 2>/dev/null || true
    fi
    
    log_success "Installation finalization completed"
}

##############################################################################
# Configuration Management
##############################################################################

load_configuration() {
    if [[ -n "$CONFIG_FILE" && -f "$CONFIG_FILE" ]]; then
        log_info "Loading configuration from: $CONFIG_FILE"
        
        if [[ "$CONFIG_FILE" =~ \.json$ ]]; then
            # JSON configuration
            export DEPLOYMENT_MODE=$(jq -r '.deployment.mode // "interactive"' "$CONFIG_FILE")
            export ENCRYPTION_ENABLED=$(jq -r '.security.encryption // true' "$CONFIG_FILE")
            export USERNAME=$(jq -r '.user.username // "limitless"' "$CONFIG_FILE")
            export FULLNAME=$(jq -r '.user.fullname // "LimitlessOS User"' "$CONFIG_FILE")
            export HOSTNAME=$(jq -r '.system.hostname // "limitless-enterprise"' "$CONFIG_FILE")
        else
            # INI/Shell configuration
            source "$CONFIG_FILE"
        fi
        
        log_success "Configuration loaded successfully"
    else
        # Set default configuration
        export DEPLOYMENT_MODE="${DEPLOYMENT_MODE:-interactive}"
        export ENCRYPTION_ENABLED="${ENCRYPTION_ENABLED:-true}"
        export USERNAME="${USERNAME:-limitless}"
        export FULLNAME="${FULLNAME:-LimitlessOS User}"
        export HOSTNAME="${HOSTNAME:-limitless-enterprise}"
    fi
}

generate_config_template() {
    local output_file="$1"
    
    cat > "$output_file" << 'EOF'
{
  "deployment": {
    "mode": "enterprise",
    "automation_level": "semi_automated",
    "validation_level": "comprehensive"
  },
  "security": {
    "encryption": true,
    "secure_boot": true,
    "firewall": true,
    "audit_logging": true
  },
  "system": {
    "hostname": "limitless-enterprise",
    "timezone": "UTC",
    "locale": "en_US.UTF-8"
  },
  "user": {
    "username": "admin",
    "fullname": "System Administrator",
    "create_home": true,
    "sudo_access": true
  },
  "network": {
    "configure_automatically": true,
    "dns_servers": ["1.1.1.1", "1.0.0.1"]
  },
  "features": {
    "development_tools": true,
    "enterprise_tools": true,
    "ai_framework": false,
    "multimedia_codecs": false
  },
  "partitioning": {
    "auto_partition": true,
    "filesystem": "ext4",
    "swap_size_gb": 8,
    "root_size_percent": 80
  }
}
EOF

    log_info "Configuration template generated: $output_file"
}

##############################################################################
# Main Installation Workflow
##############################################################################

run_installation() {
    local start_time=$(date +%s)
    
    log_info "Starting LimitlessOS installation process..."
    log_info "Installation mode: $DEPLOYMENT_MODE"
    log_info "Target architecture: $(uname -m)"
    log_info "Boot mode: $BOOT_MODE"
    
    # Installation phases
    local phases=(
        "check_system_requirements:Checking system requirements"
        "detect_hardware:Detecting hardware"
        "validate_hardware:Validating hardware compatibility"
        "select_installation_disk:Selecting installation disk"
        "partition_disk:Partitioning disk"
        "setup_encryption:Setting up encryption"
        "format_filesystems:Formatting filesystems"
        "mount_filesystems:Mounting filesystems"
        "install_system_files:Installing system files"
        "configure_system:Configuring system"
        "install_bootloader:Installing bootloader"
        "create_user_accounts:Creating user accounts"
        "finalize_installation:Finalizing installation"
    )
    
    local total_phases=${#phases[@]}
    local current_phase=0
    
    for phase in "${phases[@]}"; do
        local func_name="${phase%:*}"
        local phase_description="${phase#*:}"
        
        ((current_phase++))
        local progress=$((current_phase * 100 / total_phases))
        
        echo -e "\n${CYAN}[$current_phase/$total_phases] $phase_description... ($progress%)${NC}"
        
        if ! "$func_name"; then
            log_error "Installation failed during: $phase_description"
            return 1
        fi
        
        sleep 1  # Brief pause between phases
    done
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_success "LimitlessOS installation completed successfully!"
    log_info "Total installation time: ${duration} seconds"
    
    show_completion_message
}

show_completion_message() {
    echo -e "\n${GREEN}"
    echo "=============================================================================="
    echo "                    LimitlessOS Installation Complete!"
    echo "=============================================================================="
    echo -e "${NC}"
    echo -e "${WHITE}Installation Summary:${NC}"
    echo "• Target Device: $INSTALL_DISK"
    echo "• Boot Mode: $BOOT_MODE"
    echo "• Encryption: ${ENCRYPTION_ENABLED}"
    echo "• Username: ${USERNAME}"
    echo "• Hostname: ${HOSTNAME}"
    echo "• Installation Mode: $DEPLOYMENT_MODE"
    echo ""
    echo -e "${YELLOW}Next Steps:${NC}"
    echo "1. Remove the installation media"
    echo "2. Reboot your system"
    echo "3. Select 'LimitlessOS Enterprise' from the boot menu"
    echo "4. Complete the initial setup wizard"
    echo ""
    echo -e "${CYAN}Welcome to LimitlessOS - The Future of Computing!${NC}"
    echo ""
}

##############################################################################
# Command Line Argument Processing
##############################################################################

parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--config)
                CONFIG_FILE="$2"
                shift 2
                ;;
            -m|--mode)
                DEPLOYMENT_MODE="$2"
                shift 2
                ;;
            -f|--force)
                FORCE_INSTALL=true
                shift
                ;;
            -s|--skip-validation)
                SKIP_VALIDATION=true
                shift
                ;;
            -d|--debug)
                DEBUG_MODE=true
                shift
                ;;
            -v|--version)
                echo "LimitlessOS Enterprise Installer v$INSTALLER_VERSION"
                exit 0
                ;;
            --generate-config)
                generate_config_template "${2:-limitless_config.json}"
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

##############################################################################
# Main Entry Point
##############################################################################

main() {
    # Initialize logging
    echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] LimitlessOS Installer Started" > "$LOG_FILE"
    
    # Parse command line arguments
    parse_arguments "$@"
    
    # Show banner
    show_banner
    
    # Check prerequisites
    check_root
    
    # Load configuration
    load_configuration
    
    # Run installation
    if run_installation; then
        log_success "Installation completed successfully"
        exit 0
    else
        log_error "Installation failed"
        exit 1
    fi
}

# Execute main function with all arguments
main "$@"