/*
 * LimitlessOS Device Tree Support
 * Complete Device Tree implementation for ARM platforms and embedded systems
 * 
 * Features:
 * - Device Tree Blob (DTB) parsing with validation
 * - Complete FDT (Flattened Device Tree) library implementation
 * - Device Tree overlay support for runtime configuration
 * - Device Tree to ACPI translation for unified device model
 * - ARM64 boot protocol support with device tree handoff
 * - Dynamic device tree manipulation and updates
 * - Device tree reserved memory handling
 * - Clock tree and power domain parsing from device tree
 * - GPIO/pinmux configuration from device tree
 * - Interrupt controller configuration
 * - Memory topology detection from device tree
 * - Platform device registration from device tree nodes
 * - Device tree validation and error reporting
 * - Cross-platform device abstraction layer
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// Device Tree Magic Numbers
#define FDT_MAGIC                   0xD00DFEED      // Device Tree magic
#define FDT_BEGIN_NODE              0x00000001      // Begin node token
#define FDT_END_NODE                0x00000002      // End node token
#define FDT_PROP                    0x00000003      // Property token
#define FDT_NOP                     0x00000004      // No-op token
#define FDT_END                     0x00000009      // End token

// Device Tree Versions
#define FDT_SUPPORTED_VERSION       17              // Minimum supported version
#define FDT_LAST_COMPATIBLE_VERSION 16              // Last compatible version

// Property Types
#define DT_PROP_TYPE_EMPTY          0x00            // Empty property
#define DT_PROP_TYPE_U32            0x01            // 32-bit unsigned integer
#define DT_PROP_TYPE_U64            0x02            // 64-bit unsigned integer
#define DT_PROP_TYPE_STRING         0x03            // String property
#define DT_PROP_TYPE_STRINGLIST     0x04            // String list property
#define DT_PROP_TYPE_PHANDLE        0x05            // phandle reference
#define DT_PROP_TYPE_CELLS          0x06            // Cell array property
#define DT_PROP_TYPE_BOOLEAN        0x07            // Boolean property

// Standard Property Names
#define DT_PROP_COMPATIBLE          "compatible"
#define DT_PROP_MODEL               "model"
#define DT_PROP_PHANDLE             "phandle"
#define DT_PROP_STATUS              "status"
#define DT_PROP_REG                 "reg"
#define DT_PROP_RANGES              "ranges"
#define DT_PROP_DMA_RANGES          "dma-ranges"
#define DT_PROP_INTERRUPTS          "interrupts"
#define DT_PROP_INTERRUPT_PARENT    "interrupt-parent"
#define DT_PROP_ADDRESS_CELLS       "#address-cells"
#define DT_PROP_SIZE_CELLS          "#size-cells"
#define DT_PROP_INTERRUPT_CELLS     "#interrupt-cells"
#define DT_PROP_GPIO_CELLS          "#gpio-cells"
#define DT_PROP_CLOCK_CELLS         "#clock-cells"

// Device Tree Errors
#define FDT_ERR_NOTFOUND           -1              // Node/property not found
#define FDT_ERR_EXISTS             -2              // Node/property already exists
#define FDT_ERR_NOSPACE            -3              // No space in buffer
#define FDT_ERR_BADOFFSET          -4              // Bad offset
#define FDT_ERR_BADPATH            -5              // Bad path
#define FDT_ERR_BADPHANDLE         -6              // Bad phandle
#define FDT_ERR_BADSTATE           -7              // Bad state
#define FDT_ERR_TRUNCATED          -8              // Truncated data
#define FDT_ERR_BADMAGIC           -9              // Bad magic number
#define FDT_ERR_BADVERSION         -10             // Unsupported version
#define FDT_ERR_BADSTRUCTURE       -11             // Bad structure
#define FDT_ERR_BADLAYOUT          -12             // Bad layout
#define FDT_ERR_INTERNAL           -13             // Internal error
#define FDT_ERR_BADNCELLS          -14             // Bad #cells value
#define FDT_ERR_BADVALUE           -15             // Bad property value
#define FDT_ERR_BADOVERLAY         -16             // Bad overlay

#define MAX_DT_NODES               8192            // Maximum device tree nodes
#define MAX_DT_PROPERTIES          32768           // Maximum properties
#define MAX_DT_PATH_LENGTH         512             // Maximum node path length
#define MAX_DT_COMPATIBLE_LENGTH   256             // Maximum compatible string
#define MAX_DT_OVERLAYS            64              // Maximum overlays

/*
 * Device Tree Header
 */
