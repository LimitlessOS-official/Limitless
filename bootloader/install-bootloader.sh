#!/bin/bash
#
# LimitlessOS v2.0.0 Bootloader Installation Script
# Modern GRUB2 bootloader installation (Ubuntu-style approach)
#

set -e

# Configuration
LIMITLESSOS_VERSION="2.0.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GRUB_CFG_DIR="/boot/grub"
EFI_DIR="/boot/efi"
INSTALL_LOG="/var/log/limitlessos-bootloader-install.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging function
log() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$INSTALL_LOG"
}

info() {
    echo -e "${BLUE}[INFO]${NC} $1"
    log "INFO: $1"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
    log "WARN: $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    log "ERROR: $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
    log "SUCCESS: $1"
}

# Check if running as root
check_root() {
    if [[ $EUID -ne 0 ]]; then
        error "This script must be run as root"
        exit 1
    fi
}

# Detect boot mode (UEFI or BIOS)
detect_boot_mode() {
    if [[ -d /sys/firmware/efi ]]; then
        BOOT_MODE="UEFI"
        info "Detected UEFI boot mode"
    else
        BOOT_MODE="BIOS"
        info "Detected BIOS boot mode"
    fi
}

# Create necessary directories
create_directories() {
    info "Creating bootloader directories..."
    
    mkdir -p "$GRUB_CFG_DIR"
    mkdir -p /boot/kernel
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        mkdir -p "$EFI_DIR/EFI/LimitlessOS"
        mkdir -p "$EFI_DIR/EFI/BOOT"
    fi
    
    success "Directories created successfully"
}

# Install GRUB for BIOS systems
install_grub_bios() {
    info "Installing GRUB for BIOS systems..."
    
    # Detect target device
    TARGET_DEVICE=$(lsblk -no PKNAME $(findmnt -no SOURCE /))
    if [[ -z "$TARGET_DEVICE" ]]; then
        TARGET_DEVICE="/dev/sda"
        warn "Could not detect target device, using default: $TARGET_DEVICE"
    else
        TARGET_DEVICE="/dev/$TARGET_DEVICE"
        info "Target device: $TARGET_DEVICE"
    fi
    
    # Install GRUB to MBR
    if command -v grub-install &> /dev/null; then
        grub-install --target=i386-pc --boot-directory=/boot "$TARGET_DEVICE"
        success "GRUB installed to $TARGET_DEVICE"
    elif command -v grub2-install &> /dev/null; then
        grub2-install --target=i386-pc --boot-directory=/boot "$TARGET_DEVICE"
        success "GRUB2 installed to $TARGET_DEVICE"
    else
        error "GRUB installation command not found"
        return 1
    fi
}

# Install GRUB for UEFI systems
install_grub_uefi() {
    info "Installing GRUB for UEFI systems..."
    
    # Install GRUB EFI
    if command -v grub-install &> /dev/null; then
        grub-install --target=x86_64-efi --efi-directory="$EFI_DIR" --bootloader-id=LimitlessOS --recheck
    elif command -v grub2-install &> /dev/null; then
        grub2-install --target=x86_64-efi --efi-directory="$EFI_DIR" --bootloader-id=LimitlessOS --recheck
    else
        error "GRUB installation command not found"
        return 1
    fi
    
    # Create fallback bootloader
    cp "$EFI_DIR/EFI/LimitlessOS/grubx64.efi" "$EFI_DIR/EFI/BOOT/BOOTX64.EFI"
    
    success "GRUB EFI installed successfully"
}

# Copy configuration files
install_config_files() {
    info "Installing GRUB configuration files..."
    
    # Copy main GRUB configuration
    if [[ -f "$SCRIPT_DIR/grub/grub.cfg" ]]; then
        cp "$SCRIPT_DIR/grub/grub.cfg" "$GRUB_CFG_DIR/grub.cfg"
        chmod 644 "$GRUB_CFG_DIR/grub.cfg"
        success "Main GRUB configuration installed"
    else
        warn "Main GRUB configuration not found, generating basic config"
        generate_basic_config
    fi
    
    # Copy default settings
    if [[ -f "$SCRIPT_DIR/grub/default" ]]; then
        cp "$SCRIPT_DIR/grub/default" "/etc/default/grub"
        chmod 644 "/etc/default/grub"
        success "Default GRUB settings installed"
    fi
    
    # Copy theme files if they exist
    if [[ -d "$SCRIPT_DIR/grub/themes" ]]; then
        cp -r "$SCRIPT_DIR/grub/themes" "$GRUB_CFG_DIR/"
        success "GRUB themes installed"
    fi
    
    # Copy background image if it exists
    if [[ -f "$SCRIPT_DIR/grub/background.png" ]]; then
        cp "$SCRIPT_DIR/grub/background.png" "$GRUB_CFG_DIR/"
        success "Background image installed"
    fi
}

