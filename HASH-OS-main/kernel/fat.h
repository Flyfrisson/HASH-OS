#ifndef HASHOS_FAT_H
#define HASHOS_FAT_H

#include <stdint.h>
#include <stdbool.h>

// FAT filesystem constants
#define FAT_SECTOR_SIZE         512
#define FAT_MAX_FILENAME        11      // 8.3 format
#define FAT_MAX_PATH            256
#define FAT_ROOT_DIR_ENTRIES    224     // Typical for FAT12/16
#define FAT_CLUSTER_FREE        0x0000
#define FAT_CLUSTER_BAD         0xFFF7
#define FAT_CLUSTER_EOF         0xFFFF

// FAT types
typedef enum {
    FAT_TYPE_UNKNOWN = 0,
    FAT_TYPE_FAT12,
    FAT_TYPE_FAT16,
    FAT_TYPE_FAT32
} fat_type_t;

// Disk operation return codes
#define DISK_SUCCESS        1
#define DISK_ERROR_INVALID  0
#define DISK_ERROR_TIMEOUT  2
#define DISK_ERROR_NOT_READY 3

// FAT Boot Sector structure (packed to match on-disk layout)
struct __attribute__((packed)) fat_boot_sector {
    uint8_t  jump_instruction[3];       // 0x00: Jump to boot code
    uint8_t  oem_name[8];              // 0x03: OEM identifier
    uint16_t bytes_per_sector;         // 0x0B: Bytes per sector
    uint8_t  sectors_per_cluster;      // 0x0D: Sectors per cluster
    uint16_t reserved_sectors;         // 0x0E: Reserved sectors
    uint8_t  fat_count;               // 0x10: Number of FAT copies
    uint16_t root_dir_entries;        // 0x11: Root directory entries
    uint16_t total_sectors_16;        // 0x13: Total sectors (if < 65536)
    uint8_t  media_descriptor;        // 0x15: Media descriptor
    uint16_t sectors_per_fat;         // 0x16: Sectors per FAT
    uint16_t sectors_per_track;       // 0x18: Sectors per track
    uint16_t heads;                   // 0x1A: Number of heads
    uint32_t hidden_sectors;          // 0x1C: Hidden sectors
    uint32_t total_sectors_32;        // 0x20: Total sectors (if >= 65536)
    
    // Extended boot record (differs between FAT12/16 and FAT32)
    union {
        struct {
            uint8_t  drive_number;        // 0x24: Drive number
            uint8_t  reserved;           // 0x25: Reserved
            uint8_t  boot_signature;     // 0x26: Extended boot signature
            uint32_t volume_serial;      // 0x27: Volume serial number
            uint8_t  volume_label[11];   // 0x2B: Volume label
            uint8_t  filesystem_type[8]; // 0x36: Filesystem type
        } fat16;
        
        struct {
            uint32_t sectors_per_fat_32;  // 0x24: Sectors per FAT (FAT32)
            uint16_t flags;              // 0x28: Flags
            uint16_t version;            // 0x2A: Version
            uint32_t root_cluster;       // 0x2C: Root directory cluster
            uint16_t fsinfo_sector;      // 0x30: FSInfo sector
            uint16_t backup_boot_sector; // 0x32: Backup boot sector
            uint8_t  reserved[12];       // 0x34: Reserved
            uint8_t  drive_number;       // 0x40: Drive number
            uint8_t  reserved2;          // 0x41: Reserved
            uint8_t  boot_signature;     // 0x42: Extended boot signature
            uint32_t volume_serial;      // 0x43: Volume serial number
            uint8_t  volume_label[11];   // 0x47: Volume label
            uint8_t  filesystem_type[8]; // 0x52: Filesystem type
        } fat32;
    } extended;
};

// FAT directory entry structure
struct __attribute__((packed)) fat_dir_entry {
    uint8_t  filename[11];        // 0x00: Filename (8.3 format)
    uint8_t  attributes;          // 0x0B: File attributes
    uint8_t  reserved;           // 0x0C: Reserved
    uint8_t  creation_time_fine; // 0x0D: Creation time (fine resolution)
    uint16_t creation_time;      // 0x0E: Creation time
    uint16_t creation_date;      // 0x10: Creation date
    uint16_t access_date;        // 0x12: Last access date
    uint16_t cluster_high;       // 0x14: High 16 bits of cluster (FAT32)
    uint16_t modify_time;        // 0x16: Last modification time
    uint16_t modify_date;        // 0x18: Last modification date
    uint16_t cluster_low;        // 0x1A: Low 16 bits of cluster
    uint32_t file_size;          // 0x1C: File size in bytes
};

// File attributes
#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLUME_ID  0x08
#define FAT_ATTR_DIRECTORY  0x10
#define FAT_ATTR_ARCHIVE    0x20
#define FAT_ATTR_LONG_NAME  0x0F  // Long filename entry

// FAT filesystem context
struct fat_context {
    struct fat_boot_sector boot_sector;
    fat_type_t fat_type;
    uint32_t fat_start_sector;
    uint32_t root_dir_start_sector;
    uint32_t data_start_sector;
    uint32_t total_clusters;
    uint16_t bytes_per_cluster;
    bool initialized;
};

// File handle structure
struct fat_file {
    char filename[FAT_MAX_FILENAME + 1];
    uint32_t cluster;
    uint32_t size;
    uint32_t position;
    uint8_t attributes;
    bool is_open;
};

// Core disk I/O functions
unsigned char read_sector(unsigned int lba, unsigned char* buffer);
unsigned char read_sectors(unsigned int start_lba, unsigned int count, unsigned char* buffer);
unsigned char init_disk(void);
void get_disk_info(unsigned int* total_sectors, unsigned int* bytes_per_sector);

// FAT filesystem functions
unsigned char fat_init(struct fat_context* ctx);
unsigned char fat_mount(struct fat_context* ctx);
fat_type_t fat_detect_type(const struct fat_boot_sector* boot);

// Directory operations
unsigned char fat_list_directory(struct fat_context* ctx, const char* path);
unsigned char fat_find_file(struct fat_context* ctx, const char* filename, 
                           struct fat_dir_entry* entry);
unsigned char fat_create_directory(struct fat_context* ctx, const char* dirname);

// File operations
unsigned char fat_open_file(struct fat_context* ctx, const char* filename, 
                           struct fat_file* file);
unsigned char fat_read_file(struct fat_context* ctx, struct fat_file* file, 
                           void* buffer, uint32_t size, uint32_t* bytes_read);
unsigned char fat_write_file(struct fat_context* ctx, struct fat_file* file,
                            const void* buffer, uint32_t size, uint32_t* bytes_written);
unsigned char fat_close_file(struct fat_file* file);
unsigned char fat_delete_file(struct fat_context* ctx, const char* filename);

// Cluster management
uint32_t fat_get_next_cluster(struct fat_context* ctx, uint32_t cluster);
uint32_t fat_allocate_cluster(struct fat_context* ctx);
unsigned char fat_free_cluster_chain(struct fat_context* ctx, uint32_t start_cluster);

// Utility functions
void fat_filename_to_83(const char* filename, char* fat_name);
void fat_83_to_filename(const char* fat_name, char* filename);
bool fat_is_valid_filename(const char* filename);
uint32_t fat_cluster_to_sector(struct fat_context* ctx, uint32_t cluster);

// Debug and logging
void fat_log(const char* message);
void fat_dump_boot_sector(const struct fat_boot_sector* boot);
void fat_dump_directory_entry(const struct fat_dir_entry* entry);

#endif // HASHOS_FAT_H
