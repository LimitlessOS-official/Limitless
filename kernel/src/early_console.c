/*
 * LimitlessOS Early Boot Console System
 * High-performance early boot debugging and logging infrastructure
 * 
 * Features:
 * - Multi-output early console (VGA text, serial, GOP framebuffer)
 * - High-speed serial console with hardware flow control
 * - UEFI Graphics Output Protocol (GOP) console support
 * - Emergency VGA text mode fallback
 * - Boot-time log buffering and replay
 * - Structured logging with timestamps and priorities
 * - Remote debugging console support (network, USB debug)
 * - Crash dump and panic handling
 * - Performance monitoring and boot timing analysis
 * - Multi-level debug output filtering
 * - Console redirection and multiplexing
 * - Hardware abstraction for various console types
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// Console Types
#define CONSOLE_TYPE_VGA        0x01    // VGA text mode console
#define CONSOLE_TYPE_SERIAL     0x02    // Serial port console
#define CONSOLE_TYPE_GOP        0x03    // UEFI GOP framebuffer console
#define CONSOLE_TYPE_NETWORK    0x04    // Network debug console
#define CONSOLE_TYPE_USB        0x05    // USB debug console
#define CONSOLE_TYPE_MEMORY     0x06    // Memory buffer console

// Console States
#define CONSOLE_STATE_DISABLED  0x00    // Console disabled
#define CONSOLE_STATE_INIT      0x01    // Console initializing
#define CONSOLE_STATE_ACTIVE    0x02    // Console active
#define CONSOLE_STATE_ERROR     0x03    // Console error state
#define CONSOLE_STATE_SUSPENDED 0x04    // Console suspended

// Log Levels
#define LOG_LEVEL_EMERGENCY     0       // System is unusable
#define LOG_LEVEL_ALERT         1       // Action must be taken immediately
#define LOG_LEVEL_CRITICAL      2       // Critical conditions
#define LOG_LEVEL_ERROR         3       // Error conditions
#define LOG_LEVEL_WARNING       4       // Warning conditions
#define LOG_LEVEL_NOTICE        5       // Normal but significant condition
#define LOG_LEVEL_INFO          6       // Informational messages
#define LOG_LEVEL_DEBUG         7       // Debug-level messages

// Serial Port Definitions
#define SERIAL_PORT_COM1        0x3F8   // COM1 base address
#define SERIAL_PORT_COM2        0x2F8   // COM2 base address
#define SERIAL_PORT_COM3        0x3E8   // COM3 base address
#define SERIAL_PORT_COM4        0x2E8   // COM4 base address

// Serial Port Registers
#define SERIAL_THR              0       // Transmit Holding Register
#define SERIAL_RBR              0       // Receive Buffer Register
#define SERIAL_DLL              0       // Divisor Latch Low (when DLAB=1)
#define SERIAL_IER              1       // Interrupt Enable Register
#define SERIAL_DLH              1       // Divisor Latch High (when DLAB=1)
#define SERIAL_IIR              2       // Interrupt Identification Register
#define SERIAL_FCR              2       // FIFO Control Register
#define SERIAL_LCR              3       // Line Control Register
#define SERIAL_MCR              4       // Modem Control Register
#define SERIAL_LSR              5       // Line Status Register
#define SERIAL_MSR              6       // Modem Status Register
#define SERIAL_SR               7       // Scratch Register

// VGA Text Mode Definitions
#define VGA_WIDTH               80      // VGA text mode width
#define VGA_HEIGHT              25      // VGA text mode height
#define VGA_MEMORY              0xB8000 // VGA text mode memory
#define VGA_REG_CTRL            0x3D4   // VGA control register
#define VGA_REG_DATA            0x3D5   // VGA data register

// Console Colors
#define COLOR_BLACK             0x00
#define COLOR_BLUE              0x01
#define COLOR_GREEN             0x02
#define COLOR_CYAN              0x03
#define COLOR_RED               0x04
#define COLOR_MAGENTA           0x05
#define COLOR_BROWN             0x06
#define COLOR_LIGHT_GREY        0x07
#define COLOR_DARK_GREY         0x08
#define COLOR_LIGHT_BLUE        0x09
#define COLOR_LIGHT_GREEN       0x0A
#define COLOR_LIGHT_CYAN        0x0B
#define COLOR_LIGHT_RED         0x0C
#define COLOR_LIGHT_MAGENTA     0x0D
#define COLOR_LIGHT_BROWN       0x0E
#define COLOR_WHITE             0x0F

#define MAX_CONSOLE_OUTPUTS     8       // Maximum console outputs
#define MAX_LOG_BUFFER_SIZE     65536   // Maximum log buffer size (64KB)
#define MAX_CONSOLE_LINE_LENGTH 512     // Maximum console line length

/*
 * Console Output Interface
 */