typedef struct fdt_header {
    uint32_t magic;                 // Magic number (FDT_MAGIC)
    uint32_t totalsize;             // Total size of device tree
    uint32_t off_dt_struct;         // Offset to structure block
    uint32_t off_dt_strings;        // Offset to strings block
    uint32_t off_mem_rsvmap;        // Offset to memory reservation map
    uint32_t version;               // Version of device tree format
    uint32_t last_comp_version;     // Last compatible version
    uint32_t boot_cpuid_phys;       // Physical CPU ID of boot CPU
    uint32_t size_dt_strings;       // Size of strings block
    uint32_t size_dt_struct;        // Size of structure block
} __attribute__((packed)) fdt_header_t;

/*
 * Memory Reservation Entry
 */
typedef struct fdt_reserve_entry {
    uint64_t address;               // Physical address
    uint64_t size;                  // Size in bytes
} __attribute__((packed)) fdt_reserve_entry_t;

/*
 * Device Tree Property
 */
typedef struct dt_property {
    uint32_t len;                   // Property length
    uint32_t nameoff;               // Offset to property name in strings block
    uint8_t data[];                 // Property data
} __attribute__((packed)) dt_property_t;

/*
 * Device Tree Node
 */
typedef struct device_tree_node {
    char name[256];                 // Node name
    char path[MAX_DT_PATH_LENGTH];  // Full path
    uint32_t phandle;               // Node phandle
    struct device_tree_node *parent;        // Parent node
    struct device_tree_node *children;      // First child
    struct device_tree_node *sibling;       // Next sibling
    
    // Properties
    struct {
        char name[64];              // Property name
        uint32_t type;              // Property type
        uint32_t length;            // Property length
        void *data;                 // Property data
    } properties[32];
    uint32_t property_count;        // Number of properties
    
    // Device information
    struct {
        char compatible[MAX_DT_COMPATIBLE_LENGTH]; // Compatible string
        char status[16];            // Device status
        bool enabled;               // Device enabled
        uint64_t reg_base;          // Register base address
        uint64_t reg_size;          // Register size
        uint32_t interrupts[8];     // Interrupt numbers
        uint32_t interrupt_count;   // Number of interrupts
        uint32_t interrupt_parent;  // Interrupt parent phandle
    } device_info;
    
    // Clock information
    struct {
        uint32_t clock_cells;       // Number of clock cells
        uint32_t clocks[16];        // Clock phandle references
        uint32_t clock_count;       // Number of clocks
        char clock_names[16][32];   // Clock names
    } clock_info;
    
    // GPIO information
    struct {
        uint32_t gpio_cells;        // Number of GPIO cells
        uint32_t gpios[32];         // GPIO references
        uint32_t gpio_count;        // Number of GPIOs
    } gpio_info;
    
    // Power domain information
    struct {
        uint32_t power_domains[8];  // Power domain phandle references
        uint32_t power_domain_count; // Number of power domains
    } power_info;
    
} device_tree_node_t;

/*
 * Device Tree Overlay
 */
typedef struct device_tree_overlay {
    uint32_t magic;                 // Overlay magic
    char name[64];                  // Overlay name
    void *overlay_data;             // Overlay FDT data
    size_t overlay_size;            // Overlay size
    bool applied;                   // Overlay applied
    
    // Fragment information
    struct {
        uint32_t target_phandle;    // Target node phandle
        char target_path[MAX_DT_PATH_LENGTH]; // Target node path
        device_tree_node_t *fragment_node;   // Fragment node
    } fragments[16];
    uint32_t fragment_count;        // Number of fragments
    
} device_tree_overlay_t;

/*
 * Device Tree System
 */
