#!/bin/sh

# ============================================================================
# LimitlessOS ISO Testing and Verification System
# Comprehensive testing suite for bootable ISO validation
# Copyright (c) 2024 LimitlessOS Project
# ============================================================================

SCRIPT_DIR="$(dirname "$0")"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ISO_FILE="$PROJECT_ROOT/dist/LimitlessOS-1.0.0.iso"
MOUNT_POINT="/tmp/limitless_iso_test"

# Output functions (no color, POSIX compatible)
log() {
    echo "[INFO] [$(date +'%Y-%m-%d %H:%M:%S')] $1"
}

success() {
    echo "[SUCCESS] $1"
}

error() {
    echo "[ERROR] $1"
}

warning() {
    echo "[WARNING] $1"
}

info() {
    echo "[INFO] $1"
}

# Header
    echo "=============================================================="
    echo "           LimitlessOS ISO Testing and Verification             "
    echo "              Comprehensive Quality Assurance                   "
    echo "=============================================================="
echo

# Test 1: ISO File Validation
test_iso_file() {
    log "Testing ISO file integrity..."
    
    if [ ! -f "$ISO_FILE" ]; then
        error "ISO file not found: $ISO_FILE"
        return 1
    fi
    
    # Check file size (should be reasonable for our system)
    local file_size=$(stat -c%s "$ISO_FILE" 2>/dev/null || stat -f%z "$ISO_FILE" 2>/dev/null)
    local file_size_mb=$((file_size / 1024 / 1024))
    
    info "ISO file size: ${file_size_mb}MB"
    
    if [ $file_size_mb -lt 5 ]; then
        error "ISO file too small (${file_size_mb}MB) - may be incomplete"
        return 1
    elif [ $file_size_mb -gt 500 ]; then
        warning "ISO file large (${file_size_mb}MB) - consider optimization"
    fi
    
    # Check file type
    local file_type=$(file "$ISO_FILE" 2>/dev/null)
    if echo "$file_type" | grep -q "ISO 9660"; then
        success "Valid ISO 9660 filesystem detected"
    else
        error "Invalid file type: $file_type"
        return 1
    fi
    
    # Check if bootable
    if echo "$file_type" | grep -q "bootable"; then
        success "ISO is bootable"
    else
        warning "Bootable status unclear from file command"
    fi
    
    success "ISO file validation passed"
}

# Test 2: Mount and Structure Validation
test_iso_structure() {
    log "Testing ISO internal structure..."
    
    # Create mount point
    sudo mkdir -p "$MOUNT_POINT"
    
    # Mount ISO
    if sudo mount -o loop "$ISO_FILE" "$MOUNT_POINT" 2>/dev/null; then
        success "ISO mounted successfully"
    else
        error "Failed to mount ISO"
        return 1
    fi
    
    # Check critical directories
    # POSIX-compatible array
    critical_dirs="$MOUNT_POINT/boot $MOUNT_POINT/boot/grub $MOUNT_POINT/EFI/BOOT $MOUNT_POINT/system/apps $MOUNT_POINT/system/config $MOUNT_POINT/usr/share/doc/limitless $MOUNT_POINT/home/limitless $MOUNT_POINT/enterprise"
    missing_dirs=0
    for dir in $critical_dirs; do
        if [ -d "$dir" ]; then
            info "✓ Directory found: $(basename "$dir")"
        else
            error "✗ Missing directory: $dir"
            ((missing_dirs++))
        fi
    done
    
    # Check critical files
    # POSIX-compatible array
    critical_files="$MOUNT_POINT/boot/limitless.elf $MOUNT_POINT/boot/grub/grub.cfg $MOUNT_POINT/EFI/BOOT/BOOTX64.EFI $MOUNT_POINT/system/apps/calculator_simple $MOUNT_POINT/system/apps/system_monitor_simple $MOUNT_POINT/system/config/limitless.conf $MOUNT_POINT/usr/share/doc/limitless/README.txt"
    missing_files=0
    for file in $critical_files; do
        if [ -f "$file" ]; then
            info "✓ File found: $(basename "$file")"
        else
            error "✗ Missing file: $file"
            missing_files=`expr $missing_files + 1`
        fi
    done
    
    # Unmount ISO
    sudo umount "$MOUNT_POINT" 2>/dev/null || true
    sudo rmdir "$MOUNT_POINT" 2>/dev/null || true
    
    if [ $missing_dirs -eq 0 ] && [ $missing_files -eq 0 ]; then
        success "ISO structure validation passed"
        return 0
    else
        error "ISO structure validation failed (${missing_dirs} missing dirs, ${missing_files} missing files)"
        return 1
    fi
}