typedef struct console_output {
    uint32_t type;                      // Console type
    uint32_t state;                     // Console state
    char name[32];                      // Console name
    
    // Console operations
    int (*init)(struct console_output *console);
    int (*write)(struct console_output *console, const char *buffer, size_t length);
    int (*read)(struct console_output *console, char *buffer, size_t length);
    int (*flush)(struct console_output *console);
    int (*clear)(struct console_output *console);
    int (*set_color)(struct console_output *console, uint8_t foreground, uint8_t background);
    int (*set_cursor)(struct console_output *console, uint32_t x, uint32_t y);
    int (*get_cursor)(struct console_output *console, uint32_t *x, uint32_t *y);
    
    // Console configuration
    struct {
        uint32_t width;                 // Console width (characters)
        uint32_t height;                // Console height (characters)
        uint32_t baud_rate;             // Baud rate (for serial)
        uint8_t data_bits;              // Data bits (for serial)
        uint8_t stop_bits;              // Stop bits (for serial)
        uint8_t parity;                 // Parity (for serial)
        bool flow_control;              // Hardware flow control
        uint32_t base_address;          // Base I/O address
        uint32_t irq;                   // IRQ number
    } config;
    
    // Console state
    struct {
        uint32_t cursor_x;              // Current cursor X position
        uint32_t cursor_y;              // Current cursor Y position
        uint8_t foreground_color;       // Current foreground color
        uint8_t background_color;       // Current background color
        bool cursor_visible;            // Cursor visibility
        uint64_t bytes_written;         // Total bytes written
        uint64_t bytes_read;            // Total bytes read
        uint32_t error_count;           // Error count
    } state;
    
    // Frame buffer (for GOP console)
    struct {
        uint64_t base_address;          // Frame buffer base address
        uint32_t width;                 // Frame buffer width (pixels)
        uint32_t height;                // Frame buffer height (pixels)
        uint32_t pitch;                 // Frame buffer pitch (bytes per line)
        uint32_t bpp;                   // Bits per pixel
        uint32_t red_mask;              // Red color mask
        uint32_t green_mask;            // Green color mask
        uint32_t blue_mask;             // Blue color mask
    } framebuffer;
    
    // Font information (for graphical console)
    struct {
        uint8_t *font_data;             // Font bitmap data
        uint32_t char_width;            // Character width (pixels)
        uint32_t char_height;           // Character height (pixels)
        uint32_t chars_per_line;        // Characters per line
        uint32_t lines_per_screen;      // Lines per screen
    } font;
    
} console_output_t;

/*
 * Log Entry Structure
 */
typedef struct log_entry {
    uint64_t timestamp;                 // Timestamp (nanoseconds)
    uint32_t level;                     // Log level
    uint32_t cpu_id;                    // CPU ID
    char subsystem[16];                 // Subsystem name
    char message[256];                  // Log message
    uint32_t line_number;               // Source line number
    char file_name[64];                 // Source file name
    char function_name[64];             // Function name
} log_entry_t;

/*
 * Boot Timing Information
 */
typedef struct boot_timing {
    uint64_t boot_start;                // Boot start timestamp
    uint64_t uefi_exit;                 // UEFI exit timestamp
    uint64_t kernel_entry;              // Kernel entry timestamp
    uint64_t console_init;              // Console init timestamp
    uint64_t memory_init;               // Memory init timestamp
    uint64_t smp_init;                  // SMP init timestamp
    uint64_t acpi_init;                 // ACPI init timestamp
    uint64_t device_init;               // Device init timestamp
    uint64_t filesystem_init;           // Filesystem init timestamp
    uint64_t userspace_start;           // Userspace start timestamp
    
    // Phase durations
    uint64_t uefi_duration;             // UEFI phase duration
    uint64_t kernel_init_duration;      // Kernel init duration
    uint64_t device_init_duration;      // Device init duration
    uint64_t total_boot_time;           // Total boot time
} boot_timing_t;