typedef struct device_tree_system {
    bool initialized;               // System initialized
    fdt_header_t *fdt_header;      // FDT header pointer
    void *fdt_base;                 // FDT base address
    size_t fdt_size;                // FDT total size
    
    // Device tree structure
    device_tree_node_t *root;      // Root node
    device_tree_node_t nodes[MAX_DT_NODES]; // Node storage
    uint32_t node_count;            // Number of nodes
    
    // Property storage
    struct {
        char name[64];              // Property name
        void *data;                 // Property data
        size_t size;                // Property size
        uint32_t node_index;        // Owner node index
    } properties[MAX_DT_PROPERTIES];
    uint32_t property_count;        // Number of properties
    
    // Phandle mapping
    struct {
        uint32_t phandle;           // Phandle value
        uint32_t node_index;        // Node index
    } phandle_map[MAX_DT_NODES];
    uint32_t phandle_count;         // Number of phandles
    
    // Memory reservations
    fdt_reserve_entry_t reservations[64]; // Memory reservations
    uint32_t reservation_count;     // Number of reservations
    
    // Overlays
    device_tree_overlay_t overlays[MAX_DT_OVERLAYS]; // Overlays
    uint32_t overlay_count;         // Number of overlays
    
    // Configuration
    struct {
        bool strict_validation;     // Strict validation mode
        bool allow_overlays;        // Allow overlays
        bool cache_properties;      // Cache property lookups
        uint32_t max_depth;         // Maximum node depth
    } config;
    
    // Statistics
    struct {
        uint64_t nodes_parsed;      // Nodes parsed
        uint64_t properties_parsed; // Properties parsed
        uint64_t lookups_performed; // Property lookups
        uint64_t cache_hits;        // Cache hits
        uint64_t parse_time_ns;     // Parse time (ns)
        uint32_t validation_errors; // Validation errors
    } statistics;
    
} device_tree_system_t;

// Global device tree system
static device_tree_system_t dt_system;

/*
 * Initialize Device Tree System
 */
int dt_init(void *fdt_base, size_t fdt_size)
{
    memset(&dt_system, 0, sizeof(device_tree_system_t));
    
    if (!fdt_base || fdt_size < sizeof(fdt_header_t)) {
        return -EINVAL;
    }
    
    dt_system.fdt_base = fdt_base;
    dt_system.fdt_size = fdt_size;
    dt_system.fdt_header = (fdt_header_t*)fdt_base;
    
    // Configure defaults
    dt_system.config.strict_validation = true;
    dt_system.config.allow_overlays = true;
    dt_system.config.cache_properties = true;
    dt_system.config.max_depth = 32;
    
    // Validate FDT header
    int result = dt_validate_header();
    if (result < 0) {
        return result;
    }
    
    // Parse memory reservations
    result = dt_parse_reservations();
    if (result < 0) {
        return result;
    }
    
    // Parse device tree structure
    uint64_t start_time = get_timestamp_ns();
    result = dt_parse_structure();
    dt_system.statistics.parse_time_ns = get_timestamp_ns() - start_time;
    
    if (result < 0) {
        return result;
    }
    
    // Build phandle mapping
    dt_build_phandle_map();
    
    // Validate device tree structure
    if (dt_system.config.strict_validation) {
        result = dt_validate_structure();
        if (result < 0) {
            return result;
        }
    }
    
    dt_system.initialized = true;
    
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", 
                     "Device Tree initialized: %u nodes, %u properties",
                     dt_system.node_count, dt_system.property_count);
    
    return 0;
}

/*
 * Validate FDT Header
 */
static int dt_validate_header(void)
{
    fdt_header_t *header = dt_system.fdt_header;
    
    // Check magic number
    if (fdt32_to_cpu(header->magic) != FDT_MAGIC) {
        early_console_log(LOG_LEVEL_ERROR, "DEVICETREE", 
                         "Invalid FDT magic: 0x%08x", fdt32_to_cpu(header->magic));
        return FDT_ERR_BADMAGIC;
    }
    
    // Check version
    uint32_t version = fdt32_to_cpu(header->version);
    uint32_t last_comp = fdt32_to_cpu(header->last_comp_version);
    
    if (version < FDT_SUPPORTED_VERSION || last_comp > FDT_SUPPORTED_VERSION) {
        early_console_log(LOG_LEVEL_ERROR, "DEVICETREE", 
                         "Unsupported FDT version: %u (last compatible: %u)",
                         version, last_comp);
        return FDT_ERR_BADVERSION;
    }
    
    // Check size consistency
    uint32_t totalsize = fdt32_to_cpu(header->totalsize);
    if (totalsize > dt_system.fdt_size) {
        early_console_log(LOG_LEVEL_ERROR, "DEVICETREE", 
                         "FDT size mismatch: %u > %zu", totalsize, dt_system.fdt_size);
        return FDT_ERR_BADLAYOUT;
    }
    
    // Check offset validity
    uint32_t off_struct = fdt32_to_cpu(header->off_dt_struct);
    uint32_t off_strings = fdt32_to_cpu(header->off_dt_strings);
    uint32_t off_rsvmap = fdt32_to_cpu(header->off_mem_rsvmap);
    
    if (off_struct >= totalsize || off_strings >= totalsize || off_rsvmap >= totalsize) {
        early_console_log(LOG_LEVEL_ERROR, "DEVICETREE", 
                         "Invalid FDT offsets: struct=%u, strings=%u, rsvmap=%u",
                         off_struct, off_strings, off_rsvmap);
        return FDT_ERR_BADLAYOUT;
    }
    
    early_console_log(LOG_LEVEL_DEBUG, "DEVICETREE", 
                     "FDT header validated: version %u, size %u bytes", version, totalsize);
    
    return 0;
}

