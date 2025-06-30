#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Filesystem constants
#define MAX_FILENAME_LENGTH 256
#define MAX_PATH_LENGTH 1024
#define BLOCK_SIZE 4096
#define MAX_OPEN_FILES 256
#define MAX_INODES 65536
#define MAGIC_NUMBER 0x48415348  // "HASH" in hex

// File types
typedef enum {
    FILE_TYPE_REGULAR = 0,
    FILE_TYPE_DIRECTORY = 1,
    FILE_TYPE_SYMLINK = 2,
    FILE_TYPE_DEVICE = 3
} file_type_t;

// File permissions
#define PERM_READ_OWNER   0x400
#define PERM_WRITE_OWNER  0x200
#define PERM_EXEC_OWNER   0x100
#define PERM_READ_GROUP   0x040
#define PERM_WRITE_GROUP  0x020
#define PERM_EXEC_GROUP   0x010
#define PERM_READ_OTHER   0x004
#define PERM_WRITE_OTHER  0x002
#define PERM_EXEC_OTHER   0x001

// Superblock structure
typedef struct {
    uint32_t magic;              // Magic number to identify filesystem
    uint32_t block_size;         // Size of each block
    uint32_t total_blocks;       // Total number of blocks
    uint32_t free_blocks;        // Number of free blocks
    uint32_t total_inodes;       // Total number of inodes
    uint32_t free_inodes;        // Number of free inodes
    uint32_t inode_table_start;  // Starting block of inode table
    uint32_t data_blocks_start;  // Starting block of data area
    uint32_t block_bitmap_start; // Starting block of block bitmap
    uint32_t inode_bitmap_start; // Starting block of inode bitmap
    char volume_name[64];        // Volume name
    uint64_t created_time;       // Filesystem creation time
    uint64_t last_mount_time;    // Last mount time
    uint32_t mount_count;        // Number of times mounted
    uint32_t checksum;           // Superblock checksum
} superblock_t;

// Inode structure
typedef struct {
    uint32_t mode;               // File type and permissions
    uint32_t uid;                // User ID
    uint32_t gid;                // Group ID
    uint64_t size;               // File size in bytes
    uint64_t atime;              // Access time
    uint64_t mtime;              // Modification time
    uint64_t ctime;              // Creation time
    uint32_t links_count;        // Number of hard links
    uint32_t blocks;             // Number of blocks allocated
    uint32_t direct_blocks[12];  // Direct block pointers
    uint32_t indirect_block;     // Single indirect block pointer
    uint32_t double_indirect;    // Double indirect block pointer
    uint32_t triple_indirect;    // Triple indirect block pointer
    uint32_t reserved[4];        // Reserved for future use
} inode_t;

// Directory entry structure
typedef struct {
    uint32_t inode;              // Inode number
    uint16_t rec_len;            // Record length
    uint8_t name_len;            // Name length
    uint8_t file_type;           // File type
    char name[MAX_FILENAME_LENGTH]; // File name
} dir_entry_t;

// File descriptor structure
typedef struct {
    bool in_use;                 // Whether this FD is in use
    uint32_t inode_num;          // Inode number
    uint64_t position;           // Current file position
    uint32_t flags;              // Open flags
    inode_t *inode;              // Cached inode
} file_descriptor_t;

// Filesystem context
typedef struct {
    superblock_t *superblock;
    uint8_t *block_bitmap;
    uint8_t *inode_bitmap;
    inode_t *inode_table;
    file_descriptor_t fd_table[MAX_OPEN_FILES];
    uint32_t next_fd;
} filesystem_t;

// Global filesystem instance
static filesystem_t fs;

// Function prototypes
int hash_fs_init(void *disk_image, size_t size);
int hash_fs_format(void *disk_image, size_t size, const char *volume_name);
int hash_fs_mount(void *disk_image);
int hash_fs_unmount(void);

// File operations
int hash_fs_open(const char *path, int flags);
int hash_fs_close(int fd);
ssize_t hash_fs_read(int fd, void *buffer, size_t count);
ssize_t hash_fs_write(int fd, const void *buffer, size_t count);
off_t hash_fs_lseek(int fd, off_t offset, int whence);
int hash_fs_unlink(const char *path);