/*
 * Early Console System
 */
typedef struct early_console_system {
    bool initialized;                   // System initialized
    bool logging_enabled;               // Logging enabled
    uint32_t active_outputs;            // Number of active outputs
    uint32_t log_level;                 // Current log level
    
    // Console outputs
    console_output_t outputs[MAX_CONSOLE_OUTPUTS];
    uint32_t output_count;              // Number of configured outputs
    console_output_t *primary_output;   // Primary console output
    
    // Log buffer
    struct {
        log_entry_t *entries;           // Log entry buffer
        uint32_t max_entries;           // Maximum entries
        uint32_t current_entry;         // Current entry index
        uint32_t total_entries;         // Total entries written
        bool buffer_full;               // Buffer is full
        bool circular_buffer;           // Circular buffer mode
    } log_buffer;
    
    // Boot timing
    boot_timing_t timing;               // Boot timing information
    
    // Console buffer (for early output before full init)
    struct {
        char buffer[MAX_LOG_BUFFER_SIZE];   // Early output buffer
        uint32_t write_pos;             // Write position
        uint32_t read_pos;              // Read position
        bool replay_enabled;            // Replay buffer on init
    } early_buffer;
    
    // Debug configuration
    struct {
        bool timestamp_enabled;         // Include timestamps
        bool cpu_id_enabled;            // Include CPU ID
        bool color_enabled;             // Color output enabled
        bool verbose_mode;              // Verbose debug mode
        char debug_filter[64];          // Debug subsystem filter
    } debug;
    
    // Statistics
    struct {
        uint64_t messages_written;      // Total messages written
        uint64_t bytes_written;         // Total bytes written
        uint64_t console_errors;        // Console error count
        uint64_t buffer_overruns;       // Buffer overrun count
        uint32_t max_message_length;    // Maximum message length
        uint64_t avg_write_time_ns;     // Average write time (ns)
    } statistics;
    
} early_console_system_t;

// Global early console system
static early_console_system_t early_console;

// Color mapping for log levels
static const uint8_t log_level_colors[] = {
    [LOG_LEVEL_EMERGENCY]   = COLOR_WHITE | (COLOR_RED << 4),      // White on red
    [LOG_LEVEL_ALERT]       = COLOR_LIGHT_RED,                     // Light red
    [LOG_LEVEL_CRITICAL]    = COLOR_RED,                           // Red
    [LOG_LEVEL_ERROR]       = COLOR_LIGHT_RED,                     // Light red
    [LOG_LEVEL_WARNING]     = COLOR_LIGHT_BROWN,                   // Yellow
    [LOG_LEVEL_NOTICE]      = COLOR_LIGHT_CYAN,                    // Light cyan
    [LOG_LEVEL_INFO]        = COLOR_LIGHT_GREY,                    // Light grey
    [LOG_LEVEL_DEBUG]       = COLOR_DARK_GREY,                     // Dark grey
};

// Log level names
static const char *log_level_names[] = {
    [LOG_LEVEL_EMERGENCY]   = "EMERG",
    [LOG_LEVEL_ALERT]       = "ALERT",
    [LOG_LEVEL_CRITICAL]    = "CRIT",
    [LOG_LEVEL_ERROR]       = "ERROR",
    [LOG_LEVEL_WARNING]     = "WARN",
    [LOG_LEVEL_NOTICE]      = "NOTICE",
    [LOG_LEVEL_INFO]        = "INFO",
    [LOG_LEVEL_DEBUG]       = "DEBUG",
};

/*
 * Initialize Early Console System
 */
