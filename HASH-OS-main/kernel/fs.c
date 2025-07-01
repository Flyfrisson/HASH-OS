#include "fs.h"
#include "../drivers/display4k.h"
#include <string.h>
#include <stdint.h>

// Mock file system structure
typedef struct {
    char name[32];
    uint32_t size;
    uint8_t attributes;
    uint32_t creation_time;
    char* content;  // For demonstration purposes
} mock_file_t;

// File attributes
#define FS_ATTR_READ_ONLY   0x01
#define FS_ATTR_HIDDEN      0x02
#define FS_ATTR_SYSTEM      0x04
#define FS_ATTR_DIRECTORY   0x08
#define FS_ATTR_ARCHIVE     0x10

// Filesystem status
typedef enum {
    FS_STATUS_UNINITIALIZED = 0,
    FS_STATUS_INITIALIZING,
    FS_STATUS_READY,
    FS_STATUS_ERROR
} fs_status_t;

static fs_status_t fs_status = FS_STATUS_UNINITIALIZED;
static uint32_t total_files = 0;
static uint32_t total_size = 0;

// Enhanced mock file system with file details
static mock_file_t mock_files[] = {
    {"bootldr.sys",  8192,  FS_ATTR_SYSTEM | FS_ATTR_READ_ONLY, 0x12345678, "Boot loader binary"},
    {"kernel.bin",   65536, FS_ATTR_SYSTEM | FS_ATTR_READ_ONLY, 0x12345679, "Kernel executable"},
    {"config.cfg",   1024,  FS_ATTR_ARCHIVE, 0x1234567A, "System configuration"},
    {"readme.txt",   2048,  FS_ATTR_ARCHIVE, 0x1234567B, "System documentation"},
    {"HASHOS.sys",   32768, FS_ATTR_SYSTEM | FS_ATTR_READ_ONLY, 0x1234567C, "Operating system core"},
    {"drivers/",     0,     FS_ATTR_DIRECTORY, 0x1234567D, NULL},
    {"temp/",        0,     FS_ATTR_DIRECTORY, 0x1234567E, NULL},
    {"user.dat",     4096,  FS_ATTR_ARCHIVE | FS_ATTR_HIDDEN, 0x1234567F, "User data file"},
    {0}  // Sentinel
};

// Color scheme for filesystem display
#define COLOR_BACKGROUND    0x001122
#define COLOR_HEADER        0x3366FF
#define COLOR_FILE_NORMAL   0xFFFFFF
#define COLOR_FILE_SYSTEM   0xFF6666
#define COLOR_FILE_HIDDEN   0x888888
#define COLOR_DIRECTORY     0x66FFFF
#define COLOR_STATUS_OK     0x66FF66
#define COLOR_STATUS_ERROR  0xFF3333

// Logging function
void fs_log(const char* message) {
    // In real system: write to debug console or log buffer
    // For now: could write to screen or serial port
}

// Calculate filesystem statistics
void calculate_fs_stats(void) {
    total_files = 0;
    total_size = 0;
    
    for (int i = 0; mock_files[i].name[0] != 0; i++) {
        total_files++;
        total_size += mock_files[i].size;
    }
}

// Enhanced filesystem initialization
int init_filesystem(void) {
    fs_status = FS_STATUS_INITIALIZING;
    fs_log("Initializing filesystem...");
    
    // Clear the display area
    draw_rect(90, 90, 620, 400, COLOR_BACKGROUND);
    
    // Draw filesystem initialization header
    draw_rect(100, 100, 600, 40, COLOR_HEADER);
    draw_string(110, 115, "HASHOS Filesystem Initializing...", COLOR_FILE_NORMAL);
    
    // Simulate initialization delay
    for (volatile int i = 0; i < 1000000; i++);
    
    // Calculate filesystem statistics
    calculate_fs_stats();
    
    // Check for initialization errors (simulation)
    if (total_files == 0) {
        fs_status = FS_STATUS_ERROR;
        draw_rect(100, 150, 600, 30, COLOR_STATUS_ERROR);
        draw_string(110, 165, "ERROR: No files found!", COLOR_FILE_NORMAL);
        return -1;
    }
    
    fs_status = FS_STATUS_READY;
    
    // Update status display
    draw_rect(100, 100, 600, 40, COLOR_STATUS_OK);
    draw_string(110, 115, "HASHOS Filesystem Ready", COLOR_FILE_NORMAL);
    
    // Display filesystem statistics
    char stats[64];
    sprintf(stats, "Files: %u | Total Size: %u bytes", total_files, total_size);
    draw_string(110, 170, stats, COLOR_FILE_NORMAL);
    
    fs_log("Filesystem initialized successfully");
    return 0;
}

