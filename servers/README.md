# LimitlessOS Userspace Servers

## Overview

LimitlessOS implements a hybrid kernel architecture where most device drivers and system services run in userspace as isolated server processes. This provides:

- **Fault Isolation**: Driver crashes don't crash the kernel
- **Security**: Servers run with minimal privileges
- **Modularity**: Services can be updated independently
- **Debugging**: Easier to debug userspace code
- **Safety**: Memory corruption in drivers is contained

## Server Types

### 1. File System Servers (`servers/fs/`)

File system implementations run in userspace and communicate with the kernel VFS layer via IPC.

#### VFS Server (`vfs_server.c`)
- Implements Virtual File System operations
- Handles file open, read, write, close
- Manages inode cache
- Coordinates with underlying file systems

**Features**:
- Path resolution
- Permission checking
- Directory operations
- File descriptor management
- Page cache integration

**IPC Messages**:
- `MSG_VFS_OPEN`: Open file
- `MSG_VFS_READ`: Read from file
- `MSG_VFS_WRITE`: Write to file
- `MSG_VFS_CLOSE`: Close file
- `MSG_VFS_STAT`: Get file status
- `MSG_VFS_MKDIR`: Create directory
- `MSG_VFS_RMDIR`: Remove directory
- `MSG_VFS_UNLINK`: Delete file

### 2. Device Driver Servers (`servers/drivers/`)

Hardware device drivers run in userspace with capabilities to access device resources.

#### USB Server (`usb_server.c`)
- USB host controller driver (UHCI, OHCI, EHCI, XHCI)
- Device enumeration and configuration
- Transfer management
- Hot-plug support

**Features**:
- USB 1.x, 2.0, and 3.x support
- Control, bulk, interrupt, and isochronous transfers
- Device class driver loading
- Power management
- Error recovery

**Capabilities Needed**:
- MMIO access to USB controller registers
- DMA buffer allocation
- Interrupt forwarding
- PCI configuration access

#### Network Driver Server (Planned)
- Network interface card drivers
- Packet transmission and reception
- Link state management
- Statistics collection

#### Storage Driver Server (Planned)
- SATA/NVMe controller drivers
- Block device operations
- Command queuing
- Error handling

#### Graphics Driver Server (Planned)
- GPU initialization and mode-setting
- Display buffer management
- Acceleration commands
- Power management

### 3. System Service Daemons (`servers/services/`)

System-level services that don't require kernel privileges.

#### Power Management Daemon (`power_mgmt_daemon.c`)
- Battery monitoring and reporting
- Thermal zone management
- CPU frequency scaling
- Power state transitions
- ACPI event handling

**Features**:
- Real-time battery status
- Thermal throttling
- CPU governor policies
- Sleep/suspend/hibernate
- Wake events
- Critical shutdown

**Power States**:
- `POWER_STATE_ON`: Full power
- `POWER_STATE_IDLE`: Idle, low power
- `POWER_STATE_STANDBY`: Quick wake
- `POWER_STATE_SUSPEND`: RAM powered, fast resume
- `POWER_STATE_HIBERNATE`: Disk hibernation
- `POWER_STATE_POWEROFF`: Complete shutdown

#### Display Server (Planned)
- Wayland compositor
- Window management
- Input handling
- Application rendering

#### Audio Server (Planned)
- Audio mixing and routing
- Device management
- Low-latency playback
- Recording support

## Server Communication

### IPC Message Format

```c
typedef struct {
    uint32_t type;          /* Message type */
    uint32_t sender_pid;    /* Sender process ID */
    uint32_t flags;         /* Message flags */
    uint32_t param1;        /* Parameter 1 */
    uint32_t param2;        /* Parameter 2 */
    uint32_t param3;        /* Parameter 3 */
    void *data_ptr;         /* Pointer to data */
    size_t data_size;       /* Size of data */
} ipc_message_t;
```

### Message Passing

Servers use synchronous and asynchronous IPC to communicate:

1. **Synchronous IPC**: Client sends message and blocks waiting for reply
   ```c
   result = ipc_send_receive(server_pid, &msg, &reply);
   ```

2. **Asynchronous IPC**: Client sends message and continues
   ```c
   result = ipc_send(server_pid, &msg);
   /* Later... */
   result = ipc_receive(&reply);
   ```

3. **Zero-Copy Transfer**: Large data uses shared memory
   ```c
   /* Share buffer with server */
   cap = create_shared_memory_cap(buffer, size);
   msg.data_ptr = cap;
   ipc_send(server_pid, &msg);
   ```

## Capability-Based Security

Servers receive minimal capabilities from the kernel:

### USB Server Capabilities
- `CAP_PCI_CONFIG`: Read PCI configuration space
- `CAP_MMIO_USB`: Access USB controller MMIO region
- `CAP_IRQ_USB`: Receive USB interrupts
- `CAP_DMA_ALLOC`: Allocate DMA buffers
- `CAP_DEVICE_CREATE`: Create USB device nodes

### File System Server Capabilities
- `CAP_BLOCK_READ`: Read from block devices
- `CAP_BLOCK_WRITE`: Write to block devices
- `CAP_PAGE_CACHE`: Access kernel page cache
- `CAP_INODE_CREATE`: Create new inodes

### Power Management Daemon Capabilities
- `CAP_ACPI_READ`: Read ACPI tables and methods
- `CAP_ACPI_EXECUTE`: Execute ACPI methods
- `CAP_CPU_FREQ`: Modify CPU frequency
- `CAP_POWER_STATE`: Change system power state

## Building Servers

### Prerequisites
- LimitlessOS kernel headers
- C compiler with userspace support
- Standard C library

### Build Command
```bash
# Build all servers
make -C servers

# Build specific server
make -C servers/drivers usb_server

# Build all file system servers
make -C servers/fs
```

### Installation
```bash
# Install servers to system
make -C servers install

# Servers are installed to:
# /system/servers/drivers/
# /system/servers/fs/
# /system/servers/services/
```

## Starting Servers

Servers are started by the init system during boot:

```bash
# /etc/init.d/servers.conf

# Start VFS server
start_server vfs /system/servers/fs/vfs_server

# Start USB server
start_server usb /system/servers/drivers/usb_server

# Start power management daemon
start_server power_mgmt /system/servers/services/power_mgmt_daemon
```

## Monitoring Servers

Use system tools to monitor server status:

```bash
# List running servers
serverctl list

# Check server status
serverctl status usb

# Restart server
serverctl restart vfs

# View server logs
serverctl logs power_mgmt
```

## Debugging Servers

Since servers run in userspace, standard debugging tools work:

```bash
# Attach GDB to running server
gdb -p $(pidof usb_server)

# Start server under GDB
gdb /system/servers/drivers/usb_server

# Use strace to trace system calls
strace -p $(pidof vfs_server)

# Profile server performance
perf record -p $(pidof usb_server)
```

## Error Handling

Servers implement robust error handling:

1. **Crash Recovery**: Kernel automatically restarts crashed servers
2. **Resource Cleanup**: Resources are freed on server exit
3. **State Restoration**: Servers save state periodically
4. **Error Reporting**: Errors logged to system logger
5. **Graceful Degradation**: System continues with reduced functionality

## Testing Servers

Each server includes test suite:

```bash
# Run server tests
make -C servers test

# Run specific server test
./servers/drivers/usb_server_test

# Run integration tests
make -C servers integration_test
```

## Performance Considerations

### IPC Overhead
- Synchronous IPC: ~500 cycles
- Asynchronous IPC: ~200 cycles
- Zero-copy transfer: ~100 cycles + DMA time

### Optimization Techniques
1. **Message Batching**: Combine multiple requests
2. **Caching**: Cache frequently accessed data
3. **Prefetching**: Predict future requests
4. **Parallel Processing**: Use multiple threads
5. **Zero-Copy**: Avoid data copying when possible

## Future Development

### Planned Servers
- [ ] Network driver server (Ethernet, Wi-Fi)
- [ ] Storage driver server (SATA, NVMe)
- [ ] Graphics driver server (Intel, AMD, NVIDIA)
- [ ] Audio driver server (HDA, USB Audio)
- [ ] Input driver server (Mouse, Keyboard, Touchpad)
- [ ] Display server (Wayland compositor)
- [ ] Audio server (PulseAudio equivalent)
- [ ] Network service daemon (DHCP, DNS)
- [ ] Print service daemon
- [ ] Bluetooth service daemon

### Enhancements
- [ ] Hot-plug driver loading
- [ ] Dynamic capability granting
- [ ] Server sandboxing with seccomp
- [ ] Performance profiling tools
- [ ] Automated testing framework
- [ ] Documentation generation

## Contributing

When adding new servers:

1. Follow coding style guidelines
2. Include comprehensive error handling
3. Add test cases
4. Document IPC interface
5. List required capabilities
6. Provide usage examples
7. Include performance benchmarks

## See Also

- [Hybrid Kernel Architecture](HYBRID_KERNEL_ARCHITECTURE.md)
- [Production Kernel README](PRODUCTION_README.md)
- [IPC Documentation](IPC.md)
- [Capability System](CAPABILITIES.md)

---

**Status**: Under Development | **Version**: 2.0.0