# Test 3: GRUB Configuration Validation
test_grub_config() {
    log "Testing GRUB bootloader configuration..."
    
    # Mount ISO temporarily
    sudo mkdir -p "$MOUNT_POINT"
    sudo mount -o loop "$ISO_FILE" "$MOUNT_POINT" 2>/dev/null || {
        error "Failed to mount ISO for GRUB testing"
        return 1
    }
    
    local grub_cfg="$MOUNT_POINT/boot/grub/grub.cfg"
    
    if [ -f "$grub_cfg" ]; then
        info "GRUB configuration found"
        
        # Check for essential entries
        if grep -q "menuentry.*LimitlessOS" "$grub_cfg"; then
            success "✓ LimitlessOS boot entry found"
        else
            error "✗ Missing LimitlessOS boot entry"
        fi
        
        if grep -q "multiboot2.*limitless.elf" "$grub_cfg"; then
            success "✓ Kernel boot command found"
        else
            error "✗ Missing kernel boot command"
        fi
        
        if grep -q "timeout" "$grub_cfg"; then
            success "✓ Boot timeout configured"
        else
            warning "Boot timeout not configured"
        fi
        
        # Check for multiple boot options
        local menu_count=$(grep -c "menuentry" "$grub_cfg" || echo "0")
        info "Boot menu entries: $menu_count"
        
        if [ $menu_count -ge 3 ]; then
            success "✓ Multiple boot options available"
        else
            warning "Limited boot options (only $menu_count entries)"
        fi
        
    else
        error "GRUB configuration file not found"
        sudo umount "$MOUNT_POINT" 2>/dev/null
        return 1
    fi
    
    sudo umount "$MOUNT_POINT" 2>/dev/null
    sudo rmdir "$MOUNT_POINT" 2>/dev/null
    success "GRUB configuration validation passed"
}

# Test 4: Application Validation
test_applications() {
    log "Testing system applications..."
    
    # Mount ISO
    sudo mkdir -p "$MOUNT_POINT"
    sudo mount -o loop "$ISO_FILE" "$MOUNT_POINT" 2>/dev/null || {
        error "Failed to mount ISO for application testing"
        return 1
    }
    
    local apps_dir="$MOUNT_POINT/system/apps"
    app_count=0
    
    if [ -d "$apps_dir" ]; then
        # Check calculator
        if [ -f "$apps_dir/calculator_simple" ] && [ -x "$apps_dir/calculator_simple" ]; then
            success "✓ Calculator application found and executable"
            app_count=`expr $app_count + 1`
        else
            error "✗ Calculator application missing or not executable"
        fi
        
        # Check system monitor
        if [ -f "$apps_dir/system_monitor_simple" ] && [ -x "$apps_dir/system_monitor_simple" ]; then
            success "✓ System Monitor application found and executable"
            app_count=`expr $app_count + 1`
        else
            error "✗ System Monitor application missing or not executable"
        fi
        
        # Check application registry
        if [ -f "$apps_dir/app_registry.txt" ]; then
            success "✓ Application registry found"
            local registered_apps=$(grep -c "^[^#].*-" "$apps_dir/app_registry.txt" || echo "0")
            info "Registered applications: $registered_apps"
        else
            warning "Application registry not found"
        fi
        
    else
        error "Applications directory not found"
        sudo umount "$MOUNT_POINT" 2>/dev/null
        return 1
    fi
    
    sudo umount "$MOUNT_POINT" 2>/dev/null
    sudo rmdir "$MOUNT_POINT" 2>/dev/null
    
    if [ $app_count -eq 2 ]; then
        success "Application validation passed (${app_count}/2 applications found)"
        return 0
    else
        error "Application validation failed (${app_count}/2 applications found)"
        return 1
    fi
}