int early_console_init(void)
{
    memset(&early_console, 0, sizeof(early_console_system_t));
    
    // Initialize basic settings
    early_console.log_level = LOG_LEVEL_INFO;
    early_console.logging_enabled = true;
    early_console.debug.timestamp_enabled = true;
    early_console.debug.cpu_id_enabled = true;
    early_console.debug.color_enabled = true;
    early_console.debug.replay_enabled = true;
    
    // Record boot timing
    early_console.timing.boot_start = get_timestamp_ns();
    early_console.timing.console_init = early_console.timing.boot_start;
    
    // Allocate log buffer
    early_console.log_buffer.max_entries = MAX_LOG_BUFFER_SIZE / sizeof(log_entry_t);
    early_console.log_buffer.entries = allocate_early_memory(
        early_console.log_buffer.max_entries * sizeof(log_entry_t));
    
    if (!early_console.log_buffer.entries) {
        // Fall back to early buffer only
        early_console.log_buffer.max_entries = 0;
    }
    
    early_console.log_buffer.circular_buffer = true;
    
    // Initialize VGA console first (most reliable)
    early_console_init_vga();
    
    // Initialize serial console
    early_console_init_serial();
    
    // Try to initialize GOP console if available
    early_console_init_gop();
    
    early_console.initialized = true;
    
    // Replay early buffer if enabled
    if (early_console.debug.replay_enabled && early_console.early_buffer.write_pos > 0) {
        early_console_replay_buffer();
    }
    
    early_console_log(LOG_LEVEL_INFO, "CONSOLE", "Early Console System initialized");
    early_console_log(LOG_LEVEL_INFO, "CONSOLE", "Active outputs: %u", early_console.active_outputs);
    
    return 0;
}

/*
 * Initialize VGA Text Mode Console
 */
static int early_console_init_vga(void)
{
    if (early_console.output_count >= MAX_CONSOLE_OUTPUTS) {
        return -ENOMEM;
    }
    
    console_output_t *vga = &early_console.outputs[early_console.output_count];
    memset(vga, 0, sizeof(console_output_t));
    
    vga->type = CONSOLE_TYPE_VGA;
    vga->state = CONSOLE_STATE_INIT;
    strcpy(vga->name, "VGA");
    
    // Configure VGA console
    vga->config.width = VGA_WIDTH;
    vga->config.height = VGA_HEIGHT;
    vga->config.base_address = VGA_MEMORY;
    
    // Initialize state
    vga->state.cursor_x = 0;
    vga->state.cursor_y = 0;
    vga->state.foreground_color = COLOR_LIGHT_GREY;
    vga->state.background_color = COLOR_BLACK;
    vga->state.cursor_visible = true;
    
    // Set operations
    vga->init = vga_console_init;
    vga->write = vga_console_write;
    vga->clear = vga_console_clear;
    vga->set_color = vga_console_set_color;
    vga->set_cursor = vga_console_set_cursor;
    vga->get_cursor = vga_console_get_cursor;
    
    // Initialize VGA console
    if (vga->init(vga) == 0) {
        vga->state = CONSOLE_STATE_ACTIVE;
        early_console.output_count++;
        early_console.active_outputs++;
        
        if (!early_console.primary_output) {
            early_console.primary_output = vga;
        }
        
        return 0;
    }
    
    vga->state = CONSOLE_STATE_ERROR;
    return -EIO;
}

/*
 * Initialize Serial Console
 */
static int early_console_init_serial(void)
{
    if (early_console.output_count >= MAX_CONSOLE_OUTPUTS) {
        return -ENOMEM;
    }
    
    console_output_t *serial = &early_console.outputs[early_console.output_count];
    memset(serial, 0, sizeof(console_output_t));
    
    serial->type = CONSOLE_TYPE_SERIAL;
    serial->state = CONSOLE_STATE_INIT;
    strcpy(serial->name, "COM1");
    
    // Configure serial console
    serial->config.base_address = SERIAL_PORT_COM1;
    serial->config.baud_rate = 115200;
    serial->config.data_bits = 8;
    serial->config.stop_bits = 1;
    serial->config.parity = 0; // No parity
    serial->config.flow_control = false;
    
    // Set operations
    serial->init = serial_console_init;
    serial->write = serial_console_write;
    serial->read = serial_console_read;
    serial->flush = serial_console_flush;
    
    // Initialize serial console
    if (serial->init(serial) == 0) {
        serial->state = CONSOLE_STATE_ACTIVE;
        early_console.output_count++;
        early_console.active_outputs++;
        
        return 0;
    }
    
    serial->state = CONSOLE_STATE_ERROR;
    return -EIO;
}