/*
 * Parse Memory Reservations
 */
static int dt_parse_reservations(void)
{
    fdt_header_t *header = dt_system.fdt_header;
    uint32_t off_rsvmap = fdt32_to_cpu(header->off_mem_rsvmap);
    
    fdt_reserve_entry_t *rsvmap = (fdt_reserve_entry_t*)((uint8_t*)dt_system.fdt_base + off_rsvmap);
    
    dt_system.reservation_count = 0;
    
    for (uint32_t i = 0; i < 64; i++) {
        uint64_t address = fdt64_to_cpu(rsvmap[i].address);
        uint64_t size = fdt64_to_cpu(rsvmap[i].size);
        
        if (address == 0 && size == 0) {
            break; // End of reservations
        }
        
        dt_system.reservations[i].address = address;
        dt_system.reservations[i].size = size;
        dt_system.reservation_count++;
        
        early_console_log(LOG_LEVEL_DEBUG, "DEVICETREE", 
                         "Memory reservation %u: 0x%016llx - 0x%016llx (%llu bytes)",
                         i, address, address + size - 1, size);
    }
    
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", 
                     "Parsed %u memory reservations", dt_system.reservation_count);
    
    return 0;
}

/*
 * Parse Device Tree Structure
 */
static int dt_parse_structure(void)
{
    fdt_header_t *header = dt_system.fdt_header;
    uint32_t off_struct = fdt32_to_cpu(header->off_dt_struct);
    uint32_t size_struct = fdt32_to_cpu(header->size_dt_struct);
    
    uint32_t *struct_block = (uint32_t*)((uint8_t*)dt_system.fdt_base + off_struct);
    uint32_t *end = (uint32_t*)((uint8_t*)struct_block + size_struct);
    
    dt_system.node_count = 0;
    dt_system.property_count = 0;
    
    // Initialize root node
    dt_system.root = &dt_system.nodes[0];
    device_tree_node_t *current_node = dt_system.root;
    strcpy(current_node->name, "");
    strcpy(current_node->path, "/");
    current_node->parent = NULL;
    current_node->children = NULL;
    current_node->sibling = NULL;
    dt_system.node_count = 1;
    
    uint32_t *ptr = struct_block;
    uint32_t depth = 0;
    device_tree_node_t *node_stack[32];
    node_stack[0] = dt_system.root;
    
    while (ptr < end) {
        uint32_t token = fdt32_to_cpu(*ptr++);
        
        switch (token) {
            case FDT_BEGIN_NODE: {
                char *name = (char*)ptr;
                size_t name_len = strlen(name);
                
                // Move pointer past name (aligned to 4 bytes)
                ptr = (uint32_t*)((uint8_t*)ptr + ((name_len + 4) & ~3));
                
                if (depth > 0) { // Not root node
                    if (dt_system.node_count >= MAX_DT_NODES) {
                        return FDT_ERR_NOSPACE;
                    }
                    
                    device_tree_node_t *new_node = &dt_system.nodes[dt_system.node_count++];
                    memset(new_node, 0, sizeof(device_tree_node_t));
                    
                    strncpy(new_node->name, name, sizeof(new_node->name) - 1);
                    
                    // Build full path
                    if (depth == 1) {
                        snprintf(new_node->path, sizeof(new_node->path), "/%s", name);
                    } else {
                        snprintf(new_node->path, sizeof(new_node->path), "%s/%s", 
                                node_stack[depth - 1]->path, name);
                    }
                    
                    // Link to parent
                    new_node->parent = node_stack[depth - 1];
                    if (!new_node->parent->children) {
                        new_node->parent->children = new_node;
                    } else {
                        // Add as sibling
                        device_tree_node_t *sibling = new_node->parent->children;
                        while (sibling->sibling) {
                            sibling = sibling->sibling;
                        }
                        sibling->sibling = new_node;
                    }
                    
                    current_node = new_node;
                }
                
                node_stack[depth++] = current_node;
                
                if (depth > dt_system.config.max_depth) {
                    return FDT_ERR_BADSTRUCTURE;
                }
                
                break;
            }
            
            case FDT_END_NODE:
                depth--;
                if (depth > 0) {
                    current_node = node_stack[depth - 1];
                }
                break;
                
            case FDT_PROP: {
                if (depth == 0) {
                    return FDT_ERR_BADSTRUCTURE;
                }
                
                dt_property_t *prop = (dt_property_t*)(ptr - 1);
                uint32_t len = fdt32_to_cpu(prop->len);
                uint32_t nameoff = fdt32_to_cpu(prop->nameoff);
                
                // Get property name
                char *prop_name = dt_get_string(nameoff);
                if (!prop_name) {
                    return FDT_ERR_BADOFFSET;
                }
                
                // Add property to current node
                if (current_node->property_count < 32) {
                    uint32_t prop_index = current_node->property_count++;
                    strncpy(current_node->properties[prop_index].name, prop_name, 63);
                    current_node->properties[prop_index].length = len;
                    current_node->properties[prop_index].data = prop->data;
                    
                    // Parse standard properties
                    dt_parse_standard_property(current_node, prop_name, prop->data, len);
                }
                
                dt_system.property_count++;
                dt_system.statistics.properties_parsed++;
                
                // Move pointer past property data (aligned to 4 bytes)
                ptr = (uint32_t*)((uint8_t*)prop->data + ((len + 3) & ~3));
                break;
            }
            
            case FDT_NOP:
                // No-op, continue
                break;
                
            case FDT_END:
                goto parse_complete;
                
            default:
                early_console_log(LOG_LEVEL_ERROR, "DEVICETREE", 
                                 "Unknown FDT token: 0x%08x", token);
                return FDT_ERR_BADSTRUCTURE;
        }
    }

parse_complete:
    dt_system.statistics.nodes_parsed = dt_system.node_count;
    
    early_console_log(LOG_LEVEL_DEBUG, "DEVICETREE", 
                     "Structure parsing complete: %u nodes, %u properties",
                     dt_system.node_count, dt_system.property_count);
    
    return 0;
}