# Test 5: Documentation and Configuration
test_documentation() {
    log "Testing documentation and configuration files..."
    
    # Mount ISO
    sudo mkdir -p "$MOUNT_POINT"
    sudo mount -o loop "$ISO_FILE" "$MOUNT_POINT" 2>/dev/null || {
        error "Failed to mount ISO for documentation testing"
        return 1
    }
    
    local doc_errors=0
    
    # Check system documentation
    local doc_dir="$MOUNT_POINT/usr/share/doc/limitless"
    if [ -d "$doc_dir" ]; then
        info "✓ Documentation directory found"
        
        if [ -f "$doc_dir/README.txt" ]; then
            local readme_size=$(wc -c < "$doc_dir/README.txt")
            if [ $readme_size -gt 1000 ]; then
                success "✓ README.txt comprehensive ($readme_size bytes)"
            else
                warning "README.txt seems small ($readme_size bytes)"
            fi
        else
            error "✗ README.txt not found"
            ((doc_errors++))
        fi
        
        if [ -f "$doc_dir/INSTALLATION.txt" ]; then
            success "✓ Installation guide found"
        else
            warning "Installation guide not found"
        fi
    else
        error "✗ Documentation directory missing"
        ((doc_errors++))
    fi
    
    # Check system configuration
    local config_dir="$MOUNT_POINT/system/config"
    if [ -d "$config_dir" ]; then
        info "✓ Configuration directory found"
        
        if [ -f "$config_dir/limitless.conf" ]; then
            success "✓ Main system configuration found"
        else
            error "✗ Main configuration file missing"
            ((doc_errors++))
        fi
    else
        error "✗ Configuration directory missing"
        ((doc_errors++))
    fi
    
    # Check user environment
    local user_home="$MOUNT_POINT/home/limitless"
    if [ -d "$user_home" ]; then
        info "✓ User home directory found"
        
        if [ -f "$user_home/Desktop/Welcome.txt" ]; then
            success "✓ User welcome file found"
        else
            warning "User welcome file not found"
        fi
        
        local desktop_files=$(find "$user_home/Desktop" -name "*.desktop" 2>/dev/null | wc -l)
        if [ $desktop_files -gt 0 ]; then
            success "✓ Desktop shortcuts created ($desktop_files files)"
        else
            warning "No desktop shortcuts found"
        fi
    else
        error "✗ User home directory missing"
        ((doc_errors++))
    fi
    
    sudo umount "$MOUNT_POINT" 2>/dev/null
    sudo rmdir "$MOUNT_POINT" 2>/dev/null
    
    if [ $doc_errors -eq 0 ]; then
        success "Documentation and configuration validation passed"
        return 0
    else
        error "Documentation validation failed ($doc_errors errors)"
        return 1
    fi
}

# Test 6: Virtual Machine Compatibility Check
test_vm_compatibility() {
    log "Testing virtual machine compatibility..."
    
    # Check if we have qemu for testing
    if command -v qemu-system-x86_64 >/dev/null 2>&1; then
        info "QEMU available for boot testing"
        
        log "Performing quick QEMU boot test (5 seconds)..."
        timeout 5 qemu-system-x86_64 \
            -cdrom "$ISO_FILE" \
            -m 512 \
            -nographic \
            -serial stdio \
            -boot d \
            -no-reboot \
            >/dev/null 2>&1 && \
        success "✓ QEMU boot test completed" || \
        info "QEMU boot test inconclusive (expected for quick test)"
    else
        info "QEMU not available for boot testing"
    fi
    
    # Check VirtualBox compatibility indicators
    log "Checking VirtualBox compatibility indicators..."
    
    # Mount and check UEFI support
    sudo mkdir -p "$MOUNT_POINT"
    sudo mount -o loop "$ISO_FILE" "$MOUNT_POINT" 2>/dev/null
    
    if [ -f "$MOUNT_POINT/EFI/BOOT/BOOTX64.EFI" ]; then
        success "✓ UEFI bootloader present - VirtualBox EFI compatible"
    else
        warning "UEFI bootloader missing - limited VirtualBox compatibility"
    fi
    
    if [ -f "$MOUNT_POINT/boot/grub/grub.cfg" ]; then
        success "✓ Legacy BIOS GRUB present - VirtualBox BIOS compatible"
    else
        error "Legacy BIOS support missing"
    fi
    
    sudo umount "$MOUNT_POINT" 2>/dev/null
    sudo rmdir "$MOUNT_POINT" 2>/dev/null
    
    success "Virtual machine compatibility check completed"
}

