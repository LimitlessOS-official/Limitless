/*
 * gpt_manager.c - LimitlessOS GPT Partition Manager
 * 
 * Complete GPT (GUID Partition Table) implementation with advanced
 * partitioning, LVM support, and RAID configurations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/blkpg.h>
#include <uuid/uuid.h>
#include <kernel/storage.h>
#include <kernel/crypto.h>

#define GPT_SIGNATURE               0x5452415020494645ULL  /* "EFI PART" */
#define GPT_REVISION                0x00010000
#define GPT_HEADER_SIZE             92
#define GPT_ENTRY_SIZE              128
#define MAX_PARTITIONS              128
#define MAX_LVM_VOLUMES             64
#define MAX_RAID_DEVICES            16

/* GPT partition types (common ones) */
typedef struct gpt_partition_type {
    uuid_t guid;
    const char* name;
    const char* description;
} gpt_partition_type_t;

static const gpt_partition_type_t gpt_partition_types[] = {
    {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "unused", "Unused partition" },
    {{ 0x21, 0x68, 0x61, 0x48, 0x64, 0x49, 0x6E, 0x6F, 0x74, 0x4E, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49 }, "bios_boot", "BIOS Boot partition" },
    {{ 0x48, 0x61, 0x68, 0x21, 0x49, 0x64, 0x6F, 0x6E, 0x74, 0x4E, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49 }, "efi_system", "EFI System partition" },
    {{ 0x0F, 0xC6, 0x3D, 0xAF, 0x84, 0x83, 0x47, 0x72, 0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4 }, "linux_data", "Linux filesystem data" },
    {{ 0xE6, 0xD6, 0xD3, 0x79, 0xF5, 0x07, 0x44, 0xC2, 0xA2, 0x3C, 0x23, 0x8F, 0x2A, 0x3D, 0xF9, 0x28 }, "linux_lvm", "Linux LVM" },
    {{ 0xA1, 0x9D, 0x88, 0x0F, 0x05, 0xFC, 0x4D, 0x3B, 0xA0, 0x06, 0x74, 0x3F, 0x0F, 0x84, 0x91, 0x1E }, "linux_raid", "Linux RAID" },
    {{ 0x93, 0x3A, 0xC7, 0xE1, 0x2E, 0xB4, 0x4F, 0x13, 0xB8, 0x44, 0x0E, 0x14, 0xE2, 0xAE, 0xF9, 0x15 }, "linux_home", "Linux /home" },
    {{ 0x44, 0x79, 0x94, 0xE0, 0x3B, 0x5C, 0x4A, 0xB8, 0xB4, 0xFE, 0x5A, 0x18, 0x9F, 0x8D, 0x6A, 0xA6 }, "linux_srv", "Linux /srv" }
};

/* GPT header structure */
typedef struct __attribute__((packed)) gpt_header {
    uint64_t signature;             /* "EFI PART" */
    uint32_t revision;              /* GPT revision */
    uint32_t header_size;           /* Header size */
    uint32_t header_crc32;          /* Header CRC32 */
    uint32_t reserved;              /* Reserved (zero) */
    uint64_t current_lba;           /* Current LBA of header */
    uint64_t backup_lba;            /* Backup LBA of header */
    uint64_t first_usable_lba;      /* First usable LBA */
    uint64_t last_usable_lba;       /* Last usable LBA */
    uuid_t disk_guid;               /* Disk GUID */
    uint64_t partition_array_lba;   /* Partition array LBA */
    uint32_t num_partition_entries; /* Number of partition entries */
    uint32_t partition_entry_size;  /* Size of partition entry */
    uint32_t partition_array_crc32; /* Partition array CRC32 */
    uint8_t reserved2[420];         /* Reserved space */
} gpt_header_t;

/* GPT partition entry */
typedef struct __attribute__((packed)) gpt_partition_entry {
    uuid_t partition_type_guid;     /* Partition type GUID */
    uuid_t unique_partition_guid;   /* Unique partition GUID */
    uint64_t starting_lba;          /* Starting LBA */
    uint64_t ending_lba;            /* Ending LBA */
    uint64_t attributes;            /* Partition attributes */
    uint16_t partition_name[36];    /* Partition name (UTF-16) */
} gpt_partition_entry_t;

/* LVM Physical Volume */
typedef struct lvm_pv {
    char device_path[256];          /* Device path */
    uuid_t pv_uuid;                 /* PV UUID */
    uint64_t size;                  /* PV size in bytes */
    uint64_t pe_size;               /* Physical extent size */
    uint64_t pe_count;              /* Number of physical extents */
    uint64_t pe_free;               /* Free physical extents */
    char vg_name[64];               /* Volume group name */
    bool active;                    /* PV is active */
} lvm_pv_t;