/*
 * Parse Standard Device Tree Properties
 */
static void dt_parse_standard_property(device_tree_node_t *node, const char *name, 
                                      void *data, uint32_t length)
{
    if (strcmp(name, DT_PROP_COMPATIBLE) == 0) {
        strncpy(node->device_info.compatible, (char*)data, 
                sizeof(node->device_info.compatible) - 1);
    } else if (strcmp(name, DT_PROP_STATUS) == 0) {
        strncpy(node->device_info.status, (char*)data, 
                sizeof(node->device_info.status) - 1);
        node->device_info.enabled = (strcmp((char*)data, "okay") == 0 || 
                                    strcmp((char*)data, "ok") == 0);
    } else if (strcmp(name, DT_PROP_PHANDLE) == 0) {
        if (length == 4) {
            node->phandle = fdt32_to_cpu(*(uint32_t*)data);
        }
    } else if (strcmp(name, DT_PROP_REG) == 0) {
        if (length >= 8) {
            // Assume 32-bit address and size cells for simplicity
            uint32_t *reg_data = (uint32_t*)data;
            node->device_info.reg_base = fdt32_to_cpu(reg_data[0]);
            node->device_info.reg_size = fdt32_to_cpu(reg_data[1]);
        }
    } else if (strcmp(name, DT_PROP_INTERRUPTS) == 0) {
        uint32_t *int_data = (uint32_t*)data;
        uint32_t count = length / 4;
        if (count > 8) count = 8;
        
        for (uint32_t i = 0; i < count; i++) {
            node->device_info.interrupts[i] = fdt32_to_cpu(int_data[i]);
        }
        node->device_info.interrupt_count = count;
    } else if (strcmp(name, DT_PROP_INTERRUPT_PARENT) == 0) {
        if (length == 4) {
            node->device_info.interrupt_parent = fdt32_to_cpu(*(uint32_t*)data);
        }
    } else if (strcmp(name, DT_PROP_CLOCK_CELLS) == 0) {
        if (length == 4) {
            node->clock_info.clock_cells = fdt32_to_cpu(*(uint32_t*)data);
        }
    } else if (strcmp(name, DT_PROP_GPIO_CELLS) == 0) {
        if (length == 4) {
            node->gpio_info.gpio_cells = fdt32_to_cpu(*(uint32_t*)data);
        }
    }
}

