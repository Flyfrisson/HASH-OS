#include "fs.h"
#include "../drivers/display4k.h"

char *mock_files[] = {
    "bootldr.sys",
    "kernel.bin",
    "config.cfg",
    "readme.txt",
    "HASHOS.sys",
    0
};

void init_filesystem() {
    draw_rect(100, 100, 300, 100, 0x3366FF); // Filesystem Ready Visual
}

void list_root_directory() {
    int y = 250;

    for (int i = 0; mock_files[i] != 0; i++) {
        draw_string(120, y, mock_files[i], 0xFFFFFF);
        y += 20;
    }
}