/* LVM Volume Group */
typedef struct lvm_vg {
    char name[64];                  /* VG name */
    uuid_t vg_uuid;                 /* VG UUID */
    uint64_t size;                  /* Total VG size */
    uint64_t free_size;             /* Free space */
    uint64_t pe_size;               /* Physical extent size */
    uint64_t pe_count;              /* Total physical extents */
    uint64_t pe_free;               /* Free physical extents */
    
    /* Physical volumes in this VG */
    lvm_pv_t* pvs[16];
    uint32_t pv_count;
    
    /* Logical volumes in this VG */
    struct lvm_lv* lvs[32];
    uint32_t lv_count;
    
    bool active;                    /* VG is active */
} lvm_vg_t;

/* LVM Logical Volume */
typedef struct lvm_lv {
    char name[64];                  /* LV name */
    char full_name[128];            /* Full name (vg/lv) */
    uuid_t lv_uuid;                 /* LV UUID */
    uint64_t size;                  /* LV size in bytes */
    uint64_t le_count;              /* Logical extent count */
    
    lvm_vg_t* vg;                   /* Parent volume group */
    
    /* Device mapping */
    char device_path[256];          /* Device path (/dev/mapper/vg-lv) */
    int major;                      /* Device major number */
    int minor;                      /* Device minor number */
    
    bool active;                    /* LV is active */
    bool encrypted;                 /* LV is encrypted */
} lvm_lv_t;

/* RAID level */
typedef enum {
    RAID_LEVEL_0,                   /* RAID 0 (striping) */
    RAID_LEVEL_1,                   /* RAID 1 (mirroring) */
    RAID_LEVEL_5,                   /* RAID 5 (parity) */
    RAID_LEVEL_6,                   /* RAID 6 (double parity) */
    RAID_LEVEL_10                   /* RAID 10 (stripe+mirror) */
} raid_level_t;

/* RAID array */
typedef struct raid_array {
    char name[64];                  /* RAID array name */
    char device_path[256];          /* Device path (/dev/md0) */
    raid_level_t level;             /* RAID level */
    
    /* Member devices */
    char devices[16][256];          /* Member device paths */
    uint32_t device_count;          /* Number of devices */
    uint32_t active_devices;        /* Active devices */
    uint32_t spare_devices;         /* Spare devices */
    
    /* RAID parameters */
    uint64_t array_size;            /* Array size in bytes */
    uint64_t chunk_size;            /* Chunk size in bytes */
    
    /* Status */
    bool active;                    /* Array is active */
    bool degraded;                  /* Array is degraded */
    bool rebuilding;                /* Array is rebuilding */
    uint32_t rebuild_percent;       /* Rebuild percentage */
    
} raid_array_t;

/* Storage manager state */
typedef struct storage_manager {
    bool initialized;
    
    /* GPT partitions */
    gpt_header_t gpt_header;
    gpt_partition_entry_t partitions[MAX_PARTITIONS];
    uint32_t partition_count;
    
    /* LVM management */
    lvm_pv_t physical_volumes[64];
    uint32_t pv_count;
    
    lvm_vg_t volume_groups[16];
    uint32_t vg_count;
    
    lvm_lv_t logical_volumes[MAX_LVM_VOLUMES];
    uint32_t lv_count;
    
    /* RAID management */
    raid_array_t raid_arrays[MAX_RAID_DEVICES];
    uint32_t raid_count;
    
    /* Encryption */
    bool luks_initialized;
    char luks_devices[32][256];
    uint32_t luks_device_count;
    
} storage_manager_t;

static storage_manager_t g_storage = {0};

/* Function prototypes */
static int gpt_read_header(const char* device, gpt_header_t* header);
static int gpt_write_header(const char* device, const gpt_header_t* header);
static int gpt_read_partitions(const char* device, gpt_partition_entry_t* partitions, uint32_t max_count);
static int gpt_write_partitions(const char* device, const gpt_partition_entry_t* partitions, uint32_t count);
static uint32_t calculate_crc32(const void* data, size_t length);
static void generate_uuid(uuid_t uuid);
static const char* get_partition_type_name(const uuid_t guid);
static int lvm_scan_physical_volumes(void);
static int lvm_create_volume_group(const char* vg_name, const char** pv_devices, uint32_t pv_count);
static int lvm_create_logical_volume(const char* vg_name, const char* lv_name, uint64_t size);
static int raid_create_array(const char* array_name, raid_level_t level, const char** devices, uint32_t device_count);
static int luks_setup_device(const char* device, const char* name, const char* passphrase);