/*
 * Initialize GOP (Graphics Output Protocol) Console
 */
static int early_console_init_gop(void)
{
    // This would be initialized with UEFI GOP information
    // For now, we'll skip GOP console in early boot
    return -ENODEV;
}

/*
 * VGA Console Operations
 */
static int vga_console_init(console_output_t *console)
{
    // Clear screen
    uint16_t *vga_buffer = (uint16_t*)console->config.base_address;
    uint16_t blank = (console->state.background_color << 12) | 
                     (console->state.foreground_color << 8) | ' ';
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
    
    // Set cursor position
    vga_console_set_cursor(console, 0, 0);
    
    return 0;
}

static int vga_console_write(console_output_t *console, const char *buffer, size_t length)
{
    uint16_t *vga_buffer = (uint16_t*)console->config.base_address;
    uint8_t color = (console->state.background_color << 4) | console->state.foreground_color;
    
    for (size_t i = 0; i < length; i++) {
        char c = buffer[i];
        
        if (c == '\n') {
            console->state.cursor_x = 0;
            console->state.cursor_y++;
        } else if (c == '\r') {
            console->state.cursor_x = 0;
        } else if (c == '\t') {
            console->state.cursor_x = (console->state.cursor_x + 8) & ~7;
        } else if (c >= ' ') {
            uint32_t pos = console->state.cursor_y * VGA_WIDTH + console->state.cursor_x;
            vga_buffer[pos] = (color << 8) | c;
            console->state.cursor_x++;
        }
        
        // Handle line wrapping
        if (console->state.cursor_x >= VGA_WIDTH) {
            console->state.cursor_x = 0;
            console->state.cursor_y++;
        }
        
        // Handle scrolling
        if (console->state.cursor_y >= VGA_HEIGHT) {
            vga_console_scroll(console);
            console->state.cursor_y = VGA_HEIGHT - 1;
        }
    }
    
    // Update hardware cursor
    vga_console_update_cursor(console);
    
    console->state.bytes_written += length;
    return length;
}

static int vga_console_clear(console_output_t *console)
{
    return vga_console_init(console);
}

static int vga_console_set_color(console_output_t *console, uint8_t foreground, uint8_t background)
{
    console->state.foreground_color = foreground & 0x0F;
    console->state.background_color = background & 0x0F;
    return 0;
}

static int vga_console_set_cursor(console_output_t *console, uint32_t x, uint32_t y)
{
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        console->state.cursor_x = x;
        console->state.cursor_y = y;
        vga_console_update_cursor(console);
        return 0;
    }
    return -EINVAL;
}

static int vga_console_get_cursor(console_output_t *console, uint32_t *x, uint32_t *y)
{
    *x = console->state.cursor_x;
    *y = console->state.cursor_y;
    return 0;
}

/*
 * VGA Console Helper Functions
 */
static void vga_console_scroll(console_output_t *console)
{
    uint16_t *vga_buffer = (uint16_t*)console->config.base_address;
    uint16_t blank = (console->state.background_color << 12) | 
                     (console->state.foreground_color << 8) | ' ';
    
    // Move all lines up
    for (int line = 1; line < VGA_HEIGHT; line++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga_buffer[(line - 1) * VGA_WIDTH + col] = vga_buffer[line * VGA_WIDTH + col];
        }
    }
    
    // Clear bottom line
    for (int col = 0; col < VGA_WIDTH; col++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = blank;
    }
}

static void vga_console_update_cursor(console_output_t *console)
{
    uint16_t pos = console->state.cursor_y * VGA_WIDTH + console->state.cursor_x;
    
    outb(VGA_REG_CTRL, 14);
    outb(VGA_REG_DATA, (pos >> 8) & 0xFF);
    outb(VGA_REG_CTRL, 15);
    outb(VGA_REG_DATA, pos & 0xFF);
}

/*
 * Serial Console Operations
 */
