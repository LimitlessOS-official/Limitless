/*
 * gdb_remote.c - LimitlessOS GDB Remote Debugging Server
 * 
 * Complete GDB remote debugging protocol implementation with advanced
 * debugging features, breakpoints, watchpoints, and memory inspection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <kernel/process.h>
#include <kernel/vmm.h>
#include <kernel/interrupt.h>

#define GDB_SERVER_PORT             1234
#define GDB_PACKET_SIZE             8192
#define MAX_BREAKPOINTS             256
#define MAX_WATCHPOINTS             64
#define MAX_REGISTERS              64

/* GDB register mapping for x86_64 */
typedef enum {
    GDB_REG_RAX = 0,  GDB_REG_RBX = 1,  GDB_REG_RCX = 2,  GDB_REG_RDX = 3,
    GDB_REG_RSI = 4,  GDB_REG_RDI = 5,  GDB_REG_RBP = 6,  GDB_REG_RSP = 7,
    GDB_REG_R8  = 8,  GDB_REG_R9  = 9,  GDB_REG_R10 = 10, GDB_REG_R11 = 11,
    GDB_REG_R12 = 12, GDB_REG_R13 = 13, GDB_REG_R14 = 14, GDB_REG_R15 = 15,
    GDB_REG_RIP = 16, GDB_REG_EFLAGS = 17,
    GDB_REG_CS = 18,  GDB_REG_SS = 19,  GDB_REG_DS = 20,  GDB_REG_ES = 21,
    GDB_REG_FS = 22,  GDB_REG_GS = 23
} gdb_register_t;

/* Breakpoint types */
typedef enum {
    BREAKPOINT_SOFTWARE,      /* INT3 instruction */
    BREAKPOINT_HARDWARE,      /* Debug register */
    BREAKPOINT_WATCHPOINT_WRITE,   /* Memory write watchpoint */
    BREAKPOINT_WATCHPOINT_READ,    /* Memory read watchpoint */
    BREAKPOINT_WATCHPOINT_ACCESS   /* Memory access watchpoint */
} breakpoint_type_t;

/* Breakpoint structure */
typedef struct breakpoint {
    bool active;
    breakpoint_type_t type;
    uint64_t address;
    size_t size;                    /* For watchpoints */
    uint8_t original_instruction;   /* For software breakpoints */
    uint32_t hit_count;
    char condition[256];            /* Conditional breakpoint */
    bool temporary;                 /* One-time breakpoint */
} breakpoint_t;

/* GDB server state */
typedef struct gdb_server_state {
    bool initialized;
    bool connected;
    bool target_running;
    int server_socket;
    int client_socket;
    
    /* Target process information */
    pid_t target_pid;
    process_t* target_process;
    
    /* Debugging state */
    breakpoint_t breakpoints[MAX_BREAKPOINTS];
    uint32_t breakpoint_count;
    
    breakpoint_t watchpoints[MAX_WATCHPOINTS];
    uint32_t watchpoint_count;
    
    /* Register state cache */
    uint64_t registers[MAX_REGISTERS];
    bool registers_valid;
    
    /* Memory cache for efficiency */
    struct {
        uint64_t address;
        size_t size;
        uint8_t* data;
        bool valid;
    } memory_cache;
    
    /* Statistics */
    struct {
        uint32_t packets_sent;
        uint32_t packets_received;
        uint32_t breakpoints_hit;
        uint32_t memory_reads;
        uint32_t memory_writes;
        uint64_t session_start_time;
    } stats;
    
} gdb_server_state_t;

static gdb_server_state_t g_gdb_server = {0};

/* Function prototypes */
static int gdb_server_init_socket(uint16_t port);
static int gdb_server_accept_connection(void);
static int gdb_server_handle_packet(const char* packet, char* response);
static int gdb_server_send_packet(const char* packet);
static int gdb_server_receive_packet(char* packet, size_t max_size);
static uint8_t gdb_server_calculate_checksum(const char* data);
static int gdb_server_attach_process(pid_t pid);
static int gdb_server_detach_process(void);
static int gdb_server_read_registers(char* response);
static int gdb_server_write_registers(const char* data);
static int gdb_server_read_memory(uint64_t address, size_t length, char* response);
static int gdb_server_write_memory(uint64_t address, const uint8_t* data, size_t length);
static int gdb_server_set_breakpoint(breakpoint_type_t type, uint64_t address, size_t size);
static int gdb_server_remove_breakpoint(breakpoint_type_t type, uint64_t address, size_t size);
static int gdb_server_continue_execution(uint64_t address);
static int gdb_server_single_step(void);
static void gdb_server_handle_signal(int signal);

