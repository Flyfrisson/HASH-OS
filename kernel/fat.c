#include "fat.h"
#include <string.h>

// Error codes for disk operations
#define DISK_SUCCESS        1
#define DISK_ERROR_INVALID  0
#define DISK_ERROR_TIMEOUT  2
#define DISK_ERROR_NOT_READY 3

// Simulated disk parameters
#define SECTORS_PER_TRACK   63
#define HEADS_PER_CYLINDER  16
#define BYTES_PER_SECTOR    512
#define MAX_LBA_ADDRESS     0x1FFFFF  // ~1GB simulated disk

// Global variables for disk state
static unsigned char disk_ready = 1;
static unsigned int total_sectors = 0x200000; // 1GB worth of sectors

// Logging function (would be implemented elsewhere)
void disk_log(const char* message);

// Convert LBA to CHS (Cylinder, Head, Sector) - for educational purposes
void lba_to_chs(unsigned int lba, unsigned int* cylinder, 
                unsigned int* head, unsigned int* sector) {
    *sector = (lba % SECTORS_PER_TRACK) + 1;
    *head = (lba / SECTORS_PER_TRACK) % HEADS_PER_CYLINDER;
    *cylinder = lba / (SECTORS_PER_TRACK * HEADS_PER_CYLINDER);
}

// Simulate disk delay (in real system, this would be actual I/O wait)
void simulate_disk_delay(void) {
    // Simple delay loop to simulate disk access time
    for (volatile int i = 0; i < 10000; i++) {
        // Simulate ~1ms delay
    }
}

// Enhanced sector reader with proper error handling and validation
unsigned char read_sector(unsigned int lba, unsigned char* buffer) {
    // Input validation
    if (buffer == NULL) {
        disk_log("ERROR: NULL buffer provided to read_sector");
        return DISK_ERROR_INVALID;
    }
    
    if (lba >= MAX_LBA_ADDRESS || lba >= total_sectors) {
        disk_log("ERROR: LBA address out of range");
        return DISK_ERROR_INVALID;
    }
    
    // Check if disk is ready
    if (!disk_ready) {
        disk_log("ERROR: Disk not ready");
        return DISK_ERROR_NOT_READY;
    }
    
    // Log the operation (can be disabled in production)
    #ifdef DEBUG_DISK_IO
    char log_msg[64];
    snprintf(log_msg, sizeof(log_msg), "Reading sector LBA: %u", lba);
    disk_log(log_msg);
    #endif
    
    // Convert LBA to CHS for educational purposes
    unsigned int cylinder, head, sector;
    lba_to_chs(lba, &cylinder, &head, &sector);
    
    // Simulate disk access delay
    simulate_disk_delay();
    
    // In a real implementation, this would be one of:
    // 1. ATA PIO mode disk read
    // 2. AHCI SATA interface
    // 3. NVMe interface
    // 4. Pre-loaded ramdisk access
    // 5. Network block device
    
    // For now: Fill buffer with simulated data based on LBA
    // This creates predictable test data for filesystem testing
    for (int i = 0; i < BYTES_PER_SECTOR; i++) {
        // Create pattern based on LBA and offset
        buffer[i] = (unsigned char)((lba + i) & 0xFF);
    }
    
    // Simulate occasional read errors (1 in 10000 chance)
    #ifdef SIMULATE_DISK_ERRORS
    static unsigned int error_counter = 0;
    error_counter++;
    if (error_counter % 10000 == 0) {
        disk_log("Simulated disk read error");
        return DISK_ERROR_TIMEOUT;
    }
    #endif
    
    return DISK_SUCCESS;
}

// Additional utility functions for disk management

// Initialize disk subsystem
unsigned char init_disk(void) {
    disk_log("Initializing disk subsystem...");
    
    // In real system: probe for disk controllers, identify drives
    // Reset disk controller, set up DMA if available
    
    disk_ready = 1;
    disk_log("Disk subsystem initialized successfully");
    return DISK_SUCCESS;
}

// Read multiple consecutive sectors (more efficient than single reads)
unsigned char read_sectors(unsigned int start_lba, unsigned int count, 
                          unsigned char* buffer) {
    if (buffer == NULL || count == 0) {
        return DISK_ERROR_INVALID;
    }
    
    for (unsigned int i = 0; i < count; i++) {
        unsigned char result = read_sector(start_lba + i, 
                                         buffer + (i * BYTES_PER_SECTOR));
        if (result != DISK_SUCCESS) {
            return result;
        }
    }
    
    return DISK_SUCCESS;
}

// Get disk information
void get_disk_info(unsigned int* total_sectors_out, 
                   unsigned int* bytes_per_sector_out) {
    if (total_sectors_out) {
        *total_sectors_out = total_sectors;
    }
    if (bytes_per_sector_out) {
        *bytes_per_sector_out = BYTES_PER_SECTOR;
    }
}

// Placeholder logging implementation
void disk_log(const char* message) {
    // In real system: write to serial port, debug console, or log buffer
    // For now: do nothing or write to memory buffer
    (void)message; // Suppress unused parameter warning
}
