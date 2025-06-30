#include "fat.h"

// Dummy sector reader for now
// BIOS disk reads are not supported inside protected mode kernel

unsigned char read_sector(unsigned int lba, unsigned char* buffer) {
    // In a real system: read from disk using ATA PIO or pre-loaded data.
    // For now: just simulate success and leave buffer unchanged.

    (void)lba;    // Suppress unused parameter warning
    (void)buffer; // Suppress unused parameter warning

    return 1; // Always return success
}