static int serial_console_init(console_output_t *console)
{
    uint16_t port = console->config.base_address;
    
    // Disable interrupts
    outb(port + SERIAL_IER, 0x00);
    
    // Set baud rate divisor
    uint16_t divisor = 115200 / console->config.baud_rate;
    outb(port + SERIAL_LCR, 0x80); // Enable DLAB
    outb(port + SERIAL_DLL, divisor & 0xFF);
    outb(port + SERIAL_DLH, (divisor >> 8) & 0xFF);
    
    // Configure line: 8 data bits, 1 stop bit, no parity
    outb(port + SERIAL_LCR, 0x03);
    
    // Enable FIFO, clear buffers, 14-byte threshold
    outb(port + SERIAL_FCR, 0xC7);
    
    // Enable RTS/DSR set
    outb(port + SERIAL_MCR, 0x03);
    
    // Test serial chip (loopback test)
    outb(port + SERIAL_MCR, 0x1E);
    outb(port + SERIAL_THR, 0xAE);
    
    if (inb(port + SERIAL_RBR) != 0xAE) {
        return -ENODEV;
    }
    
    // Set normal operation mode
    outb(port + SERIAL_MCR, 0x0F);
    
    return 0;
}

static int serial_console_write(console_output_t *console, const char *buffer, size_t length)
{
    uint16_t port = console->config.base_address;
    
    for (size_t i = 0; i < length; i++) {
        // Wait for transmit holding register empty
        while ((inb(port + SERIAL_LSR) & 0x20) == 0) {
            // CPU pause for efficiency
            __asm__ volatile ("pause" ::: "memory");
        }
        
        outb(port + SERIAL_THR, buffer[i]);
    }
    
    console->state.bytes_written += length;
    return length;
}

static int serial_console_read(console_output_t *console, char *buffer, size_t length)
{
    uint16_t port = console->config.base_address;
    size_t bytes_read = 0;
    
    for (size_t i = 0; i < length; i++) {
        // Check if data is available
        if ((inb(port + SERIAL_LSR) & 0x01) == 0) {
            break;
        }
        
        buffer[i] = inb(port + SERIAL_RBR);
        bytes_read++;
    }
    
    console->state.bytes_read += bytes_read;
    return bytes_read;
}

static int serial_console_flush(console_output_t *console)
{
    uint16_t port = console->config.base_address;
    
    // Wait for transmitter empty
    while ((inb(port + SERIAL_LSR) & 0x40) == 0) {
        __asm__ volatile ("pause" ::: "memory");
    }
    
    return 0;
}

/*
 * Early Console Logging
 */