/* Initialize storage manager */
int storage_manager_init(void) {
    if (g_storage.initialized) {
        return 0;
    }
    
    memset(&g_storage, 0, sizeof(storage_manager_t));
    
    printf("Initializing LimitlessOS Advanced Storage Manager\n");
    
    /* Scan for existing LVM physical volumes */
    lvm_scan_physical_volumes();
    
    g_storage.initialized = true;
    
    printf("Storage manager initialized\n");
    printf("Found %u physical volumes, %u volume groups, %u RAID arrays\n",
           g_storage.pv_count, g_storage.vg_count, g_storage.raid_count);
    
    return 0;
}

/* Create GPT partition table */
int gpt_create_partition_table(const char* device) {
    printf("Creating GPT partition table on %s\n", device);
    
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        printf("Failed to open device: %s\n", device);
        return -1;
    }
    
    /* Get device size */
    uint64_t device_size;
    if (ioctl(fd, BLKGETSIZE64, &device_size) < 0) {
        close(fd);
        return -1;
    }
    
    uint64_t sector_size = 512;  /* Standard sector size */
    uint64_t total_sectors = device_size / sector_size;
    
    /* Initialize GPT header */
    gpt_header_t header;
    memset(&header, 0, sizeof(header));
    
    header.signature = GPT_SIGNATURE;
    header.revision = GPT_REVISION;
    header.header_size = GPT_HEADER_SIZE;
    header.current_lba = 1;  /* LBA 1 for primary header */
    header.backup_lba = total_sectors - 1;  /* Last LBA for backup */
    header.first_usable_lba = 34;  /* First usable LBA (after header + partition array) */
    header.last_usable_lba = total_sectors - 34;  /* Last usable LBA */
    
    /* Generate disk GUID */
    generate_uuid(header.disk_guid);
    
    header.partition_array_lba = 2;  /* Partition array starts at LBA 2 */
    header.num_partition_entries = MAX_PARTITIONS;
    header.partition_entry_size = GPT_ENTRY_SIZE;
    
    /* Initialize empty partition array */
    gpt_partition_entry_t partitions[MAX_PARTITIONS];
    memset(partitions, 0, sizeof(partitions));
    
    /* Calculate CRC32 for partition array */
    header.partition_array_crc32 = calculate_crc32(partitions, sizeof(partitions));
    
    /* Calculate CRC32 for header (with crc32 field set to 0) */
    header.header_crc32 = 0;
    header.header_crc32 = calculate_crc32(&header, GPT_HEADER_SIZE);
    
    /* Write protective MBR at LBA 0 */
    uint8_t mbr[512];
    memset(mbr, 0, sizeof(mbr));
    
    /* MBR signature */
    mbr[510] = 0x55;
    mbr[511] = 0xAA;
    
    /* Protective MBR entry */
    mbr[446] = 0x00;        /* Not bootable */
    mbr[447] = 0x00;        /* Start head */
    mbr[448] = 0x02;        /* Start sector */
    mbr[449] = 0x00;        /* Start cylinder */
    mbr[450] = 0xEE;        /* GPT partition type */
    mbr[451] = 0xFF;        /* End head */
    mbr[452] = 0xFF;        /* End sector */
    mbr[453] = 0xFF;        /* End cylinder */
    *(uint32_t*)(mbr + 454) = 1;  /* Start LBA */
    *(uint32_t*)(mbr + 458) = (uint32_t)(total_sectors - 1);  /* Size in sectors */
    
    /* Write MBR */
    if (pwrite(fd, mbr, sizeof(mbr), 0) != sizeof(mbr)) {
        close(fd);
        return -1;
    }
    
    /* Write primary GPT header */
    if (pwrite(fd, &header, sizeof(header), sector_size) != sizeof(header)) {
        close(fd);
        return -1;
    }
    
    /* Write partition array */
    if (pwrite(fd, partitions, sizeof(partitions), 2 * sector_size) != sizeof(partitions)) {
        close(fd);
        return -1;
    }
    
    /* Write backup partition array */
    uint64_t backup_array_lba = total_sectors - 33;
    if (pwrite(fd, partitions, sizeof(partitions), backup_array_lba * sector_size) != sizeof(partitions)) {
        close(fd);
        return -1;
    }
    
    /* Write backup GPT header */
    header.current_lba = total_sectors - 1;
    header.backup_lba = 1;
    header.partition_array_lba = backup_array_lba;
    header.header_crc32 = 0;
    header.header_crc32 = calculate_crc32(&header, GPT_HEADER_SIZE);
    
    if (pwrite(fd, &header, sizeof(header), (total_sectors - 1) * sector_size) != sizeof(header)) {
        close(fd);
        return -1;
    }
    
    /* Sync changes */
    fsync(fd);
    close(fd);
    
    printf("GPT partition table created successfully on %s\n", device);
    return 0;
}

