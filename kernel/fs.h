#ifndef HASHOS_FS_H
#define HASHOS_FS_H

#include <stdint.h>

typedef struct {
    unsigned char filename[11];
    unsigned short starting_cluster;
    unsigned int file_size;
} DirectoryEntry;

int init_filesystem(void);              // 🔧 Return type must match fs.c
void list_root_directory(void);
void filesystem_background_loop(void);

#endif
