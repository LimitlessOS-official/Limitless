# LimitlessOS Hybrid Kernel Architecture

## Overview
LimitlessOS implements a production-grade hybrid kernel architecture that combines the best aspects of microkernel and monolithic kernel designs. This architecture provides:

- **Performance**: Critical paths run in kernel mode for minimal overhead
- **Reliability**: Device drivers isolated in userspace with fault isolation
- **Security**: Principle of least privilege with capability-based security
- **Modularity**: Dynamic loading/unloading of kernel modules
- **POSIX Compliance**: Standard interfaces for application compatibility

## Architecture Layers

### Layer 1: Microkernel Core (Kernel Mode)
The minimal privileged core handles:
- Memory management (virtual memory, paging, page faults)
- Process/thread scheduling and context switching
- Inter-process communication (IPC) primitives
- Low-level interrupt and exception handling
- System call interface
- Basic synchronization primitives (mutexes, semaphores)

**Size Target**: < 50,000 lines of code
**Location**: `/kernel/core/`

### Layer 2: Kernel Services (Kernel Mode)
Essential services running in kernel space:
- Virtual File System (VFS) layer
- Network protocol stack (TCP/IP)
- Core device drivers (timer, interrupt controller, console)
- Security subsystem (capabilities, MAC, audit)
- Signal handling
- POSIX compatibility layer

**Size Target**: ~100,000 lines of code
**Location**: `/kernel/services/`

### Layer 3: Server Processes (User Mode)
System servers providing high-level services:
- Device driver servers (USB, PCI, network cards, etc.)
- File system servers (ext4, fat32, tmpfs, etc.)
- Network service daemons
- Display server / compositor
- Audio server
- Power management daemon

**Location**: `/servers/`

### Layer 4: User Applications
Standard POSIX applications and desktop environment

**Location**: `/userspace/`

## Key Design Principles

### 1. Hybrid Approach
- **Kernel Mode**: Memory management, scheduling, core IPC, critical drivers
- **User Mode**: Most device drivers, file systems, protocol handlers
- **Rationale**: Balance between performance and fault isolation

### 2. Message-Based IPC
- Synchronous and asynchronous message passing
- Zero-copy capabilities for large data transfers
- Priority-based message delivery
- Support for RPC-style communication

### 3. Capability-Based Security
- Fine-grained access control
- Object capabilities for resources
- Delegable permissions
- Least privilege by default

### 4. Fault Isolation
- Driver crashes don't crash kernel
- Automatic driver restart on failure
- Protected memory regions
- Resource limits per process

### 5. Real-Time Support
- Preemptive kernel
- Priority inheritance for mutexes
- Deadline scheduling
- Bounded interrupt latency

## Component Directory Structure

```
kernel/
├── core/                    # Microkernel core
│   ├── mm/                  # Memory management
│   ├── sched/               # Scheduler
│   ├── ipc/                 # Inter-process communication
│   ├── irq/                 # Interrupt handling
│   └── syscall/             # System call interface
├── services/                # Kernel services
│   ├── vfs/                 # Virtual file system
│   ├── net/                 # Network stack
│   ├── security/            # Security subsystem
│   └── drivers/             # Core kernel drivers
└── include/                 # Kernel headers

servers/                     # Userspace servers
├── drivers/                 # Device driver servers
├── fs/                      # File system servers
└── services/                # System service daemons

userspace/                   # User applications
├── libc/                    # C standard library
├── init/                    # Init system
├── shell/                   # Command shell
└── apps/                    # Applications
```

## Implementation Status

- [x] Architecture documentation
- [ ] Core memory management
- [ ] Process/thread scheduling
- [ ] IPC subsystem
- [ ] System call interface
- [ ] VFS layer
- [ ] Basic device drivers
- [ ] Userspace infrastructure
- [ ] Build system integration
- [ ] Testing framework