// Directory operations
int hash_fs_mkdir(const char *path, mode_t mode);
int hash_fs_rmdir(const char *path);
int hash_fs_opendir(const char *path);
int hash_fs_readdir(int dirfd, dir_entry_t *entry);
int hash_fs_closedir(int dirfd);

// Utility functions
static uint32_t allocate_block(void);
static void free_block(uint32_t block_num);
static uint32_t allocate_inode(void);
static void free_inode(uint32_t inode_num);
static inode_t *get_inode(uint32_t inode_num);
static int write_inode(uint32_t inode_num, inode_t *inode);
static uint32_t find_free_fd(void);

// Implementation

int hash_fs_init(void *disk_image, size_t size) {
    if (!disk_image || size < BLOCK_SIZE * 1024) {
        return -1; // Invalid parameters
    }
    
    memset(&fs, 0, sizeof(filesystem_t));
    return hash_fs_mount(disk_image);
}

int hash_fs_format(void *disk_image, size_t size, const char *volume_name) {
    if (!disk_image || size < BLOCK_SIZE * 1024) {
        return -1;
    }
    
    // Calculate filesystem layout
    uint32_t total_blocks = size / BLOCK_SIZE;
    uint32_t inode_blocks = (MAX_INODES * sizeof(inode_t) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint32_t bitmap_blocks = (total_blocks + BLOCK_SIZE * 8 - 1) / (BLOCK_SIZE * 8);
    uint32_t inode_bitmap_blocks = (MAX_INODES + BLOCK_SIZE * 8 - 1) / (BLOCK_SIZE * 8);
    
    // Initialize superblock
    superblock_t *sb = (superblock_t *)disk_image;
    memset(sb, 0, sizeof(superblock_t));
    
    sb->magic = MAGIC_NUMBER;
    sb->block_size = BLOCK_SIZE;
    sb->total_blocks = total_blocks;
    sb->free_blocks = total_blocks - 1 - bitmap_blocks - inode_bitmap_blocks - inode_blocks;
    sb->total_inodes = MAX_INODES;
    sb->free_inodes = MAX_INODES - 1; // Root directory uses one inode
    sb->inode_table_start = 1 + bitmap_blocks + inode_bitmap_blocks;
    sb->data_blocks_start = sb->inode_table_start + inode_blocks;
    sb->block_bitmap_start = 1;
    sb->inode_bitmap_start = 1 + bitmap_blocks;
    
    if (volume_name) {
        strncpy(sb->volume_name, volume_name, sizeof(sb->volume_name) - 1);
    } else {
        strcpy(sb->volume_name, "HASH_FS");
    }
    
    // Initialize bitmaps
    uint8_t *block_bitmap = (uint8_t *)disk_image + sb->block_bitmap_start * BLOCK_SIZE;
    uint8_t *inode_bitmap = (uint8_t *)disk_image + sb->inode_bitmap_start * BLOCK_SIZE;
    
    memset(block_bitmap, 0, bitmap_blocks * BLOCK_SIZE);
    memset(inode_bitmap, 0, inode_bitmap_blocks * BLOCK_SIZE);
    
    // Mark used blocks in bitmap
    for (uint32_t i = 0; i < sb->data_blocks_start; i++) {
        block_bitmap[i / 8] |= (1 << (i % 8));
    }
    
    // Mark root inode as used
    inode_bitmap[0] |= 1;
    
    // Initialize inode table
    inode_t *inode_table = (inode_t *)((uint8_t *)disk_image + sb->inode_table_start * BLOCK_SIZE);
    memset(inode_table, 0, inode_blocks * BLOCK_SIZE);
    
    // Create root directory inode
    inode_t *root_inode = &inode_table[0];
    root_inode->mode = FILE_TYPE_DIRECTORY | 0755;
    root_inode->uid = 0;
    root_inode->gid = 0;
    root_inode->size = 0;
    root_inode->links_count = 2; // . and ..
    root_inode->blocks = 0;
    
    return 0;
}

int hash_fs_mount(void *disk_image) {
    if (!disk_image) {
        return -1;
    }
    
    fs.superblock = (superblock_t *)disk_image;
    
    // Verify magic number
    if (fs.superblock->magic != MAGIC_NUMBER) {
        return -1; // Invalid filesystem
    }
    
    // Set up pointers to filesystem structures
    fs.block_bitmap = (uint8_t *)disk_image + fs.superblock->block_bitmap_start * BLOCK_SIZE;
    fs.inode_bitmap = (uint8_t *)disk_image + fs.superblock->inode_bitmap_start * BLOCK_SIZE;
    fs.inode_table = (inode_t *)((uint8_t *)disk_image + fs.superblock->inode_table_start * BLOCK_SIZE);
    
    // Initialize file descriptor table
    memset(fs.fd_table, 0, sizeof(fs.fd_table));
    fs.next_fd = 0;
    
    return 0;
}

int hash_fs_unmount(void) {
    // Close all open files
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs.fd_table[i].in_use) {
            hash_fs_close(i);
        }
    }
    
    // Clear filesystem context
    memset(&fs, 0, sizeof(filesystem_t));
    return 0;
}