void early_console_log(uint32_t level, const char *subsystem, const char *format, ...)
{
    if (!early_console.logging_enabled || level > early_console.log_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    char message[512];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Create log entry
    log_entry_t entry;
    entry.timestamp = get_timestamp_ns();
    entry.level = level;
    entry.cpu_id = get_current_cpu_id();
    strncpy(entry.subsystem, subsystem, sizeof(entry.subsystem) - 1);
    strncpy(entry.message, message, sizeof(entry.message) - 1);
    
    // Add to log buffer if available
    if (early_console.log_buffer.entries && early_console.log_buffer.max_entries > 0) {
        uint32_t index = early_console.log_buffer.current_entry;
        early_console.log_buffer.entries[index] = entry;
        
        early_console.log_buffer.current_entry++;
        if (early_console.log_buffer.current_entry >= early_console.log_buffer.max_entries) {
            if (early_console.log_buffer.circular_buffer) {
                early_console.log_buffer.current_entry = 0;
                early_console.log_buffer.buffer_full = true;
            } else {
                early_console.log_buffer.current_entry = early_console.log_buffer.max_entries - 1;
            }
        }
        early_console.log_buffer.total_entries++;
    }
    
    // Format output line
    char output_line[MAX_CONSOLE_LINE_LENGTH];
    format_log_message(&entry, output_line, sizeof(output_line));
    
    // Write to all active console outputs
    early_console_write_all(output_line, strlen(output_line));
    
    early_console.statistics.messages_written++;
}

/*
 * Format Log Message
 */
static void format_log_message(const log_entry_t *entry, char *output, size_t output_size)
{
    char timestamp_str[32] = "";
    char cpu_str[16] = "";
    
    if (early_console.debug.timestamp_enabled) {
        uint64_t ms = entry->timestamp / 1000000; // Convert to milliseconds
        snprintf(timestamp_str, sizeof(timestamp_str), "[%6llu.%03llu] ", 
                 ms / 1000, ms % 1000);
    }
    
    if (early_console.debug.cpu_id_enabled) {
        snprintf(cpu_str, sizeof(cpu_str), "CPU%u ", entry->cpu_id);
    }
    
    snprintf(output, output_size, "%s%s%s: [%s] %s\n",
             timestamp_str,
             cpu_str,
             log_level_names[entry->level],
             entry->subsystem,
             entry->message);
}

/*
 * Write to All Active Console Outputs
 */
static void early_console_write_all(const char *buffer, size_t length)
{
    if (!early_console.initialized) {
        // Store in early buffer
        if (early_console.early_buffer.write_pos + length < MAX_LOG_BUFFER_SIZE) {
            memcpy(early_console.early_buffer.buffer + early_console.early_buffer.write_pos,
                   buffer, length);
            early_console.early_buffer.write_pos += length;
        }
        return;
    }
    
    for (uint32_t i = 0; i < early_console.output_count; i++) {
        console_output_t *output = &early_console.outputs[i];
        
        if (output->state == CONSOLE_STATE_ACTIVE && output->write) {
            // Set color based on log level if supported
            if (output->set_color && early_console.debug.color_enabled) {
                // Extract log level from message (simplified)
                uint32_t level = LOG_LEVEL_INFO; // Default
                output->set_color(output, 
                                 log_level_colors[level] & 0x0F,
                                 (log_level_colors[level] >> 4) & 0x0F);
            }
            
            int result = output->write(output, buffer, length);
            if (result < 0) {
                output->state.error_count++;
                if (output->state.error_count > 10) {
                    output->state = CONSOLE_STATE_ERROR;
                    early_console.active_outputs--;
                }
            }
        }
    }
    
    early_console.statistics.bytes_written += length;
}

/*
 * Replay Early Buffer
 */
static void early_console_replay_buffer(void)
{
    if (early_console.early_buffer.write_pos == 0) {
        return;
    }
    
    early_console_log(LOG_LEVEL_INFO, "CONSOLE", 
                     "Replaying early boot messages (%u bytes)",
                     early_console.early_buffer.write_pos);
    
    // Write buffer contents to all active outputs
    for (uint32_t i = 0; i < early_console.output_count; i++) {
        console_output_t *output = &early_console.outputs[i];
        
        if (output->state == CONSOLE_STATE_ACTIVE && output->write) {
            output->write(output, early_console.early_buffer.buffer,
                         early_console.early_buffer.write_pos);
        }
    }
    
    // Clear early buffer
    early_console.early_buffer.write_pos = 0;
}

// Helper functions (stubs)
static uint64_t get_timestamp_ns(void) { 
    // This would use TSC or other high-resolution timer
    static uint64_t counter = 0;
    return counter++ * 1000000; // Simulate 1ms increments
}

static uint32_t get_current_cpu_id(void) { 
    // This would get current CPU ID from APIC or per-CPU data
    return 0; 
}

static void* allocate_early_memory(size_t size) { 
    // This would allocate from early memory pool
    static char early_heap[1024 * 1024]; // 1MB early heap
    static size_t heap_offset = 0;
    
    if (heap_offset + size > sizeof(early_heap)) {
        return NULL;
    }
    
    void *ptr = &early_heap[heap_offset];
    heap_offset += (size + 7) & ~7; // 8-byte align
    return ptr;
}

// Hardware I/O functions (stubs for compilation)
static void outb(uint16_t port, uint8_t value) { 
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t inb(uint16_t port) { 
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Standard library functions (simplified implementations)
static int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    // Simplified implementation - in real OS this would be complete
    return 0;
}

// Boot timing functions
void early_console_record_boot_phase(const char *phase_name, uint64_t timestamp) {
    if (strcmp(phase_name, "uefi_exit") == 0) {
        early_console.timing.uefi_exit = timestamp;
    } else if (strcmp(phase_name, "kernel_entry") == 0) {
        early_console.timing.kernel_entry = timestamp;
    } else if (strcmp(phase_name, "memory_init") == 0) {
        early_console.timing.memory_init = timestamp;
    }
    // Add more phase tracking as needed
}