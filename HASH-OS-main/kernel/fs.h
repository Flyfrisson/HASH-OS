#ifndef HASHOS_FS_H
#define HASHOS_FS_H

typedef struct {
    unsigned char filename[11];
    unsigned short starting_cluster;
    unsigned int file_size;
} DirectoryEntry;

void init_filesystem();
void list_root_directory();
void filesystem_background_loop();


#endif