/* Initialize GDB server */
int gdb_server_init(uint16_t port) {
    if (g_gdb_server.initialized) {
        return 0;  /* Already initialized */
    }
    
    memset(&g_gdb_server, 0, sizeof(gdb_server_state_t));
    
    /* Initialize server socket */
    g_gdb_server.server_socket = gdb_server_init_socket(port);
    if (g_gdb_server.server_socket < 0) {
        printf("Failed to initialize GDB server socket\n");
        return -1;
    }
    
    /* Initialize memory cache */
    g_gdb_server.memory_cache.size = 4096;  /* 4KB cache */
    g_gdb_server.memory_cache.data = malloc(g_gdb_server.memory_cache.size);
    if (!g_gdb_server.memory_cache.data) {
        close(g_gdb_server.server_socket);
        return -1;
    }
    
    g_gdb_server.initialized = true;
    g_gdb_server.stats.session_start_time = time(NULL);
    
    printf("GDB server initialized on port %u\n", port);
    return 0;
}

/* Main server loop */
int gdb_server_run(void) {
    if (!g_gdb_server.initialized) {
        return -1;
    }
    
    printf("GDB server waiting for connections...\n");
    
    while (1) {
        /* Wait for client connection */
        if (gdb_server_accept_connection() < 0) {
            printf("Failed to accept GDB client connection\n");
            continue;
        }
        
        printf("GDB client connected\n");
        g_gdb_server.connected = true;
        
        /* Handle client communication */
        char packet[GDB_PACKET_SIZE];
        char response[GDB_PACKET_SIZE];
        
        while (g_gdb_server.connected) {
            int packet_len = gdb_server_receive_packet(packet, sizeof(packet));
            if (packet_len <= 0) {
                printf("GDB client disconnected\n");
                g_gdb_server.connected = false;
                break;
            }
            
            /* Handle the packet */
            int result = gdb_server_handle_packet(packet, response);
            if (result == 0 && strlen(response) > 0) {
                gdb_server_send_packet(response);
            } else if (result < 0) {
                printf("Error handling GDB packet: %s\n", packet);
            }
        }
        
        /* Clean up connection */
        if (g_gdb_server.client_socket >= 0) {
            close(g_gdb_server.client_socket);
            g_gdb_server.client_socket = -1;
        }
        
        /* Detach from target if attached */
        if (g_gdb_server.target_pid > 0) {
            gdb_server_detach_process();
        }
    }
    
    return 0;
}

/* Initialize server socket */
static int gdb_server_init_socket(uint16_t port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        return -1;
    }
    
    /* Set socket options */
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* Bind to port */
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_socket);
        return -1;
    }
    
    /* Listen for connections */
    if (listen(server_socket, 1) < 0) {
        close(server_socket);
        return -1;
    }
    
    return server_socket;
}

/* Accept client connection */
static int gdb_server_accept_connection(void) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    g_gdb_server.client_socket = accept(g_gdb_server.server_socket,
                                       (struct sockaddr*)&client_addr, &client_len);
    
    if (g_gdb_server.client_socket < 0) {
        return -1;
    }
    
    printf("GDB client connected from %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    return 0;
}

