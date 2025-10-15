#!/bin/bash

##############################################################################
# LimitlessOS Installer Validation and Testing Suite
# Comprehensive testing framework for installer validation
##############################################################################

set -e

# Test Configuration
INSTALLER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_LOG_FILE="/tmp/limitless_installer_test.log"
VALIDATION_RESULTS_FILE="/tmp/limitless_validation_results.json"
TEST_VERBOSE=false
TEST_MODE="full"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

##############################################################################
# Logging and Output Functions
##############################################################################

log_test() {
    local level="$1"
    shift
    local message="$*"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    echo "[$timestamp] [TEST-$level] $message" >> "$TEST_LOG_FILE"
    
    case "$level" in
        "PASS")
            echo -e "${GREEN}[PASS]${NC} $message"
            ;;
        "FAIL")
            echo -e "${RED}[FAIL]${NC} $message"
            ;;
        "WARN")
            echo -e "${YELLOW}[WARN]${NC} $message"
            ;;
        "INFO")
            echo -e "${BLUE}[INFO]${NC} $message"
            ;;
        "DEBUG")
            if [[ "$TEST_VERBOSE" == true ]]; then
                echo -e "${PURPLE}[DEBUG]${NC} $message"
            fi
            ;;
    esac
}

##############################################################################
# Test Infrastructure
##############################################################################

test_results=()
test_count=0
pass_count=0
fail_count=0
warn_count=0

run_test() {
    local test_name="$1"
    local test_function="$2"
    local test_description="$3"
    
    ((test_count++))
    
    echo ""
    echo "=========================================="
    echo "Test $test_count: $test_name"
    echo "Description: $test_description"
    echo "=========================================="
    
    local start_time=$(date +%s.%N)
    
    if $test_function; then
        local result="PASS"
        ((pass_count++))
        log_test "PASS" "$test_name"
    else
        local result="FAIL"
        ((fail_count++))
        log_test "FAIL" "$test_name"
    fi
    
    local end_time=$(date +%s.%N)
    local duration=$(echo "$end_time - $start_time" | bc -l 2>/dev/null || echo "0")
    
    test_results+=("$test_name:$result:$duration")
}

##############################################################################
# Hardware Validation Tests
##############################################################################

test_system_requirements() {
    log_test "INFO" "Testing system requirements validation"
    
    # Test minimum RAM check
    local ram_kb=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    local ram_gb=$((ram_kb / 1024 / 1024))
    
    if [[ $ram_gb -ge 4 ]]; then
        log_test "PASS" "RAM requirement met: ${ram_gb}GB"
    else
        log_test "FAIL" "Insufficient RAM: ${ram_gb}GB (4GB required)"
        return 1
    fi
    
    # Test CPU cores
    local cpu_cores=$(nproc)
    if [[ $cpu_cores -ge 2 ]]; then
        log_test "PASS" "CPU cores requirement met: $cpu_cores"
    else
        log_test "FAIL" "Insufficient CPU cores: $cpu_cores (2 required)"
        return 1
    fi
    
    # Test disk space
    local disk_space_gb=$(df / | tail -1 | awk '{print int($4/1024/1024)}')
    if [[ $disk_space_gb -ge 50 ]]; then
        log_test "PASS" "Disk space requirement met: ${disk_space_gb}GB"
    else
        log_test "FAIL" "Insufficient disk space: ${disk_space_gb}GB (50GB required)"
        return 1
    fi
    
    return 0
}

test_hardware_detection() {
    log_test "INFO" "Testing hardware detection functionality"
    
    # Test CPU detection
    local cpu_model=$(grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)
    if [[ -n "$cpu_model" ]]; then
        log_test "PASS" "CPU detected: $cpu_model"
    else
        log_test "FAIL" "CPU detection failed"
        return 1
    fi
    
    # Test memory detection
    local total_mem=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    if [[ $total_mem -gt 0 ]]; then
        log_test "PASS" "Memory detected: $((total_mem / 1024 / 1024))GB"
    else
        log_test "FAIL" "Memory detection failed"
        return 1
    fi
    
    # Test storage detection
    local storage_devices=$(lsblk -dn -o NAME | grep -E '^(sd|nvme|vd)' | wc -l)
    if [[ $storage_devices -gt 0 ]]; then
        log_test "PASS" "Storage devices detected: $storage_devices"
    else
        log_test "FAIL" "No storage devices detected"
        return 1
    fi
    
    # Test network interfaces
    local network_interfaces=$(ip link show | grep -E '^[0-9]+:' | grep -v lo | wc -l)
    if [[ $network_interfaces -gt 0 ]]; then
        log_test "PASS" "Network interfaces detected: $network_interfaces"
    else
        log_test "WARN" "No network interfaces detected"
    fi
    
    return 0
}