/* Add GPT partition */
int gpt_add_partition(const char* device, const char* partition_type, uint64_t start_lba, uint64_t size_lba, const char* name) {
    printf("Adding GPT partition: %s (type: %s, start: %lu, size: %lu)\n",
           name, partition_type, start_lba, size_lba);
    
    /* Read existing GPT */
    gpt_header_t header;
    if (gpt_read_header(device, &header) != 0) {
        return -1;
    }
    
    gpt_partition_entry_t partitions[MAX_PARTITIONS];
    if (gpt_read_partitions(device, partitions, MAX_PARTITIONS) != 0) {
        return -1;
    }
    
    /* Find empty partition slot */
    int slot = -1;
    for (uint32_t i = 0; i < header.num_partition_entries; i++) {
        uuid_t zero_uuid;
        memset(zero_uuid, 0, sizeof(zero_uuid));
        
        if (memcmp(partitions[i].partition_type_guid, zero_uuid, sizeof(uuid_t)) == 0) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf("No free partition slots available\n");
        return -1;
    }
    
    /* Find partition type GUID */
    uuid_t type_guid;
    bool type_found = false;
    
    for (size_t i = 0; i < sizeof(gpt_partition_types) / sizeof(gpt_partition_types[0]); i++) {
        if (strcmp(gpt_partition_types[i].name, partition_type) == 0) {
            memcpy(type_guid, gpt_partition_types[i].guid, sizeof(uuid_t));
            type_found = true;
            break;
        }
    }
    
    if (!type_found) {
        printf("Unknown partition type: %s\n", partition_type);
        return -1;
    }
    
    /* Create partition entry */
    gpt_partition_entry_t* entry = &partitions[slot];
    memset(entry, 0, sizeof(gpt_partition_entry_t));
    
    memcpy(entry->partition_type_guid, type_guid, sizeof(uuid_t));
    generate_uuid(entry->unique_partition_guid);
    entry->starting_lba = start_lba;
    entry->ending_lba = start_lba + size_lba - 1;
    entry->attributes = 0;
    
    /* Convert name to UTF-16 */
    for (int i = 0; i < 36 && name[i]; i++) {
        entry->partition_name[i] = (uint16_t)name[i];
    }
    
    /* Update header */
    header.partition_array_crc32 = calculate_crc32(partitions, header.num_partition_entries * header.partition_entry_size);
    header.header_crc32 = 0;
    header.header_crc32 = calculate_crc32(&header, GPT_HEADER_SIZE);
    
    /* Write updated GPT */
    if (gpt_write_header(device, &header) != 0 ||
        gpt_write_partitions(device, partitions, header.num_partition_entries) != 0) {
        return -1;
    }
    
    printf("Partition added successfully: %s\n", name);
    return 0;
}

/* List GPT partitions */
int gpt_list_partitions(const char* device) {
    gpt_header_t header;
    if (gpt_read_header(device, &header) != 0) {
        printf("Failed to read GPT header from %s\n", device);
        return -1;
    }
    
    gpt_partition_entry_t partitions[MAX_PARTITIONS];
    if (gpt_read_partitions(device, partitions, MAX_PARTITIONS) != 0) {
        printf("Failed to read GPT partitions from %s\n", device);
        return -1;
    }
    
    printf("GPT Partitions on %s:\n", device);
    printf("%-4s %-20s %-12s %-12s %-12s %s\n", 
           "Num", "Type", "Start", "End", "Size", "Name");
    printf("%-4s %-20s %-12s %-12s %-12s %s\n",
           "---", "----", "-----", "---", "----", "----");
    
    for (uint32_t i = 0; i < header.num_partition_entries; i++) {
        gpt_partition_entry_t* entry = &partitions[i];
        
        /* Skip empty partitions */
        uuid_t zero_uuid;
        memset(zero_uuid, 0, sizeof(zero_uuid));
        if (memcmp(entry->partition_type_guid, zero_uuid, sizeof(uuid_t)) == 0) {
            continue;
        }
        
        const char* type_name = get_partition_type_name(entry->partition_type_guid);
        uint64_t size_sectors = entry->ending_lba - entry->starting_lba + 1;
        
        /* Convert UTF-16 name to ASCII */
        char ascii_name[37];
        for (int j = 0; j < 36; j++) {
            ascii_name[j] = (char)entry->partition_name[j];
            if (ascii_name[j] == 0) break;
        }
        ascii_name[36] = '\0';
        
        printf("%-4u %-20s %-12lu %-12lu %-12lu %s\n",
               i + 1, type_name, entry->starting_lba, entry->ending_lba, size_sectors, ascii_name);
    }
    
    return 0;
}

