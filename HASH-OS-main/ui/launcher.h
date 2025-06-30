#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <stdint.h>

#define MAX_LAUNCHER_APPS 32
#define LAUNCHER_COLS 4
#define LAUNCHER_ROWS 8

typedef struct {
    char name[64];
    char icon_path[256];
    char executable_path[256];
    uint32_t icon_color;
    int x, y;
    int width, height;
    int visible;
    int pinned;
} launcher_app_t;

typedef struct {
    launcher_app_t apps[MAX_LAUNCHER_APPS];
    int app_count;
    int selected_app;
    int scroll_offset;
    int search_mode;
    char search_query[128];
    int animation_frame;
} launcher_state_t;

// Launcher functions
void launcher_init(void);
void launcher_ui_loop(void);
void launcher_handle_input(int key, int x, int y);
void launcher_add_app(const char* name, const char* icon_path, const char* executable_path);
void launcher_remove_app(int index);
void launcher_launch_app(int index);
void launcher_search_apps(const char* query);

// UI functions
void draw_launcher_grid(void);
void draw_launcher_dock(void);
void draw_search_bar(void);
void draw_app_icon(int index, int x, int y, int selected);
void handle_launcher_touch(int x, int y);

#endif