test_virtualization_support() {
    log_test "INFO" "Testing virtualization support detection"
    
    if grep -q vmx /proc/cpuinfo || grep -q svm /proc/cpuinfo; then
        log_test "PASS" "Hardware virtualization support detected"
    else
        log_test "WARN" "No hardware virtualization support"
    fi
    
    return 0
}

##############################################################################
# Installation Script Tests
##############################################################################

test_installer_scripts_exist() {
    log_test "INFO" "Testing installer script existence"
    
    local required_scripts=(
        "install_limitless_enterprise.sh"
        "install_silent.sh"
        "install_developer.sh"
        "install_server.sh"
        "install_kiosk.sh"
    )
    
    local all_exist=true
    
    for script in "${required_scripts[@]}"; do
        if [[ -f "$INSTALLER_DIR/$script" ]]; then
            log_test "PASS" "Script exists: $script"
        else
            log_test "FAIL" "Missing script: $script"
            all_exist=false
        fi
    done
    
    return $([[ "$all_exist" == true ]])
}

test_installer_script_syntax() {
    log_test "INFO" "Testing installer script syntax"
    
    local scripts=(
        "install_limitless_enterprise.sh"
        "install_silent.sh"
        "install_developer.sh"
        "install_server.sh"
        "install_kiosk.sh"
    )
    
    local all_valid=true
    
    for script in "${scripts[@]}"; do
        if [[ -f "$INSTALLER_DIR/$script" ]]; then
            if bash -n "$INSTALLER_DIR/$script"; then
                log_test "PASS" "Syntax check passed: $script"
            else
                log_test "FAIL" "Syntax error in: $script"
                all_valid=false
            fi
        fi
    done
    
    return $([[ "$all_valid" == true ]])
}

test_configuration_files() {
    log_test "INFO" "Testing configuration file handling"
    
    # Test JSON configuration template generation
    local temp_config="/tmp/test_config.json"
    if command -v jq >/dev/null 2>&1; then
        cat > "$temp_config" << 'EOF'
{
  "deployment": {
    "mode": "test",
    "automation_level": "semi_automated"
  },
  "security": {
    "encryption": true,
    "secure_boot": false
  },
  "system": {
    "hostname": "test-system",
    "timezone": "UTC"
  }
}
EOF
        
        if jq empty "$temp_config" 2>/dev/null; then
            log_test "PASS" "JSON configuration validation works"
        else
            log_test "FAIL" "JSON configuration validation failed"
            rm -f "$temp_config"
            return 1
        fi
        
        rm -f "$temp_config"
    else
        log_test "WARN" "jq not available for JSON validation"
    fi
    
    return 0
}

##############################################################################
# GUI Component Tests
##############################################################################

test_gui_components() {
    log_test "INFO" "Testing GUI installer components"
    
    # Test GUI header files exist
    local gui_headers=(
        "gui_installer.h"
        "intelligent_installer.h"
        "enterprise_deployment.h"
    )
    
    local headers_exist=true
    
    for header in "${gui_headers[@]}"; do
        if [[ -f "$INSTALLER_DIR/$header" ]]; then
            log_test "PASS" "GUI header exists: $header"
        else
            log_test "FAIL" "Missing GUI header: $header"
            headers_exist=false
        fi
    done
    
    # Test C source files exist
    local gui_sources=(
        "gui_installer.c"
        "intelligent_installer.c"
        "enterprise_deployment.c"
        "complete_installer.c"
    )
    
    for source in "${gui_sources[@]}"; do
        if [[ -f "$INSTALLER_DIR/$source" ]]; then
            log_test "PASS" "GUI source exists: $source"
        else
            log_test "FAIL" "Missing GUI source: $source"
            headers_exist=false
        fi
    done
    
    return $([[ "$headers_exist" == true ]])
}

