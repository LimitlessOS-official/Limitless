# LimitlessOS Architecture Implementation Makefile
# Builds all core architecture modules for production deployment
# Copyright (c) LimitlessOS Project

CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c11 -fPIC -pthread -fstack-protector-strong
KERNEL_CFLAGS = $(CFLAGS) -nostdlib -fno-builtin -mcmodel=kernel
LDFLAGS = -shared -pthread

KERNEL_INCLUDE = -I../include -I../../hal/include -I../../acpica/source/include
KERNEL_LIBS = -llimitless_hal -llimitless_crypto -llimitless_memory


# ACPICA sources (full parity)
ACPICA_SOURCES = \
	../../acpica/source/components/dispatcher/*.c \
	../../acpica/source/components/executer/*.c \
	../../acpica/source/components/namespace/*.c \
	../../acpica/source/components/tables/*.c \
	../../acpica/source/components/utilities/*.c \
	../../acpica/source/components/parser/*.c \
	../../acpica/source/components/resources/*.c \
	../../acpica/source/components/events/*.c \
	../../acpica/source/components/hardware/*.c \
	../../acpica/source/components/debugger/*.c \
	../../acpica/source/components/disassembler/*.c

MICROKERNEL_SOURCES = microkernel_core.c
MEMORY_SOURCES = memory_management.c
SECURITY_SOURCES = security_framework.c
NETWORK_SOURCES = network_stack.c
STORAGE_SOURCES = storage_architecture.c

ALL_SOURCES = $(MICROKERNEL_SOURCES) $(MEMORY_SOURCES) $(SECURITY_SOURCES) $(NETWORK_SOURCES) $(STORAGE_SOURCES) $(ACPICA_SOURCES)
ALL_OBJECTS = $(ALL_SOURCES:.c=.o)


# Build targets
MICROKERNEL_TARGET = limitless_microkernel.ko
MEMORY_TARGET = limitless_memory.ko
SECURITY_TARGET = limitless_security.ko
NETWORK_TARGET = limitless_network.ko
STORAGE_TARGET = limitless_storage.ko
ACPICA_TARGET = acpica.o

# Build all kernel modules
all: $(MICROKERNEL_TARGET) $(MEMORY_TARGET) $(SECURITY_TARGET) $(NETWORK_TARGET) $(STORAGE_TARGET) $(ACPICA_TARGET)
# ACPICA object (full parity)
$(ACPICA_TARGET): $(ACPICA_SOURCES)
	$(CC) $(KERNEL_CFLAGS) $(KERNEL_INCLUDE) -c $^ -o $@

# Microkernel core module
$(MICROKERNEL_TARGET): microkernel_core.o
	$(CC) $(KERNEL_CFLAGS) -o $@ $^ $(KERNEL_LIBS)

# Memory management module
$(MEMORY_TARGET): memory_management.o
	$(CC) $(KERNEL_CFLAGS) -o $@ $^ $(KERNEL_LIBS)

# Security framework module  
$(SECURITY_TARGET): security_framework.o
	$(CC) $(KERNEL_CFLAGS) -o $@ $^ $(KERNEL_LIBS) -llimitless_tpm -llimitless_pqcrypto

# Network stack module
$(NETWORK_TARGET): network_stack.o
	$(CC) $(KERNEL_CFLAGS) -o $@ $^ $(KERNEL_LIBS) -llimitless_dpdk -llimitless_rdma

# Storage architecture module
$(STORAGE_TARGET): storage_architecture.o
	$(CC) $(KERNEL_CFLAGS) -o $@ $^ $(KERNEL_LIBS) -llimitless_nvme -llimitless_raid

# Compile source files with kernel-specific flags
%.o: %.c
	$(CC) $(KERNEL_CFLAGS) $(KERNEL_INCLUDE) -c $< -o $@

# Installation target for kernel modules
install: all
	mkdir -p /boot/limitless/modules/kernel
	cp *.ko /boot/limitless/modules/kernel/
	mkdir -p /usr/include/limitless/kernel
	cp ../include/*.h /usr/include/limitless/kernel/

# Create kernel headers package
headers:
	mkdir -p kernel-headers/limitless
	cp ../include/*.h kernel-headers/limitless/
	tar czf limitless-kernel-headers.tar.gz kernel-headers/

# Development build with debugging symbols
debug: KERNEL_CFLAGS += -g -DDEBUG -fno-omit-frame-pointer
debug: all

# Performance optimized build
performance: KERNEL_CFLAGS += -O3 -march=native -flto -fomit-frame-pointer
performance: all

# Security hardened build
security: KERNEL_CFLAGS += -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE
security: all

# Clean build artifacts
clean:
	rm -f *.o *.ko limitless-kernel-headers.tar.gz
	rm -rf kernel-headers/

# Run architecture validation tests
test: all
	./test_microkernel.sh
	./test_memory.sh
	./test_security.sh
	./test_network.sh
	./test_storage.sh

# Generate documentation
docs:
	doxygen Doxyfile
	sphinx-build -b html docs/ docs/_build/

# Static analysis
analyze:
	clang-static-analyzer $(ALL_SOURCES)
	cppcheck --enable=all $(ALL_SOURCES)

# Code coverage analysis
coverage: KERNEL_CFLAGS += --coverage
coverage: all
	gcov $(ALL_SOURCES)
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage-html

.PHONY: all install headers debug performance security clean test docs analyze coverage