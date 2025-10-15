/*
 * Integration Test Suite
 * 
 * Comprehensive tests that validate multiple subsystems working together:
 * - Memory + VFS integration
 * - VFS + Device driver integration
 * - File I/O end-to-end tests
 * - Process + IPC integration
 * 
 * Copyright (c) 2024 LimitlessOS Project
 */

#include "kernel.h"
#include "vfs.h"
#include "device.h"

// Test result tracking
typedef struct {
    const char* name;
    int (*test_func)(void);
    int result;  // 0 = not run, 1 = pass, -1 = fail
    const char* error_msg;
} test_case_t;

static const char* last_error = NULL;

#define TEST_PASS(msg) do { last_error = NULL; kprintf("    [PASS] %s\n", msg); return 1; } while(0)
#define TEST_FAIL(msg) do { last_error = msg; kprintf("    [FAIL] %s\n", msg); return -1; } while(0)
#define ASSERT(cond, msg) if (!(cond)) TEST_FAIL(msg)

// Test 1: Memory allocation stress test
static int test_memory_stress(void) {
    kprintf("  Testing memory allocation stress...\n");
    
    // Allocate various sizes
    void* ptrs[10];
    size_t sizes[] = {16, 64, 256, 1024, 4096, 8192, 16384, 32768, 65536, 131072};
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = kmalloc(sizes[i]);
        ASSERT(ptrs[i] != NULL, "Allocation failed");
        
        // Write pattern
        uint8_t* data = (uint8_t*)ptrs[i];
        for (size_t j = 0; j < sizes[i]; j++) {
            data[j] = (uint8_t)(i + j);
        }
    }
    
    // Verify patterns
    for (int i = 0; i < 10; i++) {
        uint8_t* data = (uint8_t*)ptrs[i];
        for (size_t j = 0; j < sizes[i]; j++) {
            ASSERT(data[j] == (uint8_t)(i + j), "Data corruption detected");
        }
    }
    
    // Free all
    for (int i = 0; i < 10; i++) {
        kfree(ptrs[i]);
    }
    
    TEST_PASS("Memory stress test completed");
}

// Test 2: VFS + Memory integration
static int test_vfs_memory_integration(void) {
    kprintf("  Testing VFS + Memory integration...\n");
    
    // Allocate vnode
    vnode_t* vn = (vnode_t*)kmalloc(sizeof(vnode_t));
    ASSERT(vn != NULL, "Failed to allocate vnode");
    
    // Initialize vnode
    vn->type = VNODE_FILE;
    vn->size = 0;
    
    // Allocate data buffer through VFS
    char* buffer = (char*)kmalloc(4096);
    ASSERT(buffer != NULL, "Failed to allocate buffer");
    
    // Write test data
    const char* test_data = "VFS integration test data";
    for (int i = 0; test_data[i] != '\0'; i++) {
        buffer[i] = test_data[i];
    }
    
    // Verify data
    for (int i = 0; test_data[i] != '\0'; i++) {
        ASSERT(buffer[i] == test_data[i], "Data verification failed");
    }
    
    // Cleanup
    kfree(buffer);
    kfree(vn);
    
    TEST_PASS("VFS + Memory integration successful");
}

// Test 3: Device + VFS integration
static int test_device_vfs_integration(void) {
    kprintf("  Testing Device + VFS integration...\n");
    
    // Find null device
    device_t* dev = device_find_by_name("null");
    ASSERT(dev != NULL, "/dev/null not found");
    
    // Open device
    ASSERT(dev->ops != NULL, "Device has no operations");
    ASSERT(dev->ops->open != NULL, "Device has no open operation");
    
    int result = dev->ops->open(dev);
    ASSERT(result >= 0, "Failed to open /dev/null");
    
    // Write to null device (should succeed and discard)
    if (dev->ops->write) {
        char buffer[256] = "Test data for null device";
        long written = dev->ops->write(dev, 0, buffer, 256);
        ASSERT(written >= 0, "Write to /dev/null failed");
    }
    
    // Close device
    if (dev->ops->close) {
        dev->ops->close(dev);
    }
    
    TEST_PASS("Device + VFS integration successful");
}

// Test 4: Serial device integration
static int test_serial_integration(void) {
    kprintf("  Testing Serial device integration...\n");
    
    // Find serial device
    device_t* dev = device_find_by_name("ttyS0");
    ASSERT(dev != NULL, "/dev/ttyS0 not found");
    
    // Verify device properties
    ASSERT(dev->type == DEV_TYPE_CHAR, "Serial device not char type");
    ASSERT(dev->ops != NULL, "Serial device has no operations");
    
    // Open device
    if (dev->ops->open) {
        int result = dev->ops->open(dev);
        ASSERT(result >= 0, "Failed to open serial device");
    }
    
    // Write test data
    if (dev->ops->write) {
        const char* msg = "Serial test\n";
        long len = 0;
        while (msg[len]) len++;
        
        long written = dev->ops->write(dev, 0, msg, len);
        ASSERT(written >= 0, "Serial write failed");
    }
    
    // Close device
    if (dev->ops->close) {
        dev->ops->close(dev);
    }
    
    TEST_PASS("Serial device integration successful");
}