test_gui_compilation() {
    log_test "INFO" "Testing GUI installer compilation"
    
    # Check if we have a C compiler
    if ! command -v gcc >/dev/null 2>&1; then
        log_test "WARN" "GCC not available for compilation test"
        return 0
    fi
    
    # Test basic header syntax
    local gui_headers=(
        "gui_installer.h"
        "intelligent_installer.h"
        "enterprise_deployment.h"
    )
    
    local compilation_ok=true
    
    for header in "${gui_headers[@]}"; do
        if [[ -f "$INSTALLER_DIR/$header" ]]; then
            # Create a minimal test program
            local test_file="/tmp/test_${header%.h}.c"
            cat > "$test_file" << EOF
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "$INSTALLER_DIR/$header"

int main() {
    printf("Header compilation test\\n");
    return 0;
}
EOF
            
            if gcc -c "$test_file" -o "/tmp/test_${header%.h}.o" 2>/dev/null; then
                log_test "PASS" "Header compiles: $header"
                rm -f "$test_file" "/tmp/test_${header%.h}.o"
            else
                log_test "FAIL" "Header compilation failed: $header"
                compilation_ok=false
                rm -f "$test_file"
            fi
        fi
    done
    
    return $([[ "$compilation_ok" == true ]])
}

##############################################################################
# Security Tests
##############################################################################

test_security_features() {
    log_test "INFO" "Testing security feature validation"
    
    # Test UEFI/Secure Boot detection
    if [[ -d /sys/firmware/efi ]]; then
        log_test "PASS" "UEFI firmware detected"
        
        # Check for Secure Boot
        if [[ -f /sys/firmware/efi/efivars/SecureBoot-* ]]; then
            log_test "PASS" "Secure Boot variables present"
        else
            log_test "WARN" "Secure Boot variables not found"
        fi
    else
        log_test "INFO" "BIOS firmware detected (not UEFI)"
    fi
    
    # Test TPM availability
    if [[ -c /dev/tpm0 ]] || [[ -c /dev/tpmrm0 ]]; then
        log_test "PASS" "TPM device detected"
    else
        log_test "WARN" "No TPM device found"
    fi
    
    # Test cryptsetup availability for encryption
    if command -v cryptsetup >/dev/null 2>&1; then
        log_test "PASS" "cryptsetup available for encryption"
    else
        log_test "FAIL" "cryptsetup not available (required for encryption)"
        return 1
    fi
    
    return 0
}

test_encryption_capability() {
    log_test "INFO" "Testing disk encryption capability"
    
    # Test if we can create a test LUKS container
    local test_file="/tmp/test_luks_container"
    local test_size="10M"
    
    # Create test file
    if dd if=/dev/zero of="$test_file" bs=1M count=10 2>/dev/null; then
        log_test "PASS" "Created test container file"
    else
        log_test "FAIL" "Failed to create test container"
        return 1
    fi
    
    # Test LUKS formatting
    if echo "test_passphrase" | cryptsetup luksFormat --type luks2 "$test_file" 2>/dev/null; then
        log_test "PASS" "LUKS encryption test successful"
    else
        log_test "FAIL" "LUKS encryption test failed"
        rm -f "$test_file"
        return 1
    fi
    
    # Cleanup
    rm -f "$test_file"
    return 0
}

##############################################################################
# Performance Tests
##############################################################################

test_disk_performance() {
    log_test "INFO" "Testing disk performance"
    
    # Simple disk I/O test
    local test_file="/tmp/disk_performance_test"
    local test_size="100M"
    
    # Test write performance
    local write_start=$(date +%s.%N)
    if dd if=/dev/zero of="$test_file" bs=1M count=100 2>/dev/null; then
        local write_end=$(date +%s.%N)
        local write_time=$(echo "$write_end - $write_start" | bc -l 2>/dev/null || echo "1")
        local write_speed=$(echo "scale=2; 100 / $write_time" | bc -l 2>/dev/null || echo "unknown")
        log_test "PASS" "Write speed: ${write_speed} MB/s"
    else
        log_test "FAIL" "Disk write test failed"
        return 1
    fi
    
    # Test read performance
    sync
    echo 3 > /proc/sys/vm/drop_caches 2>/dev/null || true
    
    local read_start=$(date +%s.%N)
    if dd if="$test_file" of=/dev/null bs=1M 2>/dev/null; then
        local read_end=$(date +%s.%N)
        local read_time=$(echo "$read_end - $read_start" | bc -l 2>/dev/null || echo "1")
        local read_speed=$(echo "scale=2; 100 / $read_time" | bc -l 2>/dev/null || echo "unknown")
        log_test "PASS" "Read speed: ${read_speed} MB/s"
    else
        log_test "FAIL" "Disk read test failed"
        rm -f "$test_file"
        return 1
    fi
    
    # Cleanup
    rm -f "$test_file"
    return 0
}