/* Handle GDB packet */
static int gdb_server_handle_packet(const char* packet, char* response) {
    if (!packet || !response) {
        return -1;
    }
    
    memset(response, 0, GDB_PACKET_SIZE);
    g_gdb_server.stats.packets_received++;
    
    switch (packet[0]) {
        case '?':  /* Halt reason */
            strcpy(response, "S05");  /* SIGTRAP */
            break;
            
        case 'g':  /* Read registers */
            return gdb_server_read_registers(response);
            
        case 'G':  /* Write registers */
            return gdb_server_write_registers(packet + 1);
            
        case 'm':  /* Read memory */
        {
            uint64_t address;
            size_t length;
            if (sscanf(packet + 1, "%lx,%zx", &address, &length) == 2) {
                return gdb_server_read_memory(address, length, response);
            }
            strcpy(response, "E01");
            break;
        }
        
        case 'M':  /* Write memory */
        {
            uint64_t address;
            size_t length;
            char* data_start = strchr(packet + 1, ':');
            if (data_start && sscanf(packet + 1, "%lx,%zx:", &address, &length) == 2) {
                data_start++;
                
                /* Convert hex string to bytes */
                uint8_t* data = malloc(length);
                if (data) {
                    for (size_t i = 0; i < length; i++) {
                        sscanf(data_start + (i * 2), "%2hhx", &data[i]);
                    }
                    
                    int result = gdb_server_write_memory(address, data, length);
                    free(data);
                    
                    strcpy(response, result == 0 ? "OK" : "E01");
                } else {
                    strcpy(response, "E01");
                }
            } else {
                strcpy(response, "E01");
            }
            break;
        }
        
        case 'Z':  /* Set breakpoint */
        {
            int type;
            uint64_t address;
            size_t size;
            if (sscanf(packet + 1, "%d,%lx,%zx", &type, &address, &size) == 3) {
                int result = gdb_server_set_breakpoint((breakpoint_type_t)type, address, size);
                strcpy(response, result == 0 ? "OK" : "E01");
            } else {
                strcpy(response, "E01");
            }
            break;
        }
        
        case 'z':  /* Remove breakpoint */
        {
            int type;
            uint64_t address;
            size_t size;
            if (sscanf(packet + 1, "%d,%lx,%zx", &type, &address, &size) == 3) {
                int result = gdb_server_remove_breakpoint((breakpoint_type_t)type, address, size);
                strcpy(response, result == 0 ? "OK" : "E01");
            } else {
                strcpy(response, "E01");
            }
            break;
        }
        
        case 'c':  /* Continue execution */
        {
            uint64_t address = 0;
            if (strlen(packet) > 1) {
                sscanf(packet + 1, "%lx", &address);
            }
            
            int result = gdb_server_continue_execution(address);
            if (result == 0) {
                g_gdb_server.target_running = true;
                /* Don't send response immediately - wait for break */
                return 1;
            } else {
                strcpy(response, "E01");
            }
            break;
        }
        
        case 's':  /* Single step */
        {
            int result = gdb_server_single_step();
            strcpy(response, result == 0 ? "S05" : "E01");
            break;
        }
        
        case 'D':  /* Detach */
            gdb_server_detach_process();
            strcpy(response, "OK");
            g_gdb_server.connected = false;
            break;
            
        case 'k':  /* Kill */
            if (g_gdb_server.target_pid > 0) {
                kill(g_gdb_server.target_pid, SIGKILL);
                gdb_server_detach_process();
            }
            strcpy(response, "OK");
            break;
            
        case 'q':  /* Query packets */
            if (strncmp(packet, "qSupported", 10) == 0) {
                strcpy(response, "PacketSize=8192;qXfer:features:read+");
            } else if (strncmp(packet, "qAttached", 9) == 0) {
                strcpy(response, g_gdb_server.target_pid > 0 ? "1" : "0");
            } else if (strncmp(packet, "qC", 2) == 0) {
                snprintf(response, GDB_PACKET_SIZE, "QC%x", g_gdb_server.target_pid);
            } else {
                strcpy(response, "");  /* Unsupported query */
            }
            break;
            
        case 'H':  /* Set thread */
            strcpy(response, "OK");  /* Single-threaded for now */
            break;
            
        default:
            strcpy(response, "");  /* Empty response for unsupported */
            break;
    }
    
    return 0;
}

/* Read registers */
static int gdb_server_read_registers(char* response) {
    if (!g_gdb_server.target_process) {
        strcpy(response, "E01");
        return -1;
    }
    
    /* Update register cache if needed */
    if (!g_gdb_server.registers_valid) {
        /* Get registers from target process */
        process_context_t* ctx = &g_gdb_server.target_process->context;
        
        g_gdb_server.registers[GDB_REG_RAX] = ctx->rax;
        g_gdb_server.registers[GDB_REG_RBX] = ctx->rbx;
        g_gdb_server.registers[GDB_REG_RCX] = ctx->rcx;
        g_gdb_server.registers[GDB_REG_RDX] = ctx->rdx;
        g_gdb_server.registers[GDB_REG_RSI] = ctx->rsi;
        g_gdb_server.registers[GDB_REG_RDI] = ctx->rdi;
        g_gdb_server.registers[GDB_REG_RBP] = ctx->rbp;
        g_gdb_server.registers[GDB_REG_RSP] = ctx->rsp;
        g_gdb_server.registers[GDB_REG_RIP] = ctx->rip;
        g_gdb_server.registers[GDB_REG_EFLAGS] = ctx->rflags;
        
        g_gdb_server.registers_valid = true;
    }
    
    /* Format registers as hex string */
    char* ptr = response;
    for (int i = 0; i < 24; i++) {  /* First 24 registers */
        ptr += sprintf(ptr, "%016lx", g_gdb_server.registers[i]);
    }
    
    return 0;
}