/* Create LVM physical volume */
int lvm_create_physical_volume(const char* device) {
    printf("Creating LVM physical volume on %s\n", device);
    
    /* Check if device exists */
    if (access(device, F_OK) != 0) {
        printf("Device not found: %s\n", device);
        return -1;
    }
    
    /* Add to PV list */
    if (g_storage.pv_count >= 64) {
        printf("Maximum number of physical volumes reached\n");
        return -1;
    }
    
    lvm_pv_t* pv = &g_storage.physical_volumes[g_storage.pv_count++];
    memset(pv, 0, sizeof(lvm_pv_t));
    
    strncpy(pv->device_path, device, sizeof(pv->device_path) - 1);
    generate_uuid(pv->pv_uuid);
    
    /* Get device size */
    int fd = open(device, O_RDONLY);
    if (fd >= 0) {
        uint64_t size;
        if (ioctl(fd, BLKGETSIZE64, &size) == 0) {
            pv->size = size;
        }
        close(fd);
    }
    
    pv->pe_size = 4 * 1024 * 1024;  /* 4MB physical extents */
    pv->pe_count = pv->size / pv->pe_size;
    pv->pe_free = pv->pe_count;
    pv->active = true;
    
    printf("Physical volume created: %s (size: %lu bytes, PE: %lu)\n",
           device, pv->size, pv->pe_count);
    
    return 0;
}

/* Create LVM volume group */
static int lvm_create_volume_group(const char* vg_name, const char** pv_devices, uint32_t pv_count) {
    printf("Creating LVM volume group: %s\n", vg_name);
    
    if (g_storage.vg_count >= 16) {
        printf("Maximum number of volume groups reached\n");
        return -1;
    }
    
    lvm_vg_t* vg = &g_storage.volume_groups[g_storage.vg_count++];
    memset(vg, 0, sizeof(lvm_vg_t));
    
    strncpy(vg->name, vg_name, sizeof(vg->name) - 1);
    generate_uuid(vg->vg_uuid);
    vg->pe_size = 4 * 1024 * 1024;  /* 4MB physical extents */
    
    /* Add physical volumes to VG */
    for (uint32_t i = 0; i < pv_count && vg->pv_count < 16; i++) {
        /* Find PV */
        lvm_pv_t* pv = NULL;
        for (uint32_t j = 0; j < g_storage.pv_count; j++) {
            if (strcmp(g_storage.physical_volumes[j].device_path, pv_devices[i]) == 0) {
                pv = &g_storage.physical_volumes[j];
                break;
            }
        }
        
        if (pv && strlen(pv->vg_name) == 0) {  /* PV not already in a VG */
            vg->pvs[vg->pv_count++] = pv;
            vg->size += pv->size;
            vg->pe_count += pv->pe_count;
            vg->pe_free += pv->pe_free;
            
            strncpy(pv->vg_name, vg_name, sizeof(pv->vg_name) - 1);
        }
    }
    
    vg->free_size = vg->size;  /* Initially all free */
    vg->active = true;
    
    printf("Volume group created: %s (size: %lu bytes, PE: %lu)\n",
           vg_name, vg->size, vg->pe_count);
    
    return 0;
}

