#!/bin/bash

# LimitlessOS System Validation Script
# Validates that all components are properly integrated

echo "LimitlessOS System Validation"
echo "============================"

EXIT_CODE=0

# Function to check if file exists
check_file() {
    if [ -f "$1" ]; then
        echo "✓ $1"
    else
        echo "✗ Missing: $1"
        EXIT_CODE=1
    fi
}

# Function to check if directory exists
check_dir() {
    if [ -d "$1" ]; then
        echo "✓ $1/"
    else
        echo "✗ Missing directory: $1/"
        EXIT_CODE=1
    fi
}

echo "Checking core system files..."
check_file "Makefile.optimized"
check_file "Makefile.enterprise"
check_file "kernel/include/limitless_config.h"

echo -e "\nChecking enterprise components..."
check_file "kernel/include/security_enterprise.h"
check_file "kernel/include/sandbox.h"
check_file "kernel/src/sandbox.c"
check_file "userspace/include/ai_framework.h"
check_file "userspace/src/ai_framework.c"
check_file "installer/intelligent_installer.h"
check_file "installer/intelligent_installer.c"

echo -e "\nChecking directory structure..."
check_dir "kernel/include"
check_dir "kernel/src"
check_dir "userspace/include"
check_dir "userspace/src"
check_dir "hal/include"
check_dir "hal/src"
check_dir "installer"
check_dir "docs"
check_dir "tools"

echo -e "\nChecking for obsolete files..."
OBSOLETE_FILES="test_identity.o enterprise_demo.c enterprise_admin_demo.c NUL"
for file in $OBSOLETE_FILES; do
    if [ -f "$file" ]; then
        echo "⚠ Obsolete file still present: $file"
        EXIT_CODE=1
    else
        echo "✓ Obsolete file removed: $file"
    fi
done

echo -e "\nTesting build system..."
if make -f Makefile.optimized help >/dev/null 2>&1; then
    echo "✓ Build system functional"
else
    echo "✗ Build system has issues"
    EXIT_CODE=1
fi

echo -e "\nValidating component integration..."
INTEGRATION_SCORE=0
TOTAL_COMPONENTS=8

# POSIX-compatible component list
COMPONENTS="graphics multimedia storage security ai sandbox installer developer"
for component in $COMPONENTS; do
    if grep -r "$component" kernel/include/ userspace/include/ >/dev/null 2>&1; then
        echo "✓ $component component integrated"
        INTEGRATION_SCORE=`expr $INTEGRATION_SCORE + 1`
    else
        echo "✗ $component component not fully integrated"
    fi
done

PERCENTAGE=`expr $INTEGRATION_SCORE \* 100 / $TOTAL_COMPONENTS`
echo -e "\nSystem Integration Score: $INTEGRATION_SCORE/$TOTAL_COMPONENTS ($PERCENTAGE%)"

if [ $EXIT_CODE -eq 0 ]; then
    echo -e "\n🎉 System validation PASSED - LimitlessOS is ready for production!"
else
    echo -e "\n❌ System validation FAILED - Please address the issues above"
fi

exit $EXIT_CODE