# Generate basic GRUB configuration if none exists
generate_basic_config() {
    cat > "$GRUB_CFG_DIR/grub.cfg" << 'EOF'
# LimitlessOS v2.0.0 - Basic GRUB Configuration
set timeout=5
set default=0

insmod part_gpt
insmod part_msdos
insmod fat
insmod ext2
insmod multiboot2
insmod all_video
insmod gfxterm

set gfxmode=auto
set gfxpayload=keep
terminal_output gfxterm

menuentry 'LimitlessOS v2.0.0' {
    echo 'Loading LimitlessOS v2.0.0...'
    multiboot2 /boot/kernel/limitless.elf
    boot
}

menuentry 'LimitlessOS v2.0.0 (Recovery)' {
    echo 'Loading LimitlessOS v2.0.0 in recovery mode...'
    multiboot2 /boot/kernel/limitless.elf recovery
    boot
}

menuentry 'Reboot' {
    reboot
}

menuentry 'Shutdown' {
    halt
}
EOF
    chmod 644 "$GRUB_CFG_DIR/grub.cfg"
}

# Update GRUB configuration
update_grub_config() {
    info "Updating GRUB configuration..."
    
    if command -v update-grub &> /dev/null; then
        update-grub
    elif command -v grub-mkconfig &> /dev/null; then
        grub-mkconfig -o "$GRUB_CFG_DIR/grub.cfg"
    elif command -v grub2-mkconfig &> /dev/null; then
        grub2-mkconfig -o "$GRUB_CFG_DIR/grub.cfg"
    else
        warn "No GRUB config update command found, using static configuration"
    fi
    
    success "GRUB configuration updated"
}

# Verify installation
verify_installation() {
    info "Verifying bootloader installation..."
    
    local errors=0
    
    # Check if GRUB config exists
    if [[ ! -f "$GRUB_CFG_DIR/grub.cfg" ]]; then
        error "GRUB configuration file not found"
        ((errors++))
    fi
    
    # Check if kernel exists
    if [[ ! -f "/boot/kernel/limitless.elf" ]]; then
        warn "LimitlessOS kernel not found at /boot/kernel/limitless.elf"
        info "Make sure to copy your kernel to the correct location"
    fi
    
    # Check EFI installation for UEFI systems
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        if [[ ! -f "$EFI_DIR/EFI/LimitlessOS/grubx64.efi" ]]; then
            error "GRUB EFI binary not found"
            ((errors++))
        fi
        
        if [[ ! -f "$EFI_DIR/EFI/BOOT/BOOTX64.EFI" ]]; then
            error "Fallback EFI bootloader not found"
            ((errors++))
        fi
    fi
    
    if [[ $errors -eq 0 ]]; then
        success "Bootloader installation verified successfully"
        return 0
    else
        error "Bootloader installation verification failed ($errors errors)"
        return 1
    fi
}

# Main installation function
main() {
    info "Starting LimitlessOS v$LIMITLESSOS_VERSION bootloader installation..."
    
    check_root
    detect_boot_mode
    create_directories
    
    if [[ "$BOOT_MODE" == "UEFI" ]]; then
        install_grub_uefi
    else
        install_grub_bios
    fi
    
    install_config_files
    update_grub_config
    
    if verify_installation; then
        success "LimitlessOS bootloader installation completed successfully!"
        info "Boot mode: $BOOT_MODE"
        info "Version: $LIMITLESSOS_VERSION"
        info "Log file: $INSTALL_LOG"
        
        echo
        echo "Next steps:"
        echo "1. Copy your LimitlessOS kernel to /boot/kernel/limitless.elf"
        echo "2. Reboot to test the new bootloader"
        echo "3. Check $INSTALL_LOG for detailed installation log"
    else
        error "Bootloader installation completed with errors"
        error "Please check $INSTALL_LOG for details"
        exit 1
    fi
}

# Handle command line arguments
case "${1:-}" in
    --help|-h)
        echo "LimitlessOS v$LIMITLESSOS_VERSION Bootloader Installer"
        echo
        echo "Usage: $0 [options]"
        echo
        echo "Options:"
        echo "  --help, -h     Show this help message"
        echo "  --version, -v  Show version information"
        echo "  --verify       Verify existing installation"
        echo
        exit 0
        ;;
    --version|-v)
        echo "LimitlessOS Bootloader Installer v$LIMITLESSOS_VERSION"
        exit 0
        ;;
    --verify)
        check_root
        detect_boot_mode
        verify_installation
        exit $?
        ;;
    "")
        main
        ;;
    *)
        error "Unknown option: $1"
        echo "Use --help for usage information"
        exit 1
        ;;
esac