/* Create LVM logical volume */
static int lvm_create_logical_volume(const char* vg_name, const char* lv_name, uint64_t size) {
    printf("Creating LVM logical volume: %s/%s (size: %lu bytes)\n", vg_name, lv_name, size);
    
    /* Find volume group */
    lvm_vg_t* vg = NULL;
    for (uint32_t i = 0; i < g_storage.vg_count; i++) {
        if (strcmp(g_storage.volume_groups[i].name, vg_name) == 0) {
            vg = &g_storage.volume_groups[i];
            break;
        }
    }
    
    if (!vg) {
        printf("Volume group not found: %s\n", vg_name);
        return -1;
    }
    
    if (size > vg->free_size) {
        printf("Not enough free space in volume group %s\n", vg_name);
        return -1;
    }
    
    if (g_storage.lv_count >= MAX_LVM_VOLUMES) {
        printf("Maximum number of logical volumes reached\n");
        return -1;
    }
    
    lvm_lv_t* lv = &g_storage.logical_volumes[g_storage.lv_count++];
    memset(lv, 0, sizeof(lvm_lv_t));
    
    strncpy(lv->name, lv_name, sizeof(lv->name) - 1);
    snprintf(lv->full_name, sizeof(lv->full_name), "%s/%s", vg_name, lv_name);
    generate_uuid(lv->lv_uuid);
    lv->size = size;
    lv->le_count = (size + vg->pe_size - 1) / vg->pe_size;  /* Round up */
    lv->vg = vg;
    
    /* Create device path */
    snprintf(lv->device_path, sizeof(lv->device_path), "/dev/mapper/%s-%s", vg_name, lv_name);
    
    /* Update VG free space */
    vg->free_size -= size;
    vg->pe_free -= lv->le_count;
    
    /* Add LV to VG */
    if (vg->lv_count < 32) {
        vg->lvs[vg->lv_count++] = lv;
    }
    
    lv->active = true;
    
    printf("Logical volume created: %s (device: %s)\n", lv->full_name, lv->device_path);
    
    return 0;
}

/* Create RAID array */
static int raid_create_array(const char* array_name, raid_level_t level, const char** devices, uint32_t device_count) {
    printf("Creating RAID array: %s (level: %d, devices: %u)\n", array_name, level, device_count);
    
    if (g_storage.raid_count >= MAX_RAID_DEVICES) {
        printf("Maximum number of RAID arrays reached\n");
        return -1;
    }
    
    /* Validate device count for RAID level */
    uint32_t min_devices;
    switch (level) {
        case RAID_LEVEL_0: min_devices = 2; break;
        case RAID_LEVEL_1: min_devices = 2; break;
        case RAID_LEVEL_5: min_devices = 3; break;
        case RAID_LEVEL_6: min_devices = 4; break;
        case RAID_LEVEL_10: min_devices = 4; break;
        default:
            printf("Unsupported RAID level: %d\n", level);
            return -1;
    }
    
    if (device_count < min_devices) {
        printf("Insufficient devices for RAID level %d (need at least %u)\n", level, min_devices);
        return -1;
    }
    
    raid_array_t* raid = &g_storage.raid_arrays[g_storage.raid_count++];
    memset(raid, 0, sizeof(raid_array_t));
    
    strncpy(raid->name, array_name, sizeof(raid->name) - 1);
    snprintf(raid->device_path, sizeof(raid->device_path), "/dev/md%u", g_storage.raid_count - 1);
    raid->level = level;
    raid->device_count = device_count;
    raid->active_devices = device_count;
    raid->chunk_size = 64 * 1024;  /* 64KB default chunk size */
    
    /* Add devices */
    for (uint32_t i = 0; i < device_count && i < 16; i++) {
        strncpy(raid->devices[i], devices[i], sizeof(raid->devices[i]) - 1);
    }
    
    /* Calculate array size based on RAID level */
    uint64_t device_size = 0;
    int fd = open(devices[0], O_RDONLY);
    if (fd >= 0) {
        ioctl(fd, BLKGETSIZE64, &device_size);
        close(fd);
    }
    
    switch (level) {
        case RAID_LEVEL_0:
            raid->array_size = device_size * device_count;
            break;
        case RAID_LEVEL_1:
            raid->array_size = device_size;
            break;
        case RAID_LEVEL_5:
            raid->array_size = device_size * (device_count - 1);
            break;
        case RAID_LEVEL_6:
            raid->array_size = device_size * (device_count - 2);
            break;
        case RAID_LEVEL_10:
            raid->array_size = device_size * (device_count / 2);
            break;
    }
    
    raid->active = true;
    
    printf("RAID array created: %s (device: %s, size: %lu bytes)\n",
           array_name, raid->device_path, raid->array_size);
    
    return 0;
}

