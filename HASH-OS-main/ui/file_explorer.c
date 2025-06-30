/ file_explorer.c
#include "file_explorer.h"
#include "../drivers/display4k.h"
#include "../filesystem/filesystem.h"
#include "animations.h"
#include <string.h>
#include <stdio.h>

static file_explorer_state_t explorer_state;
static int context_menu_open = 0;
static int context_menu_x = 0, context_menu_y = 0;

// File type icons (Unicode emojis)
static const char* file_icons[] = {
    "📁", // Folder
    "📄", // Text
    "🖼️", // Image
    "🎬", // Video
    "🎵", // Audio
    "⚙️", // Executable
    "📋"  // Unknown
};

void file_explorer_init(void) {
    memset(&explorer_state, 0, sizeof(file_explorer_state_t));
    strcpy(explorer_state.current_path, "/");
    explorer_state.view_mode = 0; // List view
    explorer_state.sort_mode = 0; // Sort by name
    file_explorer_refresh();
}

void file_explorer_refresh(void) {
    // Simulate loading files (in real implementation, use filesystem API)
    explorer_state.file_count = 0;
    
    // Add some sample files
    strcpy(explorer_state.files[0].name, "..");
    strcpy(explorer_state.files[0].path, "/");
    explorer_state.files[0].type = FILE_TYPE_FOLDER;
    explorer_state.files[0].size = 0;
    explorer_state.file_count++;
    
    strcpy(explorer_state.files[1].name, "Documents");
    strcpy(explorer_state.files[1].path, "/Documents");
    explorer_state.files[1].type = FILE_TYPE_FOLDER;
    explorer_state.files[1].size = 0;
    explorer_state.file_count++;
    
    strcpy(explorer_state.files[2].name, "Pictures");
    strcpy(explorer_state.files[2].path, "/Pictures");
    explorer_state.files[2].type = FILE_TYPE_FOLDER;
    explorer_state.files[2].size = 0;
    explorer_state.file_count++;
    
    strcpy(explorer_state.files[3].name, "readme.txt");
    strcpy(explorer_state.files[3].path, "/readme.txt");
    explorer_state.files[3].type = FILE_TYPE_TEXT;
    explorer_state.files[3].size = 1024;
    explorer_state.file_count++;
    
    strcpy(explorer_state.files[4].name, "wallpaper.png");
    strcpy(explorer_state.files[4].path, "/wallpaper.png");
    explorer_state.files[4].type = FILE_TYPE_IMAGE;
    explorer_state.files[4].size = 2048576;
    explorer_state.file_count++;
}

void draw_file_icon(int x, int y, file_type_t type, int selected) {
    uint32_t bg_color = selected ? 0x0066CC : 0x333333;
    uint32_t text_color = selected ? 0xFFFFFF : 0xCCCCCC;
    
    if (selected) {
        draw_rounded_rect(x - 5, y - 5, 70, 70, 10, bg_color);
        animate_pulse_notification(x + 30, y + 30, 35);
    }
    
    // Draw icon background
    draw_rounded_rect(x, y, 60, 60, 8, selected ? 0x0088FF : 0x555555);
    
    // Draw file type icon (simplified - in real implementation use actual icons)
    const char* icon = file_icons[type];
    draw_string(x + 20, y + 20, icon, text_color);
}

void draw_file_list(void) {
    int y_offset = 120;
    int visible_files = (SCREEN_HEIGHT - 200) / 50;
    
    for (int i = explorer_state.scroll_offset; 
         i < explorer_state.file_count && i < explorer_state.scroll_offset + visible_files; 
         i++) {
        
        int y = y_offset + (i - explorer_state.scroll_offset) * 50;
        int selected = (i == explorer_state.selected_file);
        
        if (selected) {
            draw_rect(50, y - 5, SCREEN_WIDTH - 100, 40, 0x0066CC);
            animate_bounce_icon(60, y, 30, 30);
        }
        
        // Draw file icon
        draw_file_icon(60, y, explorer_state.files[i].type, selected);
        
        // Draw file name
        uint32_t text_color = selected ? 0xFFFFFF : 0xCCCCCC;
        draw_string(130, y + 15, explorer_state.files[i].name, text_color);
        
        // Draw file size for non-folders
        if (explorer_state.files[i].type != FILE_TYPE_FOLDER) {
            char size_str[32];
            if (explorer_state.files[i].size > 1024 * 1024) {
                sprintf(size_str, "%.1f MB", explorer_state.files[i].size / (1024.0 * 1024.0));
            } else if (explorer_state.files[i].size > 1024) {
                sprintf(size_str, "%.1f KB", explorer_state.files[i].size / 1024.0);
            } else {
                sprintf(size_str, "%llu bytes", explorer_state.files[i].size);
            }
            draw_string(SCREEN_WIDTH - 200, y + 15, size_str, 0x888888);
        }
    }
}