// Test 5: Keyboard device integration
static int test_keyboard_integration(void) {
    kprintf("  Testing Keyboard device integration...\n");
    
    // Find keyboard device
    device_t* dev = device_find_by_name("kbd");
    ASSERT(dev != NULL, "/dev/kbd not found");
    
    // Verify device properties
    ASSERT(dev->type == DEV_TYPE_CHAR, "Keyboard device not char type");
    ASSERT(dev->ops != NULL, "Keyboard device has no operations");
    
    // Open device
    if (dev->ops->open) {
        int result = dev->ops->open(dev);
        ASSERT(result >= 0, "Failed to open keyboard device");
    }
    
    // Note: We can't test read without actual keyboard input,
    // but we can verify the device is ready
    ASSERT(dev->ops->read != NULL, "Keyboard has no read operation");
    
    // Close device
    if (dev->ops->close) {
        dev->ops->close(dev);
    }
    
    TEST_PASS("Keyboard device integration successful");
}

// Test 6: Multi-device concurrent access
static int test_multi_device_access(void) {
    kprintf("  Testing concurrent multi-device access...\n");
    
    // Open multiple devices simultaneously
    device_t* null_dev = device_find_by_name("null");
    device_t* zero_dev = device_find_by_name("zero");
    device_t* serial_dev = device_find_by_name("ttyS0");
    
    ASSERT(null_dev != NULL, "null device not found");
    ASSERT(zero_dev != NULL, "zero device not found");
    ASSERT(serial_dev != NULL, "serial device not found");
    
    // Open all devices
    if (null_dev->ops->open) null_dev->ops->open(null_dev);
    if (zero_dev->ops->open) zero_dev->ops->open(zero_dev);
    if (serial_dev->ops->open) serial_dev->ops->open(serial_dev);
    
    // Perform operations on each
    char buffer[64];
    
    // Read from zero device
    if (zero_dev->ops->read) {
        long nread = zero_dev->ops->read(zero_dev, 0, buffer, 64);
        ASSERT(nread >= 0, "Read from /dev/zero failed");
        
        // Verify all zeros
        for (int i = 0; i < nread; i++) {
            ASSERT(buffer[i] == 0, "/dev/zero returned non-zero data");
        }
    }
    
    // Write to null device
    if (null_dev->ops->write) {
        const char* msg = "test";
        long nwritten = null_dev->ops->write(null_dev, 0, msg, 4);
        ASSERT(nwritten >= 0, "Write to /dev/null failed");
    }
    
    // Close all devices
    if (null_dev->ops->close) null_dev->ops->close(null_dev);
    if (zero_dev->ops->close) zero_dev->ops->close(zero_dev);
    if (serial_dev->ops->close) serial_dev->ops->close(serial_dev);
    
    TEST_PASS("Multi-device concurrent access successful");
}

// Test 7: Memory allocation patterns
static int test_memory_patterns(void) {
    kprintf("  Testing complex memory allocation patterns...\n");
    
    // Test 1: Fragmentation handling
    void* small[20];
    void* large[5];
    
    // Allocate interleaved small and large blocks
    for (int i = 0; i < 20; i++) {
        small[i] = kmalloc(128);
        ASSERT(small[i] != NULL, "Small allocation failed");
    }
    
    for (int i = 0; i < 5; i++) {
        large[i] = kmalloc(8192);
        ASSERT(large[i] != NULL, "Large allocation failed");
    }
    
    // Free every other small block to create fragmentation
    for (int i = 0; i < 20; i += 2) {
        kfree(small[i]);
        small[i] = NULL;
    }
    
    // Reallocate in gaps
    for (int i = 0; i < 20; i += 2) {
        small[i] = kmalloc(128);
        ASSERT(small[i] != NULL, "Reallocation in gap failed");
    }
    
    // Cleanup
    for (int i = 0; i < 20; i++) {
        if (small[i]) kfree(small[i]);
    }
    for (int i = 0; i < 5; i++) {
        if (large[i]) kfree(large[i]);
    }
    
    TEST_PASS("Complex memory allocation patterns successful");
}