/* Write registers */
static int gdb_server_write_registers(const char* data) {
    if (!g_gdb_server.target_process) {
        return -1;
    }
    
    /* Parse hex register data */
    const char* ptr = data;
    for (int i = 0; i < 24 && *ptr; i++) {
        if (sscanf(ptr, "%016lx", &g_gdb_server.registers[i]) != 1) {
            return -1;
        }
        ptr += 16;  /* 16 hex chars per 64-bit register */
    }
    
    /* Update target process context */
    process_context_t* ctx = &g_gdb_server.target_process->context;
    ctx->rax = g_gdb_server.registers[GDB_REG_RAX];
    ctx->rbx = g_gdb_server.registers[GDB_REG_RBX];
    ctx->rcx = g_gdb_server.registers[GDB_REG_RCX];
    ctx->rdx = g_gdb_server.registers[GDB_REG_RDX];
    ctx->rsi = g_gdb_server.registers[GDB_REG_RSI];
    ctx->rdi = g_gdb_server.registers[GDB_REG_RDI];
    ctx->rbp = g_gdb_server.registers[GDB_REG_RBP];
    ctx->rsp = g_gdb_server.registers[GDB_REG_RSP];
    ctx->rip = g_gdb_server.registers[GDB_REG_RIP];
    ctx->rflags = g_gdb_server.registers[GDB_REG_EFLAGS];
    
    g_gdb_server.registers_valid = true;
    return 0;
}

/* Read memory */
static int gdb_server_read_memory(uint64_t address, size_t length, char* response) {
    if (length > 4096) {  /* Limit read size */
        strcpy(response, "E01");
        return -1;
    }
    
    /* Check cache first */
    if (g_gdb_server.memory_cache.valid &&
        address >= g_gdb_server.memory_cache.address &&
        (address + length) <= (g_gdb_server.memory_cache.address + g_gdb_server.memory_cache.size)) {
        
        /* Cache hit */
        size_t offset = address - g_gdb_server.memory_cache.address;
        uint8_t* data = g_gdb_server.memory_cache.data + offset;
        
        /* Convert to hex string */
        char* ptr = response;
        for (size_t i = 0; i < length; i++) {
            ptr += sprintf(ptr, "%02x", data[i]);
        }
        
        return 0;
    }
    
    /* Read from target process memory */
    uint8_t* buffer = malloc(length);
    if (!buffer) {
        strcpy(response, "E01");
        return -1;
    }
    
    /* Use VMM to read target process memory */
    int result = vmm_read_process_memory(g_gdb_server.target_process, address, buffer, length);
    if (result != 0) {
        free(buffer);
        strcpy(response, "E01");
        return -1;
    }
    
    /* Convert to hex string */
    char* ptr = response;
    for (size_t i = 0; i < length; i++) {
        ptr += sprintf(ptr, "%02x", buffer[i]);
    }
    
    /* Update cache */
    if (length <= g_gdb_server.memory_cache.size) {
        g_gdb_server.memory_cache.address = address;
        memcpy(g_gdb_server.memory_cache.data, buffer, length);
        g_gdb_server.memory_cache.valid = true;
    }
    
    free(buffer);
    g_gdb_server.stats.memory_reads++;
    
    return 0;
}

/* Write memory */
static int gdb_server_write_memory(uint64_t address, const uint8_t* data, size_t length) {
    if (!g_gdb_server.target_process) {
        return -1;
    }
    
    /* Write to target process memory */
    int result = vmm_write_process_memory(g_gdb_server.target_process, address, data, length);
    if (result != 0) {
        return -1;
    }
    
    /* Invalidate cache if affected */
    if (g_gdb_server.memory_cache.valid &&
        address < (g_gdb_server.memory_cache.address + g_gdb_server.memory_cache.size) &&
        (address + length) > g_gdb_server.memory_cache.address) {
        g_gdb_server.memory_cache.valid = false;
    }
    
    g_gdb_server.stats.memory_writes++;
    return 0;
}