static uint32_t find_free_fd(void) {
    for (uint32_t i = 0; i < MAX_OPEN_FILES; i++) {
        uint32_t fd = (fs.next_fd + i) % MAX_OPEN_FILES;
        if (!fs.fd_table[fd].in_use) {
            fs.next_fd = (fd + 1) % MAX_OPEN_FILES;
            return fd;
        }
    }
    return (uint32_t)-1; // No free file descriptors
}

static uint32_t allocate_block(void) {
    if (fs.superblock->free_blocks == 0) {
        return 0; // No free blocks
    }
    
    for (uint32_t i = fs.superblock->data_blocks_start; i < fs.superblock->total_blocks; i++) {
        uint32_t byte_index = i / 8;
        uint32_t bit_index = i % 8;
        
        if (!(fs.block_bitmap[byte_index] & (1 << bit_index))) {
            fs.block_bitmap[byte_index] |= (1 << bit_index);
            fs.superblock->free_blocks--;
            return i;
        }
    }
    
    return 0; // No free blocks found
}

static void free_block(uint32_t block_num) {
    if (block_num >= fs.superblock->total_blocks) {
        return; // Invalid block number
    }
    
    uint32_t byte_index = block_num / 8;
    uint32_t bit_index = block_num % 8;
    
    if (fs.block_bitmap[byte_index] & (1 << bit_index)) {
        fs.block_bitmap[byte_index] &= ~(1 << bit_index);
        fs.superblock->free_blocks++;
    }
}

static uint32_t allocate_inode(void) {
    if (fs.superblock->free_inodes == 0) {
        return 0; // No free inodes
    }
    
    for (uint32_t i = 1; i < fs.superblock->total_inodes; i++) {
        uint32_t byte_index = i / 8;
        uint32_t bit_index = i % 8;
        
        if (!(fs.inode_bitmap[byte_index] & (1 << bit_index))) {
            fs.inode_bitmap[byte_index] |= (1 << bit_index);
            fs.superblock->free_inodes--;
            return i;
        }
    }
    
    return 0; // No free inodes found
}

static void free_inode(uint32_t inode_num) {
    if (inode_num >= fs.superblock->total_inodes) {
        return; // Invalid inode number
    }
    
    uint32_t byte_index = inode_num / 8;
    uint32_t bit_index = inode_num % 8;
    
    if (fs.inode_bitmap[byte_index] & (1 << bit_index)) {
        fs.inode_bitmap[byte_index] &= ~(1 << bit_index);
        fs.superblock->free_inodes++;
    }
}

static inode_t *get_inode(uint32_t inode_num) {
    if (inode_num >= fs.superblock->total_inodes) {
        return NULL;
    }
    
    return &fs.inode_table[inode_num];
}

static int write_inode(uint32_t inode_num, inode_t *inode) {
    if (inode_num >= fs.superblock->total_inodes || !inode) {
        return -1;
    }
    
    memcpy(&fs.inode_table[inode_num], inode, sizeof(inode_t));
    return 0;
}

int hash_fs_open(const char *path, int flags) {
    if (!path) {
        return -1;
    }
    
    uint32_t fd = find_free_fd();
    if (fd == (uint32_t)-1) {
        return -1; // No free file descriptors
    }
    
    // For simplicity, we'll assume the path refers to inode 1 for now
    // In a real implementation, you'd need path resolution
    uint32_t inode_num = 1;
    inode_t *inode = get_inode(inode_num);
    
    if (!inode) {
        return -1;
    }
    
    fs.fd_table[fd].in_use = true;
    fs.fd_table[fd].inode_num = inode_num;
    fs.fd_table[fd].position = 0;
    fs.fd_table[fd].flags = flags;
    fs.fd_table[fd].inode = inode;
    
    return fd;
}