##############################################################################
# Network Tests
##############################################################################

test_network_connectivity() {
    log_test "INFO" "Testing network connectivity"
    
    # Test DNS resolution
    if nslookup limitlessos.org >/dev/null 2>&1 || host limitlessos.org >/dev/null 2>&1; then
        log_test "PASS" "DNS resolution works"
    else
        log_test "WARN" "DNS resolution failed"
    fi
    
    # Test internet connectivity
    if ping -c 1 -W 5 8.8.8.8 >/dev/null 2>&1; then
        log_test "PASS" "Internet connectivity available"
    else
        log_test "WARN" "Internet connectivity not available"
    fi
    
    return 0
}

##############################################################################
# Integration Tests
##############################################################################

test_installer_integration() {
    log_test "INFO" "Testing installer integration"
    
    # Test that main installer can display help
    if "$INSTALLER_DIR/install_limitless_enterprise.sh" --help >/dev/null 2>&1; then
        log_test "PASS" "Main installer help works"
    else
        log_test "FAIL" "Main installer help failed"
        return 1
    fi
    
    # Test configuration generation
    local temp_config="/tmp/test_generated_config.json"
    if "$INSTALLER_DIR/install_limitless_enterprise.sh" --generate-config "$temp_config" >/dev/null 2>&1; then
        if [[ -f "$temp_config" ]]; then
            log_test "PASS" "Configuration generation works"
            rm -f "$temp_config"
        else
            log_test "FAIL" "Configuration file not generated"
            return 1
        fi
    else
        log_test "FAIL" "Configuration generation failed"
        return 1
    fi
    
    return 0
}

##############################################################################
# Report Generation
##############################################################################

generate_validation_report() {
    local report_file="$1"
    
    cat > "$report_file" << EOF
{
  "validation_summary": {
    "total_tests": $test_count,
    "passed": $pass_count,
    "failed": $fail_count,
    "warnings": $warn_count,
    "success_rate": $(echo "scale=2; $pass_count * 100 / $test_count" | bc -l 2>/dev/null || echo "0"),
    "timestamp": "$(date -Iseconds)",
    "hostname": "$(hostname)",
    "architecture": "$(uname -m)",
    "kernel": "$(uname -r)"
  },
  "test_results": [
EOF

    local first=true
    for result in "${test_results[@]}"; do
        local test_name="${result%%:*}"
        local test_status="${result#*:}"
        test_status="${test_status%%:*}"
        local test_duration="${result##*:}"
        
        if [[ "$first" == true ]]; then
            first=false
        else
            echo "," >> "$report_file"
        fi
        
        cat >> "$report_file" << EOF
    {
      "name": "$test_name",
      "status": "$test_status",
      "duration": $test_duration
    }
EOF
    done
    
    cat >> "$report_file" << EOF
  ],
  "system_info": {
    "cpu_model": "$(grep 'model name' /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)",
    "cpu_cores": $(nproc),
    "memory_gb": $(($(grep MemTotal /proc/meminfo | awk '{print $2}') / 1024 / 1024)),
    "boot_mode": "$([ -d /sys/firmware/efi ] && echo 'UEFI' || echo 'BIOS')",
    "virtualization": "$(grep -q 'vmx\|svm' /proc/cpuinfo && echo 'available' || echo 'not_available')"
  }
}
EOF

    log_test "INFO" "Validation report generated: $report_file"
}

##############################################################################
# Main Test Execution
##############################################################################

show_test_banner() {
    echo -e "${BLUE}"
    echo "=============================================================================="
    echo "                  LimitlessOS Installer Validation Suite"
    echo "=============================================================================="
    echo -e "${NC}"
}

show_help() {
    cat << EOF
LimitlessOS Installer Validation Suite

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -h, --help          Show this help message
    -v, --verbose       Enable verbose output
    -m, --mode MODE     Test mode: full, quick, hardware, security, integration
    -o, --output FILE   Output validation report to file
    
TEST MODES:
    full               Run all tests (default)
    quick              Run quick validation tests only
    hardware           Hardware detection and validation tests
    security           Security feature tests
    integration        Integration and functionality tests

EXAMPLES:
    $0                          # Run full validation suite
    $0 -m quick                 # Quick validation
    $0 -v -o report.json        # Verbose with report output
    $0 -m hardware              # Hardware tests only

EOF
}

parse_test_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                TEST_VERBOSE=true
                shift
                ;;
            -m|--mode)
                TEST_MODE="$2"
                shift 2
                ;;
            -o|--output)
                VALIDATION_RESULTS_FILE="$2"
                shift 2
                ;;
            *)
                echo "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