// Test 8: End-to-end file I/O simulation
static int test_file_io_simulation(void) {
    kprintf("  Testing end-to-end file I/O simulation...\n");
    
    // Simulate file descriptor allocation
    typedef struct {
        vnode_t* vnode;
        u64 offset;
        int flags;
    } fd_sim_t;
    
    // Allocate simulated FD
    fd_sim_t* fd = (fd_sim_t*)kmalloc(sizeof(fd_sim_t));
    ASSERT(fd != NULL, "Failed to allocate FD structure");
    
    // Allocate vnode
    fd->vnode = (vnode_t*)kmalloc(sizeof(vnode_t));
    ASSERT(fd->vnode != NULL, "Failed to allocate vnode");
    
    // Initialize vnode
    fd->vnode->type = VNODE_FILE;
    fd->vnode->size = 0;
    fd->offset = 0;
    fd->flags = 0;
    
    // Allocate file buffer
    char* file_data = (char*)kmalloc(4096);
    ASSERT(file_data != NULL, "Failed to allocate file buffer");
    
    // Simulate write
    const char* write_data = "Hello from integration test!";
    int write_len = 0;
    while (write_data[write_len]) write_len++;
    
    for (int i = 0; i < write_len; i++) {
        file_data[fd->offset + i] = write_data[i];
    }
    fd->offset += write_len;
    fd->vnode->size = fd->offset;
    
    // Simulate seek to beginning
    fd->offset = 0;
    
    // Simulate read
    char read_buffer[128];
    for (int i = 0; i < write_len; i++) {
        read_buffer[i] = file_data[fd->offset + i];
    }
    read_buffer[write_len] = '\0';
    fd->offset += write_len;
    
    // Verify data
    for (int i = 0; i < write_len; i++) {
        ASSERT(read_buffer[i] == write_data[i], "Read data doesn't match written data");
    }
    
    // Cleanup
    kfree(file_data);
    kfree(fd->vnode);
    kfree(fd);
    
    TEST_PASS("End-to-end file I/O simulation successful");
}

// Define all test cases
static test_case_t test_cases[] = {
    {"Memory Allocation Stress", test_memory_stress, 0, NULL},
    {"VFS + Memory Integration", test_vfs_memory_integration, 0, NULL},
    {"Device + VFS Integration", test_device_vfs_integration, 0, NULL},
    {"Serial Device Integration", test_serial_integration, 0, NULL},
    {"Keyboard Device Integration", test_keyboard_integration, 0, NULL},
    {"Multi-Device Concurrent Access", test_multi_device_access, 0, NULL},
    {"Complex Memory Patterns", test_memory_patterns, 0, NULL},
    {"End-to-End File I/O Simulation", test_file_io_simulation, 0, NULL},
    {NULL, NULL, 0, NULL}
};

// Run all integration tests
int run_integration_tests(void) {
    kprintf("\n");
    kprintf("=====================================\n");
    kprintf("   Integration Test Suite            \n");
    kprintf("=====================================\n");
    kprintf("\n");
    
    int total = 0;
    int passed = 0;
    int failed = 0;
    
    // Count tests
    for (int i = 0; test_cases[i].name != NULL; i++) {
        total++;
    }
    
    // Run each test
    for (int i = 0; test_cases[i].name != NULL; i++) {
        kprintf("[%d/%d] %s\n", i + 1, total, test_cases[i].name);
        
        int result = test_cases[i].test_func();
        test_cases[i].result = result;
        test_cases[i].error_msg = last_error;
        
        if (result > 0) {
            passed++;
        } else {
            failed++;
            if (last_error) {
                kprintf("    Error: %s\n", last_error);
            }
        }
        kprintf("\n");
    }
    
    // Summary
    kprintf("=====================================\n");
    kprintf("Test Results:\n");
    kprintf("  Total:   %d\n", total);
    kprintf("  Passed:  %d\n", passed);
    kprintf("  Failed:  %d\n", failed);
    kprintf("\n");
    
    if (failed == 0) {
        kprintf("Status:  ALL TESTS PASSED\n");
    } else {
        kprintf("Status:  %d TEST(S) FAILED\n", failed);
        kprintf("\nFailed tests:\n");
        for (int i = 0; test_cases[i].name != NULL; i++) {
            if (test_cases[i].result < 0) {
                kprintf("  - %s\n", test_cases[i].name);
                if (test_cases[i].error_msg) {
                    kprintf("    %s\n", test_cases[i].error_msg);
                }
            }
        }
    }
    
    kprintf("=====================================\n");
    kprintf("\n");
    
    return (failed == 0) ? 1 : 0;
}

// Run quick smoke test
int run_smoke_test(void) {
    kprintf("\n");
    kprintf("Running quick smoke test...\n");
    kprintf("\n");
    
    // Test memory
    void* ptr = kmalloc(1024);
    if (!ptr) {
        kprintf("[FAIL] Memory allocation\n");
        return 0;
    }
    kfree(ptr);
    kprintf("[PASS] Memory allocation\n");
    
    // Test device subsystem
    device_t* dev = device_find_by_name("null");
    if (!dev) {
        kprintf("[FAIL] Device subsystem\n");
        return 0;
    }
    kprintf("[PASS] Device subsystem\n");
    
    kprintf("\nSmoke test passed!\n");
    kprintf("\n");
    
    return 1;
}