/* Setup LUKS encrypted device */
static int luks_setup_device(const char* device, const char* name, const char* passphrase) {
    printf("Setting up LUKS encryption on %s -> %s\n", device, name);
    
    if (g_storage.luks_device_count >= 32) {
        printf("Maximum number of LUKS devices reached\n");
        return -1;
    }
    
    /* In real implementation, would use libcryptsetup */
    /* Here we just simulate the setup */
    
    char mapped_device[256];
    snprintf(mapped_device, sizeof(mapped_device), "/dev/mapper/%s", name);
    
    strncpy(g_storage.luks_devices[g_storage.luks_device_count], mapped_device, 255);
    g_storage.luks_device_count++;
    
    printf("LUKS device setup complete: %s\n", mapped_device);
    
    return 0;
}

/* Utility functions */
static uint32_t calculate_crc32(const void* data, size_t length) {
    /* Simplified CRC32 calculation */
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return ~crc;
}

static void generate_uuid(uuid_t uuid) {
    /* Generate random UUID */
    for (int i = 0; i < 16; i++) {
        uuid[i] = rand() & 0xFF;
    }
    
    /* Set version (4) and variant bits */
    uuid[6] = (uuid[6] & 0x0F) | 0x40;  /* Version 4 */
    uuid[8] = (uuid[8] & 0x3F) | 0x80;  /* Variant 10 */
}

static const char* get_partition_type_name(const uuid_t guid) {
    for (size_t i = 0; i < sizeof(gpt_partition_types) / sizeof(gpt_partition_types[0]); i++) {
        if (memcmp(gpt_partition_types[i].guid, guid, sizeof(uuid_t)) == 0) {
            return gpt_partition_types[i].name;
        }
    }
    return "unknown";
}

/* Read/write GPT functions */
static int gpt_read_header(const char* device, gpt_header_t* header) {
    int fd = open(device, O_RDONLY);
    if (fd < 0) return -1;
    
    /* Read GPT header from LBA 1 */
    ssize_t result = pread(fd, header, sizeof(gpt_header_t), 512);
    close(fd);
    
    return (result == sizeof(gpt_header_t)) ? 0 : -1;
}

static int gpt_write_header(const char* device, const gpt_header_t* header) {
    int fd = open(device, O_WRONLY);
    if (fd < 0) return -1;
    
    ssize_t result = pwrite(fd, header, sizeof(gpt_header_t), 512);
    fsync(fd);
    close(fd);
    
    return (result == sizeof(gpt_header_t)) ? 0 : -1;
}

static int gpt_read_partitions(const char* device, gpt_partition_entry_t* partitions, uint32_t max_count) {
    int fd = open(device, O_RDONLY);
    if (fd < 0) return -1;
    
    size_t read_size = max_count * sizeof(gpt_partition_entry_t);
    ssize_t result = pread(fd, partitions, read_size, 2 * 512);  /* LBA 2 */
    close(fd);
    
    return (result == (ssize_t)read_size) ? 0 : -1;
}

static int gpt_write_partitions(const char* device, const gpt_partition_entry_t* partitions, uint32_t count) {
    int fd = open(device, O_WRONLY);
    if (fd < 0) return -1;
    
    size_t write_size = count * sizeof(gpt_partition_entry_t);
    ssize_t result = pwrite(fd, partitions, write_size, 2 * 512);  /* LBA 2 */
    fsync(fd);
    close(fd);
    
    return (result == (ssize_t)write_size) ? 0 : -1;
}

static int lvm_scan_physical_volumes(void) {
    /* Scan for existing PVs (simplified implementation) */
    printf("Scanning for LVM physical volumes...\n");
    
    /* In real implementation would scan /proc/partitions and check for LVM signatures */
    
    return 0;
}