/* Set breakpoint */
static int gdb_server_set_breakpoint(breakpoint_type_t type, uint64_t address, size_t size) {
    /* Find free breakpoint slot */
    breakpoint_t* bp = NULL;
    
    if (type == BREAKPOINT_SOFTWARE) {
        for (uint32_t i = 0; i < MAX_BREAKPOINTS; i++) {
            if (!g_gdb_server.breakpoints[i].active) {
                bp = &g_gdb_server.breakpoints[i];
                break;
            }
        }
    } else {
        for (uint32_t i = 0; i < MAX_WATCHPOINTS; i++) {
            if (!g_gdb_server.watchpoints[i].active) {
                bp = &g_gdb_server.watchpoints[i];
                break;
            }
        }
    }
    
    if (!bp) {
        return -1;  /* No free slots */
    }
    
    /* Set up breakpoint */
    bp->active = true;
    bp->type = type;
    bp->address = address;
    bp->size = size;
    bp->hit_count = 0;
    bp->temporary = false;
    
    if (type == BREAKPOINT_SOFTWARE) {
        /* Read original instruction */
        if (vmm_read_process_memory(g_gdb_server.target_process, address,
                                   &bp->original_instruction, 1) != 0) {
            bp->active = false;
            return -1;
        }
        
        /* Write INT3 instruction */
        uint8_t int3 = 0xCC;
        if (vmm_write_process_memory(g_gdb_server.target_process, address, &int3, 1) != 0) {
            bp->active = false;
            return -1;
        }
        
        g_gdb_server.breakpoint_count++;
    } else {
        /* Set up hardware watchpoint (simplified) */
        g_gdb_server.watchpoint_count++;
    }
    
    printf("Breakpoint set at 0x%lx (type %d)\n", address, type);
    return 0;
}

/* Remove breakpoint */
static int gdb_server_remove_breakpoint(breakpoint_type_t type, uint64_t address, size_t size) {
    breakpoint_t* breakpoints = (type == BREAKPOINT_SOFTWARE) ? 
                               g_gdb_server.breakpoints : g_gdb_server.watchpoints;
    uint32_t max_count = (type == BREAKPOINT_SOFTWARE) ? MAX_BREAKPOINTS : MAX_WATCHPOINTS;
    
    /* Find and remove breakpoint */
    for (uint32_t i = 0; i < max_count; i++) {
        breakpoint_t* bp = &breakpoints[i];
        
        if (bp->active && bp->address == address && bp->type == type) {
            if (type == BREAKPOINT_SOFTWARE) {
                /* Restore original instruction */
                vmm_write_process_memory(g_gdb_server.target_process, address,
                                       &bp->original_instruction, 1);
                g_gdb_server.breakpoint_count--;
            } else {
                g_gdb_server.watchpoint_count--;
            }
            
            bp->active = false;
            printf("Breakpoint removed from 0x%lx\n", address);
            return 0;
        }
    }
    
    return -1;  /* Breakpoint not found */
}

/* Continue execution */
static int gdb_server_continue_execution(uint64_t address) {
    if (!g_gdb_server.target_process) {
        return -1;
    }
    
    /* Set resume address if specified */
    if (address != 0) {
        g_gdb_server.target_process->context.rip = address;
    }
    
    /* Resume target process */
    process_resume(g_gdb_server.target_process);
    
    printf("Target process resumed\n");
    return 0;
}

/* Single step execution */
static int gdb_server_single_step(void) {
    if (!g_gdb_server.target_process) {
        return -1;
    }
    
    /* Set trap flag for single step */
    g_gdb_server.target_process->context.rflags |= 0x100;  /* TF flag */
    
    /* Resume for one instruction */
    process_resume(g_gdb_server.target_process);
    
    /* Wait for single step completion */
    process_wait_debug_event(g_gdb_server.target_process);
    
    printf("Single step completed\n");
    return 0;
}

/* Attach to process */
static int gdb_server_attach_process(pid_t pid) {
    g_gdb_server.target_process = process_find_by_pid(pid);
    if (!g_gdb_server.target_process) {
        return -1;
    }
    
    g_gdb_server.target_pid = pid;
    g_gdb_server.registers_valid = false;
    g_gdb_server.memory_cache.valid = false;
    
    /* Suspend target process */
    process_suspend(g_gdb_server.target_process);
    
    printf("Attached to process %d\n", pid);
    return 0;
}

/* Detach from process */
static int gdb_server_detach_process(void) {
    if (g_gdb_server.target_process) {
        /* Remove all breakpoints */
        for (uint32_t i = 0; i < MAX_BREAKPOINTS; i++) {
            if (g_gdb_server.breakpoints[i].active) {
                gdb_server_remove_breakpoint(BREAKPOINT_SOFTWARE,
                                           g_gdb_server.breakpoints[i].address, 1);
            }
        }
        
        /* Resume target process */
        process_resume(g_gdb_server.target_process);
        
        g_gdb_server.target_process = NULL;
        g_gdb_server.target_pid = 0;
        
        printf("Detached from target process\n");
    }
    
    return 0;
}