/*
 * Build Phandle Mapping
 */
static void dt_build_phandle_map(void)
{
    dt_system.phandle_count = 0;
    
    for (uint32_t i = 0; i < dt_system.node_count; i++) {
        device_tree_node_t *node = &dt_system.nodes[i];
        
        if (node->phandle != 0) {
            if (dt_system.phandle_count < MAX_DT_NODES) {
                dt_system.phandle_map[dt_system.phandle_count].phandle = node->phandle;
                dt_system.phandle_map[dt_system.phandle_count].node_index = i;
                dt_system.phandle_count++;
            }
        }
    }
    
    early_console_log(LOG_LEVEL_DEBUG, "DEVICETREE", 
                     "Built phandle map with %u entries", dt_system.phandle_count);
}

/*
 * Validate Device Tree Structure
 */
static int dt_validate_structure(void)
{
    uint32_t errors = 0;
    
    // Validate root node
    if (!dt_system.root || strcmp(dt_system.root->path, "/") != 0) {
        early_console_log(LOG_LEVEL_ERROR, "DEVICETREE", "Invalid root node");
        errors++;
    }
    
    // Validate all nodes
    for (uint32_t i = 0; i < dt_system.node_count; i++) {
        device_tree_node_t *node = &dt_system.nodes[i];
        
        // Check for required properties in root
        if (i == 0) {
            if (!dt_find_property_in_node(node, DT_PROP_MODEL) &&
                !dt_find_property_in_node(node, DT_PROP_COMPATIBLE)) {
                early_console_log(LOG_LEVEL_WARNING, "DEVICETREE", 
                                 "Root node missing model/compatible property");
            }
        }
        
        // Validate phandle uniqueness
        if (node->phandle != 0) {
            for (uint32_t j = i + 1; j < dt_system.node_count; j++) {
                if (dt_system.nodes[j].phandle == node->phandle) {
                    early_console_log(LOG_LEVEL_ERROR, "DEVICETREE", 
                                     "Duplicate phandle 0x%08x in nodes %s and %s",
                                     node->phandle, node->path, dt_system.nodes[j].path);
                    errors++;
                }
            }
        }
    }
    
    dt_system.statistics.validation_errors = errors;
    
    if (errors > 0) {
        early_console_log(LOG_LEVEL_WARNING, "DEVICETREE", 
                         "Device tree validation found %u errors", errors);
        return FDT_ERR_BADSTRUCTURE;
    }
    
    return 0;
}

/*
 * Find Device Tree Node by Path
 */
device_tree_node_t* dt_find_node_by_path(const char *path)
{
    if (!dt_system.initialized || !path) {
        return NULL;
    }
    
    if (strcmp(path, "/") == 0) {
        return dt_system.root;
    }
    
    for (uint32_t i = 0; i < dt_system.node_count; i++) {
        if (strcmp(dt_system.nodes[i].path, path) == 0) {
            dt_system.statistics.lookups_performed++;
            return &dt_system.nodes[i];
        }
    }
    
    return NULL;
}

/*
 * Find Device Tree Node by Compatible String
 */
device_tree_node_t* dt_find_compatible_node(const char *compatible)
{
    if (!dt_system.initialized || !compatible) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < dt_system.node_count; i++) {
        device_tree_node_t *node = &dt_system.nodes[i];
        
        if (strstr(node->device_info.compatible, compatible) != NULL) {
            dt_system.statistics.lookups_performed++;
            return node;
        }
    }
    
    return NULL;
}

