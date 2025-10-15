#!/bin/bash
#
# Boot Test Script
# 
# Tests kernel boot sequence and initialization
#

set -e

echo "=================================="
echo "  LimitlessOS Boot Test          "
echo "=================================="
echo ""

# Check if kernel binary exists
if [ ! -f "build/limitless.elf" ]; then
    echo "❌ Kernel binary not found: build/limitless.elf"
    echo "   Run 'make kernel' first"
    exit 1
fi

echo "✅ Kernel binary found"

# Check kernel format
if ! file build/limitless.elf | grep -q "ELF 32-bit"; then
    echo "❌ Kernel is not a valid 32-bit ELF file"
    file build/limitless.elf
    exit 1
fi

echo "✅ Kernel format valid (32-bit ELF)"

# Check for required symbols
echo ""
echo "Checking required symbols..."

SYMBOLS=(
    "multiboot2_start"
    "kmain"
    "system_init"
    "pmm_init"
    "vmm_init"
    "vfs_init"
    "device_init"
)

for sym in "${SYMBOLS[@]}"; do
    if nm build/limitless.elf | grep -q " $sym"; then
        echo "  ✅ $sym"
    else
        echo "  ❌ $sym (not found)"
        exit 1
    fi
done

# Check kernel size
KERNEL_SIZE=$(stat -c%s build/limitless.elf 2>/dev/null || stat -f%z build/limitless.elf 2>/dev/null)
echo ""
echo "Kernel size: $(($KERNEL_SIZE / 1024)) KB"

if [ $KERNEL_SIZE -lt 10000 ]; then
    echo "⚠️  Warning: Kernel seems too small"
elif [ $KERNEL_SIZE -gt 10000000 ]; then
    echo "⚠️  Warning: Kernel seems too large"
else
    echo "✅ Kernel size reasonable"
fi

# Check for integration test symbols
echo ""
echo "Checking integration test components..."

TEST_SYMBOLS=(
    "run_integration_tests"
    "run_smoke_test"
    "system_selftest"
    "system_health_check"
)

for sym in "${TEST_SYMBOLS[@]}"; do
    if nm build/limitless.elf | grep -q " $sym"; then
        echo "  ✅ $sym"
    else
        echo "  ⚠️  $sym (not found - optional)"
    fi
done

# Check for device driver symbols
echo ""
echo "Checking device drivers..."

DRIVER_SYMBOLS=(
    "serial_driver_init"
    "keyboard_driver_init"
    "device_register"
    "device_find_by_name"
)

for sym in "${DRIVER_SYMBOLS[@]}"; do
    if nm build/limitless.elf | grep -q " $sym"; then
        echo "  ✅ $sym"
    else
        echo "  ❌ $sym (not found)"
        exit 1
    fi
done

echo ""
echo "=================================="
echo "✅ All boot tests passed!"
echo "=================================="
echo ""
echo "Kernel is ready to boot."
echo "Run 'make test-qemu' to test in QEMU"
echo ""

exit 0