int hash_fs_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fs.fd_table[fd].in_use) {
        return -1;
    }
    
    fs.fd_table[fd].in_use = false;
    fs.fd_table[fd].inode = NULL;
    
    return 0;
}

ssize_t hash_fs_read(int fd, void *buffer, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fs.fd_table[fd].in_use || !buffer) {
        return -1;
    }
    
    file_descriptor_t *file_desc = &fs.fd_table[fd];
    inode_t *inode = file_desc->inode;
    
    if (file_desc->position >= inode->size) {
        return 0; // EOF
    }
    
    size_t bytes_to_read = count;
    if (file_desc->position + count > inode->size) {
        bytes_to_read = inode->size - file_desc->position;
    }
    
    // Simplified: read from first direct block only
    if (inode->direct_blocks[0] != 0) {
        uint8_t *block_data = (uint8_t *)fs.superblock + inode->direct_blocks[0] * BLOCK_SIZE;
        memcpy(buffer, block_data + file_desc->position, bytes_to_read);
        file_desc->position += bytes_to_read;
        return bytes_to_read;
    }
    
    return 0;
}

ssize_t hash_fs_write(int fd, const void *buffer, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fs.fd_table[fd].in_use || !buffer) {
        return -1;
    }
    
    file_descriptor_t *file_desc = &fs.fd_table[fd];
    inode_t *inode = file_desc->inode;
    
    // Allocate a block if needed
    if (inode->direct_blocks[0] == 0) {
        uint32_t block_num = allocate_block();
        if (block_num == 0) {
            return -1; // No free blocks
        }
        inode->direct_blocks[0] = block_num;
        inode->blocks = 1;
    }
    
    // Simplified: write to first direct block only
    uint8_t *block_data = (uint8_t *)fs.superblock + inode->direct_blocks[0] * BLOCK_SIZE;
    size_t bytes_to_write = count;
    if (file_desc->position + count > BLOCK_SIZE) {
        bytes_to_write = BLOCK_SIZE - file_desc->position;
    }
    
    memcpy(block_data + file_desc->position, buffer, bytes_to_write);
    file_desc->position += bytes_to_write;
    
    if (file_desc->position > inode->size) {
        inode->size = file_desc->position;
    }
    
    return bytes_to_write;
}

off_t hash_fs_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fs.fd_table[fd].in_use) {
        return -1;
    }
    
    file_descriptor_t *file_desc = &fs.fd_table[fd];
    off_t new_position;
    
    switch (whence) {
        case 0: // SEEK_SET
            new_position = offset;
            break;
        case 1: // SEEK_CUR
            new_position = file_desc->position + offset;
            break;
        case 2: // SEEK_END
            new_position = file_desc->inode->size + offset;
            break;
        default:
            return -1;
    }
    
    if (new_position < 0) {
        return -1;
    }
    
    file_desc->position = new_position;
    return new_position;
}

int hash_fs_mkdir(const char *path, mode_t mode) {
    if (!path) {
        return -1;
    }
    
    uint32_t inode_num = allocate_inode();
    if (inode_num == 0) {
        return -1; // No free inodes
    }
    
    inode_t *inode = get_inode(inode_num);
    if (!inode) {
        free_inode(inode_num);
        return -1;
    }
    
    // Initialize directory inode
    memset(inode, 0, sizeof(inode_t));
    inode->mode = FILE_TYPE_DIRECTORY | (mode & 0777);
    inode->uid = 0; // Current user ID
    inode->gid = 0; // Current group ID
    inode->size = 0;
    inode->links_count = 2; // . and ..
    inode->blocks = 0;
    
    return 0;
}

// Additional filesystem functions would be implemented here...
// This is a foundational implementation that can be extended

// Debug and utility functions
void hash_fs_debug_info(void) {
    if (!fs.superblock) {
        return;
    }
    
    // Print filesystem information (would be implemented based on your debug needs)
}