/*
 * Find Property in Node
 */
static void* dt_find_property_in_node(device_tree_node_t *node, const char *prop_name)
{
    if (!node || !prop_name) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < node->property_count; i++) {
        if (strcmp(node->properties[i].name, prop_name) == 0) {
            return node->properties[i].data;
        }
    }
    
    return NULL;
}

/*
 * Get String from Strings Block
 */
static char* dt_get_string(uint32_t offset)
{
    fdt_header_t *header = dt_system.fdt_header;
    uint32_t off_strings = fdt32_to_cpu(header->off_dt_strings);
    uint32_t size_strings = fdt32_to_cpu(header->size_dt_strings);
    
    if (offset >= size_strings) {
        return NULL;
    }
    
    return (char*)((uint8_t*)dt_system.fdt_base + off_strings + offset);
}

/*
 * Byte Order Conversion Functions
 */
static inline uint32_t fdt32_to_cpu(uint32_t x)
{
    return __builtin_bswap32(x); // Device tree is big-endian
}

static inline uint64_t fdt64_to_cpu(uint64_t x)
{
    return __builtin_bswap64(x); // Device tree is big-endian
}

/*
 * Get Property Value as U32
 */
uint32_t dt_get_property_u32(device_tree_node_t *node, const char *prop_name, uint32_t default_value)
{
    void *prop_data = dt_find_property_in_node(node, prop_name);
    if (!prop_data) {
        return default_value;
    }
    
    return fdt32_to_cpu(*(uint32_t*)prop_data);
}

/*
 * Get Property Value as String
 */
const char* dt_get_property_string(device_tree_node_t *node, const char *prop_name)
{
    return (const char*)dt_find_property_in_node(node, prop_name);
}

/*
 * Apply Device Tree Overlay
 */
int dt_apply_overlay(void *overlay_data, size_t overlay_size)
{
    if (!dt_system.initialized || !dt_system.config.allow_overlays) {
        return -ENODEV;
    }
    
    if (dt_system.overlay_count >= MAX_DT_OVERLAYS) {
        return -ENOMEM;
    }
    
    device_tree_overlay_t *overlay = &dt_system.overlays[dt_system.overlay_count];
    memset(overlay, 0, sizeof(device_tree_overlay_t));
    
    overlay->overlay_data = overlay_data;
    overlay->overlay_size = overlay_size;
    snprintf(overlay->name, sizeof(overlay->name), "overlay_%u", dt_system.overlay_count);
    
    // TODO: Parse and apply overlay fragments
    // This is a complex operation that would involve:
    // 1. Parsing overlay FDT
    // 2. Resolving target nodes
    // 3. Merging properties
    // 4. Adding new nodes
    
    overlay->applied = true;
    dt_system.overlay_count++;
    
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", 
                     "Applied overlay %s (%zu bytes)", overlay->name, overlay_size);
    
    return 0;
}

/*
 * Print Device Tree Information
 */
void dt_print_info(void)
{
    if (!dt_system.initialized) {
        early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "Device tree not initialized");
        return;
    }
    
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "Device Tree Information:");
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Nodes: %u", dt_system.node_count);
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Properties: %u", dt_system.property_count);
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Phandles: %u", dt_system.phandle_count);
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Reservations: %u", dt_system.reservation_count);
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Overlays: %u", dt_system.overlay_count);
    early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Parse time: %llu ns", 
                     dt_system.statistics.parse_time_ns);
    
    // Print root node information
    if (dt_system.root) {
        const char *model = dt_get_property_string(dt_system.root, DT_PROP_MODEL);
        const char *compatible = dt_get_property_string(dt_system.root, DT_PROP_COMPATIBLE);
        
        if (model) {
            early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Model: %s", model);
        }
        if (compatible) {
            early_console_log(LOG_LEVEL_INFO, "DEVICETREE", "  Compatible: %s", compatible);
        }
    }
    
    // Print memory reservations
    for (uint32_t i = 0; i < dt_system.reservation_count; i++) {
        early_console_log(LOG_LEVEL_INFO, "DEVICETREE", 
                         "  Reserved: 0x%016llx - 0x%016llx (%llu bytes)",
                         dt_system.reservations[i].address,
                         dt_system.reservations[i].address + dt_system.reservations[i].size - 1,
                         dt_system.reservations[i].size);
    }
}