void draw_file_grid(void) {
    int cols = (SCREEN_WIDTH - 100) / 120;
    int x_start = 50;
    int y_start = 120;
    
    for (int i = explorer_state.scroll_offset; i < explorer_state.file_count; i++) {
        int col = (i - explorer_state.scroll_offset) % cols;
        int row = (i - explorer_state.scroll_offset) / cols;
        
        int x = x_start + col * 120;
        int y = y_start + row * 100;
        
        if (y > SCREEN_HEIGHT - 150) break;
        
        int selected = (i == explorer_state.selected_file);
        
        // Draw file icon
        draw_file_icon(x, y, explorer_state.files[i].type, selected);
        
        // Draw file name (truncated if necessary)
        char display_name[20];
        strncpy(display_name, explorer_state.files[i].name, 15);
        if (strlen(explorer_state.files[i].name) > 15) {
            strcpy(display_name + 12, "...");
        }
        
        uint32_t text_color = selected ? 0xFFFFFF : 0xCCCCCC;
        draw_string(x, y + 65, display_name, text_color);
    }
}

void draw_breadcrumb_nav(void) {
    draw_rect(0, 0, SCREEN_WIDTH, 60, 0x222222);
    draw_string(20, 20, "📍 Path:", 0xFFFFFF);
    draw_string(100, 20, explorer_state.current_path, 0x00AAFF);
    
    // View mode toggle
    const char* view_text = explorer_state.view_mode ? "Grid View" : "List View";
    draw_string(SCREEN_WIDTH - 150, 20, view_text, 0xFFFFFF);
}

void draw_status_bar(void) {
    draw_rect(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40, 0x222222);
    
    char status[128];
    sprintf(status, "%d items", explorer_state.file_count);
    draw_string(20, SCREEN_HEIGHT - 25, status, 0xCCCCCC);
    
    if (explorer_state.selected_file >= 0 && explorer_state.selected_file < explorer_state.file_count) {
        sprintf(status, "Selected: %s", explorer_state.files[explorer_state.selected_file].name);
        draw_string(200, SCREEN_HEIGHT - 25, status, 0x00AAFF);
    }
}

void draw_context_menu(int x, int y) {
    if (!context_menu_open) return;
    
    const char* menu_items[] = {
        "Open",
        "Copy",
        "Cut",
        "Delete",
        "Properties"
    };
    int item_count = 5;
    int menu_width = 120;
    int menu_height = item_count * 30;
    
    // Animate menu opening
    animate_window_open(x, y, menu_width, menu_height);
    
    draw_rounded_rect(x, y, menu_width, menu_height, 5, 0x333333);
    draw_rect(x + 1, y + 1, menu_width - 2, menu_height - 2, 0x444444);
    
    for (int i = 0; i < item_count; i++) {
        int item_y = y + i * 30;
        draw_string(x + 10, item_y + 10, menu_items[i], 0xFFFFFF);
    }
}

void file_explorer_ui_loop(void) {
    // Clear screen with dark background
    clear_screen(0x111111);
    
    // Draw UI components
    draw_breadcrumb_nav();
    
    if (explorer_state.view_mode == 0) {
        draw_file_list();
    } else {
        draw_file_grid();
    }
    
    draw_status_bar();
    draw_context_menu(context_menu_x, context_menu_y);
    
    // Update animations
    update_animations();
}

void file_explorer_handle_input(int key, int x, int y) {
    switch (key) {
        case 1: // Up arrow
            if (explorer_state.selected_file > 0) {
                explorer_state.selected_file--;
                animate_bounce_icon(60, 120 + explorer_state.selected_file * 50, 30, 30);
            }
            break;
            
        case 2: // Down arrow
            if (explorer_state.selected_file < explorer_state.file_count - 1) {
                explorer_state.selected_file++;
                animate_bounce_icon(60, 120 + explorer_state.selected_file * 50, 30, 30);
            }
            break;
            
        case 3: // Enter
            file_explorer_open_file(explorer_state.selected_file);
            break;
            
        case 4: // Tab (toggle view mode)
            explorer_state.view_mode = !explorer_state.view_mode;
            animate_fade_transition(0, 60, SCREEN_WIDTH, SCREEN_HEIGHT - 100, 0x111111, 0x111111);
            break;
            
        case 5: // Right click (context menu)
            context_menu_open = 1;
            context_menu_x = x;
            context_menu_y = y;
            break;
    }
}

void file_explorer_open_file(int index) {
    if (index < 0 || index >= explorer_state.file_count) return;
    
    file_entry_t* file = &explorer_state.files[index];
    
    if (file->type == FILE_TYPE_FOLDER) {
        // Navigate to folder
        file_explorer_navigate_to(file->path);
        animate_slide_transition(0, 0, -SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    } else {
        // Open file with appropriate application
        animate_icon_press(60, 120 + index * 50, 30, 30);
        // TODO: Launch appropriate application
    }
}

void file_explorer_navigate_to(const char* path) {
    strcpy(explorer_state.current_path, path);
    explorer_state.selected_file = 0;
    explorer_state.scroll_offset = 0;
    file_explorer_refresh();
}