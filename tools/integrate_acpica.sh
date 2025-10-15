#!/bin/bash
# ACPICA integration script for LimitlessOS kernel
# Usage: ./integrate_acpica.sh <kernel_root>

set -e


# Kernel root and ACPICA directory
KERNEL_ROOT="${1:-.}"
ACPI_DIR="$KERNEL_ROOT/kernel/acpica"
# Use latest working ACPICA release from GitHub
ACPICA_URL="https://github.com/acpica/acpica/releases/download/20250807/acpica-unix-20250807.tar.gz"
ACPICA_ARCHIVE="acpica.tar.gz"

# Create acpica directory
mkdir -p "$ACPI_DIR"


# Download ACPICA source with error handling
echo "Downloading ACPICA from $ACPICA_URL ..."
curl -L --fail "$ACPICA_URL" -o "$ACPI_DIR/$ACPICA_ARCHIVE"
if [ $? -ne 0 ]; then
    echo "Error: Failed to download ACPICA archive. Check URL or network connectivity."
    exit 1
fi

# Validate file type before extraction
file "$ACPI_DIR/$ACPICA_ARCHIVE" | grep 'gzip compressed data' > /dev/null
if [ $? -ne 0 ]; then
    echo "Error: Downloaded file is not a valid gzip archive."
    exit 2
fi

# Extract ACPICA source
cd "$ACPI_DIR"
tar -xzf "$ACPICA_ARCHIVE" --strip-components=1
if [ $? -ne 0 ]; then
    echo "Error: Failed to extract ACPICA archive."
    exit 3
fi
rm "$ACPICA_ARCHIVE"

# Ensure target directory exists
mkdir -p "$ACPI_DIR/source/components"
# Copy core ACPICA components to kernel/acpica/source/components
cp -r source/components/* "$ACPI_DIR/source/components/"

# Create Makefile stub for ACPICA integration
cat << 'EOF' > "$ACPI_DIR/Makefile"
# ACPICA kernel integration Makefile
ACPI_SRC = $(wildcard dispatcher/*.c executer/*.c namespace/*.c tables/*.c utilities/*.c)
ACPI_INC = -I. -Iinclude

acpica.o: $(ACPI_SRC)
	$(CC) $(CFLAGS) $(ACPI_INC) -c $^ -o $@
EOF

# Print next steps
cat << 'EOM'
ACPICA source downloaded and staged in kernel/acpica.

Next steps:
1. Add acpica.o to your kernel build system (Makefile or CMakeLists.txt).
2. In kernel/acpi.c, add:
   #include "acpi.h"
   #include "acpica/include/acpi.h"
   void acpi_init_acpica(void) {
       AcpiInitializeSubsystem();
       AcpiInitializeTables(NULL, 16, TRUE);
       AcpiLoadTables();
       AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
       AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
   }
3. Refactor ACPI table parsing and method execution to use ACPICA APIs.
4. Test on real hardware and VMs.
EOM