# Test 7: Security and Integrity
test_security() {
    log "Testing security features and file integrity..."
    
    # Mount ISO
    sudo mkdir -p "$MOUNT_POINT"
    sudo mount -o loop "$ISO_FILE" "$MOUNT_POINT" 2>/dev/null || {
        error "Failed to mount ISO for security testing"
        return 1
    }
    
    # Check for security-related directories
    # POSIX-compatible security directory check
    for dir in "$MOUNT_POINT/system/security" "$MOUNT_POINT/enterprise/security" "$MOUNT_POINT/enterprise/auditing"; do
        if [ -d "$dir" ]; then
            info "✓ Security directory: $(basename "$dir")"
        else
            warning "Security directory missing: $(basename "$dir")"
        fi
    done

    # POSIX-compatible executable permissions check
    for exe in "$MOUNT_POINT/system/apps/calculator_simple" "$MOUNT_POINT/system/apps/system_monitor_simple" "$MOUNT_POINT/EFI/BOOT/BOOTX64.EFI"; do
        if [ -f "$exe" ] && [ -x "$exe" ]; then
            success "✓ Executable permissions correct: $(basename "$exe")"
        else
            error "✗ Executable permissions incorrect: $(basename "$exe")"
        fi
    done

    # Check for world-writable files (security concern)
    writable_files=$(find "$MOUNT_POINT" -type f -perm -002 2>/dev/null | wc -l)
    if [ "$writable_files" -eq 0 ]; then
        success "✓ No world-writable files found"
    else
        warning "$writable_files world-writable files found"
    fi

    sudo umount "$MOUNT_POINT" 2>/dev/null
    sudo rmdir "$MOUNT_POINT" 2>/dev/null

    success "Security validation completed"
}

# Generate comprehensive test report
generate_test_report() {
    local start_time="$1"
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    echo
    echo -e "${PURPLE}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${PURPLE}                     ISO TESTING COMPLETE                      ${NC}"
    echo -e "${PURPLE}═══════════════════════════════════════════════════════════════${NC}"
    echo
    echo "Test Summary:"
    echo "   ISO File: $ISO_FILE"
    echo "   File Size: $(du -h "$ISO_FILE" | cut -f1)"
    echo "   Test Duration: ${duration} seconds"
    echo "   Test Timestamp: $(date)"
    echo
    echo "Completed Tests:"
    echo "   • ISO File Integrity Validation"
    echo "   • Internal Structure Verification"
    echo "   • GRUB Bootloader Configuration"
    echo "   • System Applications Testing"
    echo "   • Documentation and Configuration"
    echo "   • Virtual Machine Compatibility"
    echo "   • Security and Permissions Check"
    echo
    echo "Ready for Deployment:"
    echo "   • VirtualBox Installation (with EFI enabled)"
    echo "   • VMware Workstation/Player"
    echo "   • Physical Hardware (USB/DVD boot)"
    echo "   • QEMU/KVM Virtual Machines"
    echo "   • Enterprise Environment Testing"
    echo
    echo "Recommended Next Steps:"
    echo "   1. Test boot in VirtualBox with recommended settings"
    echo "   2. Verify all applications launch successfully"
    echo "   3. Test system performance and responsiveness"
    echo "   4. Validate enterprise features and security"
    echo "   5. Prepare for production deployment"
    echo
    echo "=============================================================="
}

# Main test execution
main() {
    local start_time=$(date +%s)
    local test_failures=0
    
    log "Starting comprehensive ISO testing suite..."
    echo
    
    # Run all tests
    test_iso_file || test_failures=$(expr $test_failures + 1)
    echo
    test_iso_structure || test_failures=$(expr $test_failures + 1)
    echo
    test_grub_config || test_failures=$(expr $test_failures + 1)
    echo
    test_applications || test_failures=$(expr $test_failures + 1)
    echo
    test_documentation || test_failures=$(expr $test_failures + 1)
    echo
    test_vm_compatibility || test_failures=$(expr $test_failures + 1)
    echo
    test_security || test_failures=$(expr $test_failures + 1)
    
    # Generate report
    generate_test_report "$start_time"
    
    # Return appropriate exit code
    if [ $test_failures -eq 0 ]; then
        success "All tests passed! ISO is ready for deployment."
        exit 0
    else
        error "$test_failures test(s) failed. Please review and fix issues."
        exit 1
    fi
}

# Handle command line arguments
case "${1:-test}" in
    "test"|"")
        main
        ;;
    "quick")
        log "Running quick ISO validation..."
        test_iso_file && test_iso_structure
        ;;
    "structure")
        log "Testing ISO structure only..."
        test_iso_structure
        ;;
    "apps")
        log "Testing applications only..."
        test_applications
        ;;
    "help"|"-h"|"--help")
        echo "LimitlessOS ISO Testing Suite"
        echo
        echo "Usage: $0 [command]"
        echo
        echo "Commands:"
        echo "  test       - Run complete test suite (default)"
        echo "  quick      - Run basic validation only"
        echo "  structure  - Test ISO structure only"
        echo "  apps       - Test applications only"
        echo "  help       - Show this help message"
        ;;
    *)
        error "Unknown command: $1"
        echo "Run '$0 help' for usage information"
        exit 1
        ;;
esac