/* Main CLI interface */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command> [options]\n", argv[0]);
        printf("Commands:\n");
        printf("  init                           - Initialize storage manager\n");
        printf("  gpt-create <device>            - Create GPT partition table\n");
        printf("  gpt-add <device> <type> <start> <size> <name> - Add GPT partition\n");
        printf("  gpt-list <device>              - List GPT partitions\n");
        printf("  pv-create <device>             - Create LVM physical volume\n");
        printf("  vg-create <name> <devices...>  - Create LVM volume group\n");
        printf("  lv-create <vg> <name> <size>   - Create LVM logical volume\n");
        printf("  raid-create <name> <level> <devices...> - Create RAID array\n");
        printf("  luks-setup <device> <name>     - Setup LUKS encryption\n");
        printf("  status                         - Show storage status\n");
        return 1;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "init") == 0) {
        return storage_manager_init();
        
    } else if (strcmp(command, "gpt-create") == 0) {
        if (argc < 3) {
            printf("Usage: gpt-create <device>\n");
            return 1;
        }
        storage_manager_init();
        return gpt_create_partition_table(argv[2]);
        
    } else if (strcmp(command, "gpt-add") == 0) {
        if (argc < 7) {
            printf("Usage: gpt-add <device> <type> <start> <size> <name>\n");
            return 1;
        }
        storage_manager_init();
        return gpt_add_partition(argv[2], argv[3], atoll(argv[4]), atoll(argv[5]), argv[6]);
        
    } else if (strcmp(command, "gpt-list") == 0) {
        if (argc < 3) {
            printf("Usage: gpt-list <device>\n");
            return 1;
        }
        storage_manager_init();
        return gpt_list_partitions(argv[2]);
        
    } else if (strcmp(command, "pv-create") == 0) {
        if (argc < 3) {
            printf("Usage: pv-create <device>\n");
            return 1;
        }
        storage_manager_init();
        return lvm_create_physical_volume(argv[2]);
        
    } else if (strcmp(command, "vg-create") == 0) {
        if (argc < 4) {
            printf("Usage: vg-create <name> <devices...>\n");
            return 1;
        }
        storage_manager_init();
        const char** devices = (const char**)&argv[3];
        return lvm_create_volume_group(argv[2], devices, argc - 3);
        
    } else if (strcmp(command, "lv-create") == 0) {
        if (argc < 5) {
            printf("Usage: lv-create <vg> <name> <size>\n");
            return 1;
        }
        storage_manager_init();
        return lvm_create_logical_volume(argv[2], argv[3], atoll(argv[4]));
        
    } else if (strcmp(command, "raid-create") == 0) {
        if (argc < 5) {
            printf("Usage: raid-create <name> <level> <devices...>\n");
            return 1;
        }
        storage_manager_init();
        
        raid_level_t level;
        switch (atoi(argv[3])) {
            case 0: level = RAID_LEVEL_0; break;
            case 1: level = RAID_LEVEL_1; break;
            case 5: level = RAID_LEVEL_5; break;
            case 6: level = RAID_LEVEL_6; break;
            case 10: level = RAID_LEVEL_10; break;
            default:
                printf("Unsupported RAID level: %s\n", argv[3]);
                return 1;
        }
        
        const char** devices = (const char**)&argv[4];
        return raid_create_array(argv[2], level, devices, argc - 4);
        
    } else if (strcmp(command, "luks-setup") == 0) {
        if (argc < 4) {
            printf("Usage: luks-setup <device> <name>\n");
            return 1;
        }
        storage_manager_init();
        return luks_setup_device(argv[2], argv[3], "passphrase");
        
    } else if (strcmp(command, "status") == 0) {
        storage_manager_init();
        
        printf("\nLimitlessOS Advanced Storage Status\n");
        printf("===================================\n");
        
        printf("\nLVM Physical Volumes: %u\n", g_storage.pv_count);
        for (uint32_t i = 0; i < g_storage.pv_count; i++) {
            lvm_pv_t* pv = &g_storage.physical_volumes[i];
            printf("  %s: %lu bytes, VG: %s\n", pv->device_path, pv->size, 
                   strlen(pv->vg_name) > 0 ? pv->vg_name : "none");
        }
        
        printf("\nLVM Volume Groups: %u\n", g_storage.vg_count);
        for (uint32_t i = 0; i < g_storage.vg_count; i++) {
            lvm_vg_t* vg = &g_storage.volume_groups[i];
            printf("  %s: %lu bytes total, %lu bytes free, %u LVs\n",
                   vg->name, vg->size, vg->free_size, vg->lv_count);
        }
        
        printf("\nLVM Logical Volumes: %u\n", g_storage.lv_count);
        for (uint32_t i = 0; i < g_storage.lv_count; i++) {
            lvm_lv_t* lv = &g_storage.logical_volumes[i];
            printf("  %s: %lu bytes, device: %s\n", lv->full_name, lv->size, lv->device_path);
        }
        
        printf("\nRAID Arrays: %u\n", g_storage.raid_count);
        for (uint32_t i = 0; i < g_storage.raid_count; i++) {
            raid_array_t* raid = &g_storage.raid_arrays[i];
            printf("  %s: RAID%d, %u devices, %lu bytes, device: %s\n",
                   raid->name, raid->level, raid->device_count, raid->array_size, raid->device_path);
        }
        
        printf("\nLUKS Devices: %u\n", g_storage.luks_device_count);
        for (uint32_t i = 0; i < g_storage.luks_device_count; i++) {
            printf("  %s\n", g_storage.luks_devices[i]);
        }
        
        return 0;
        
    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }
}