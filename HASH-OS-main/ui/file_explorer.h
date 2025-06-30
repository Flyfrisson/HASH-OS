#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

#include <stdint.h>

#define MAX_FILES 256
#define MAX_FILENAME_LEN 256
#define MAX_PATH_LEN 1024

typedef enum {
    FILE_TYPE_FOLDER = 0,
    FILE_TYPE_TEXT = 1,
    FILE_TYPE_IMAGE = 2,
    FILE_TYPE_VIDEO = 3,
    FILE_TYPE_AUDIO = 4,
    FILE_TYPE_EXECUTABLE = 5,
    FILE_TYPE_UNKNOWN = 6
} file_type_t;

typedef struct {
    char name[MAX_FILENAME_LEN];
    char path[MAX_PATH_LEN];
    file_type_t type;
    uint64_t size;
    uint64_t modified_time;
    uint32_t permissions;
    int selected;
} file_entry_t;

typedef struct {
    char current_path[MAX_PATH_LEN];
    file_entry_t files[MAX_FILES];
    int file_count;
    int selected_file;
    int scroll_offset;
    int view_mode; // 0 = list, 1 = grid
    int sort_mode; // 0 = name, 1 = size, 2 = date
} file_explorer_state_t;

// File explorer functions
void file_explorer_init(void);
void file_explorer_ui_loop(void);
void file_explorer_handle_input(int key, int x, int y);
void file_explorer_navigate_to(const char* path);
void file_explorer_refresh(void);
void file_explorer_open_file(int index);
void file_explorer_create_folder(const char* name);
void file_explorer_delete_file(int index);
void file_explorer_copy_file(int index);
void file_explorer_paste_file(void);

// UI rendering functions
void draw_file_list(void);
void draw_file_grid(void);
void draw_file_icon(int x, int y, file_type_t type, int selected);
void draw_breadcrumb_nav(void);
void draw_status_bar(void);
void draw_context_menu(int x, int y);

#endif