run_test_suite() {
    log_test "INFO" "Starting LimitlessOS installer validation"
    log_test "INFO" "Test mode: $TEST_MODE"
    
    case "$TEST_MODE" in
        "full"|"")
            run_test "System Requirements" "test_system_requirements" "Validate minimum system requirements"
            run_test "Hardware Detection" "test_hardware_detection" "Test hardware detection functionality"
            run_test "Virtualization Support" "test_virtualization_support" "Check virtualization support"
            run_test "Installer Scripts" "test_installer_scripts_exist" "Verify installer scripts exist"
            run_test "Script Syntax" "test_installer_script_syntax" "Validate script syntax"
            run_test "Configuration Files" "test_configuration_files" "Test configuration handling"
            run_test "GUI Components" "test_gui_components" "Verify GUI components"
            run_test "GUI Compilation" "test_gui_compilation" "Test GUI compilation"
            run_test "Security Features" "test_security_features" "Validate security features"
            run_test "Encryption Capability" "test_encryption_capability" "Test encryption capability"
            run_test "Disk Performance" "test_disk_performance" "Test disk I/O performance"
            run_test "Network Connectivity" "test_network_connectivity" "Test network connectivity"
            run_test "Installer Integration" "test_installer_integration" "Test installer integration"
            ;;
        "quick")
            run_test "System Requirements" "test_system_requirements" "Validate minimum system requirements"
            run_test "Installer Scripts" "test_installer_scripts_exist" "Verify installer scripts exist"
            run_test "Script Syntax" "test_installer_script_syntax" "Validate script syntax"
            run_test "Security Features" "test_security_features" "Validate security features"
            ;;
        "hardware")
            run_test "System Requirements" "test_system_requirements" "Validate minimum system requirements"
            run_test "Hardware Detection" "test_hardware_detection" "Test hardware detection functionality"
            run_test "Virtualization Support" "test_virtualization_support" "Check virtualization support"
            run_test "Disk Performance" "test_disk_performance" "Test disk I/O performance"
            ;;
        "security")
            run_test "Security Features" "test_security_features" "Validate security features"
            run_test "Encryption Capability" "test_encryption_capability" "Test encryption capability"
            ;;
        "integration")
            run_test "Installer Scripts" "test_installer_scripts_exist" "Verify installer scripts exist"
            run_test "Script Syntax" "test_installer_script_syntax" "Validate script syntax"
            run_test "Configuration Files" "test_configuration_files" "Test configuration handling"
            run_test "Installer Integration" "test_installer_integration" "Test installer integration"
            ;;
        *)
            log_test "FAIL" "Unknown test mode: $TEST_MODE"
            return 1
            ;;
    esac
}

show_test_summary() {
    echo ""
    echo "=============================================================================="
    echo "                           Validation Summary"
    echo "=============================================================================="
    echo -e "${BLUE}Total Tests:${NC} $test_count"
    echo -e "${GREEN}Passed:${NC} $pass_count"
    echo -e "${RED}Failed:${NC} $fail_count"
    echo -e "${YELLOW}Warnings:${NC} $warn_count"
    
    local success_rate=$(echo "scale=1; $pass_count * 100 / $test_count" | bc -l 2>/dev/null || echo "0")
    echo -e "${BLUE}Success Rate:${NC} ${success_rate}%"
    
    if [[ $fail_count -eq 0 ]]; then
        echo -e "\n${GREEN}✓ All tests passed! LimitlessOS installer is ready for deployment.${NC}"
    else
        echo -e "\n${RED}✗ Some tests failed. Please review the issues above.${NC}"
    fi
    
    echo -e "\nDetailed log: $TEST_LOG_FILE"
    echo -e "Validation report: $VALIDATION_RESULTS_FILE"
}

main() {
    # Initialize logging
    echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] LimitlessOS Installer Validation Started" > "$TEST_LOG_FILE"
    
    # Parse arguments
    parse_test_arguments "$@"
    
    # Show banner
    show_test_banner
    
    # Run tests
    if run_test_suite; then
        generate_validation_report "$VALIDATION_RESULTS_FILE"
        show_test_summary
        
        if [[ $fail_count -eq 0 ]]; then
            exit 0
        else
            exit 1
        fi
    else
        log_test "FAIL" "Test suite execution failed"
        exit 1
    fi
}

# Execute main function with all arguments
main "$@"