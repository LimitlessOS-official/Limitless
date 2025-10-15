# LimitlessOS Driver Management System Makefile
# Builds the complete driver management system including GUI, workflows, and APIs
# Copyright (c) LimitlessOS Project

CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c11 -fPIC -pthread
LDFLAGS = -shared -pthread

# LimitlessOS native library paths (production would link against LimitlessOS libs)
LIMITLESS_INCLUDES = -I../../hal/include -I../../kernel/include
LIMITLESS_LIBS = -llimitless_kernel -llimitless_threading -llimitless_memory -llimitless_gui

# Source files
EVENT_SOURCES = driver_event_bus.c driver_event_cleanup.c
LOADER_SOURCES = ../../hal/src/limitless_driver_loader.c
PANEL_SOURCES = driver_management_panel.c vendor_workflow.c
ALL_SOURCES = $(EVENT_SOURCES) $(PANEL_SOURCES)
ALL_OBJECTS = $(ALL_SOURCES:.c=.o)

# Targets
EVENT_TARGET = liblimitless_event_bus.so
PANEL_TARGET = limitless_driver_panel
WORKFLOW_TARGET = limitless_vendor_workflow

# Build all targets
all: $(EVENT_TARGET) $(PANEL_TARGET) $(WORKFLOW_TARGET)

# Build event bus shared library
$(EVENT_TARGET): $(EVENT_SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIMITLESS_LIBS)

# Build driver management panel
$(PANEL_TARGET): driver_management_panel.o $(EVENT_SOURCES:.c=.o)
	$(CC) -pthread -o $@ $^ $(LIMITLESS_LIBS)

# Build vendor workflow tool
$(WORKFLOW_TARGET): vendor_workflow.o
	$(CC) -pthread -o $@ $^ $(LIMITLESS_LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(LIMITLESS_INCLUDES) -c $< -o $@

# Compile HAL sources
../../hal/src/%.o: ../../hal/src/%.c
	$(CC) $(CFLAGS) $(LIMITLESS_INCLUDES) -c $< -o $@

# Installation target (integrates with LimitlessOS package manager)
install: all
	mkdir -p /usr/lib/limitless /usr/bin/limitless /usr/include/limitless
	cp $(EVENT_TARGET) /usr/lib/limitless/
	cp $(PANEL_TARGET) $(WORKFLOW_TARGET) /usr/bin/limitless/
	cp ../../hal/include/limitless_driver_api.h /usr/include/limitless/
	cp ../../hal/include/limitless_driver_loader.h /usr/include/limitless/

# Clean build artifacts
clean:
	rm -f *.o ../../hal/src/*.o $(EVENT_TARGET) $(PANEL_TARGET) $(WORKFLOW_TARGET)

# Development test build (links against standard libraries for testing)
test: LIMITLESS_LIBS = -lpthread
test: all

# Run integration tests
test-run: test
	./$(PANEL_TARGET) &
	./$(WORKFLOW_TARGET) "Test Driver" "Test Vendor"

.PHONY: all install clean test test-run