// Get file color based on attributes
uint32_t get_file_color(uint8_t attributes) {
    if (attributes & FS_ATTR_DIRECTORY) {
        return COLOR_DIRECTORY;
    } else if (attributes & FS_ATTR_SYSTEM) {
        return COLOR_FILE_SYSTEM;
    } else if (attributes & FS_ATTR_HIDDEN) {
        return COLOR_FILE_HIDDEN;
    } else {
        return COLOR_FILE_NORMAL;
    }
}

// Format file size for display
void format_file_size(uint32_t size, char* buffer) {
    if (size == 0) {
        strcpy(buffer, "<DIR>");
    } else if (size < 1024) {
        sprintf(buffer, "%u B", size);
    } else if (size < 1024 * 1024) {
        sprintf(buffer, "%u KB", size / 1024);
    } else {
        sprintf(buffer, "%u MB", size / (1024 * 1024));
    }
}

// Enhanced directory listing with file details
void list_root_directory(void) {
    if (fs_status != FS_STATUS_READY) {
        draw_string(120, 250, "Filesystem not ready!", COLOR_STATUS_ERROR);
        return;
    }
    
    // Clear directory listing area
    draw_rect(100, 200, 600, 300, COLOR_BACKGROUND);
    
    // Draw header
    draw_string(110, 210, "Directory Listing:", COLOR_FILE_NORMAL);
    draw_string(110, 225, "Name                Size      Attr  Description", COLOR_FILE_NORMAL);
    draw_string(110, 235, "----                ----      ----  -----------", COLOR_FILE_NORMAL);
    
    int y = 250;
    for (int i = 0; mock_files[i].name[0] != 0; i++) {
        if (y > 480) break; // Don't overflow display
        
        uint32_t color = get_file_color(mock_files[i].attributes);
        
        // Draw filename
        draw_string(110, y, mock_files[i].name, color);
        
        // Draw file size
        char size_str[16];
        format_file_size(mock_files[i].size, size_str);
        draw_string(270, y, size_str, color);
        
        // Draw attributes
        char attr_str[8] = "------";
        if (mock_files[i].attributes & FS_ATTR_READ_ONLY) attr_str[0] = 'R';
        if (mock_files[i].attributes & FS_ATTR_HIDDEN) attr_str[1] = 'H';
        if (mock_files[i].attributes & FS_ATTR_SYSTEM) attr_str[2] = 'S';
        if (mock_files[i].attributes & FS_ATTR_DIRECTORY) attr_str[3] = 'D';
        if (mock_files[i].attributes & FS_ATTR_ARCHIVE) attr_str[4] = 'A';
        draw_string(330, y, attr_str, color);
        
        // Draw description
        if (mock_files[i].content) {
            draw_string(390, y, mock_files[i].content, color);
        }
        
        y += 15;
    }
    
    // Draw footer with summary
    char summary[64];
    sprintf(summary, "Total: %u files, %u bytes", total_files, total_size);
    draw_string(110, y + 10, summary, COLOR_FILE_NORMAL);
}

// Find file by name
int find_file(const char* filename, mock_file_t** file) {
    if (fs_status != FS_STATUS_READY || !filename || !file) {
        return -1;
    }
    
    for (int i = 0; mock_files[i].name[0] != 0; i++) {
        if (strcmp(mock_files[i].name, filename) == 0) {
            *file = &mock_files[i];
            return i;
        }
    }
    
    return -1; // File not found
}

// Read file content (mock implementation)
int read_file(const char* filename, char* buffer, uint32_t max_size) {
    mock_file_t* file;
    int index = find_file(filename, &file);
    
    if (index < 0) {
        return -1; // File not found
    }
    
    if (file->attributes & FS_ATTR_DIRECTORY) {
        return -2; // Cannot read directory as file
    }
    
    // Mock file reading - in real system would read from disk
    if (file->content && buffer) {
        uint32_t copy_size = strlen(file->content);
        if (copy_size > max_size - 1) {
            copy_size = max_size - 1;
        }
        strncpy(buffer, file->content, copy_size);
        buffer[copy_size] = '\0';
        return copy_size;
    }
    
    return 0;
}

// Get filesystem status
fs_status_t get_filesystem_status(void) {
    return fs_status;
}

// Get file count
uint32_t get_file_count(void) {
    return total_files;
}

// Get total filesystem size
uint32_t get_total_size(void) {
    return total_size;
}