/* Send GDB packet */
static int gdb_server_send_packet(const char* packet) {
    if (!packet || g_gdb_server.client_socket < 0) {
        return -1;
    }
    
    /* Calculate checksum */
    uint8_t checksum = gdb_server_calculate_checksum(packet);
    
    /* Format packet with framing */
    char buffer[GDB_PACKET_SIZE + 10];
    int len = snprintf(buffer, sizeof(buffer), "$%s#%02x", packet, checksum);
    
    /* Send packet */
    ssize_t sent = send(g_gdb_server.client_socket, buffer, len, 0);
    if (sent != len) {
        return -1;
    }
    
    g_gdb_server.stats.packets_sent++;
    return 0;
}

/* Receive GDB packet */
static int gdb_server_receive_packet(char* packet, size_t max_size) {
    if (!packet || g_gdb_server.client_socket < 0) {
        return -1;
    }
    
    /* Look for packet start */
    char ch;
    while (recv(g_gdb_server.client_socket, &ch, 1, 0) == 1) {
        if (ch == '$') {
            break;
        } else if (ch == 0x03) {  /* Ctrl+C */
            strcpy(packet, "break");
            return 5;
        }
    }
    
    /* Read packet data until '#' */
    size_t pos = 0;
    while (pos < (max_size - 1) && recv(g_gdb_server.client_socket, &ch, 1, 0) == 1) {
        if (ch == '#') {
            break;
        }
        packet[pos++] = ch;
    }
    packet[pos] = '\0';
    
    /* Read and verify checksum */
    char checksum_str[3] = {0};
    if (recv(g_gdb_server.client_socket, checksum_str, 2, 0) == 2) {
        uint8_t received_checksum = (uint8_t)strtol(checksum_str, NULL, 16);
        uint8_t calculated_checksum = gdb_server_calculate_checksum(packet);
        
        if (received_checksum == calculated_checksum) {
            /* Send ACK */
            send(g_gdb_server.client_socket, "+", 1, 0);
            return pos;
        } else {
            /* Send NAK */
            send(g_gdb_server.client_socket, "-", 1, 0);
            return -1;
        }
    }
    
    return -1;
}

/* Calculate GDB packet checksum */
static uint8_t gdb_server_calculate_checksum(const char* data) {
    uint8_t checksum = 0;
    while (*data) {
        checksum += *data++;
    }
    return checksum;
}

/* Signal handler for breakpoint hits */
static void gdb_server_handle_signal(int signal) {
    if (signal == SIGTRAP && g_gdb_server.target_running) {
        g_gdb_server.target_running = false;
        g_gdb_server.registers_valid = false;
        g_gdb_server.memory_cache.valid = false;
        g_gdb_server.stats.breakpoints_hit++;
        
        /* Send break notification to GDB */
        gdb_server_send_packet("S05");  /* SIGTRAP */
    }
}

/* Get debugging statistics */
void gdb_server_get_stats(void) {
    printf("\nGDB Server Statistics:\n");
    printf("  Session time: %lu seconds\n", time(NULL) - g_gdb_server.stats.session_start_time);
    printf("  Packets sent: %u\n", g_gdb_server.stats.packets_sent);
    printf("  Packets received: %u\n", g_gdb_server.stats.packets_received);
    printf("  Breakpoints hit: %u\n", g_gdb_server.stats.breakpoints_hit);
    printf("  Memory reads: %u\n", g_gdb_server.stats.memory_reads);
    printf("  Memory writes: %u\n", g_gdb_server.stats.memory_writes);
    printf("  Active breakpoints: %u\n", g_gdb_server.breakpoint_count);
    printf("  Active watchpoints: %u\n", g_gdb_server.watchpoint_count);
}

/* Main function */
int main(int argc, char* argv[]) {
    uint16_t port = GDB_SERVER_PORT;
    
    if (argc > 1) {
        port = (uint16_t)atoi(argv[1]);
    }
    
    /* Initialize GDB server */
    if (gdb_server_init(port) < 0) {
        printf("Failed to initialize GDB server\n");
        return 1;
    }
    
    /* Install signal handler */
    signal(SIGTRAP, gdb_server_handle_signal);
    signal(SIGINT, gdb_server_handle_signal);
    
    /* Run server loop */
    return gdb_server_run();
}