#include "launcher.h"
#include "../drivers/display.h"
#include "../kernel/app_manager.h"
#include "../input/touch.h"
#include "animations.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

// Mobile-specific constants
#define MOBILE_ICON_SIZE 80
#define MOBILE_ICON_SPACING 20
#define MOBILE_GRID_COLS 4
#define MOBILE_GRID_ROWS 6
#define MOBILE_DOCK_HEIGHT 100
#define MOBILE_STATUS_BAR_HEIGHT 30
#define MOBILE_SEARCH_HEIGHT 50
#define MOBILE_PAGE_INDICATOR_HEIGHT 20

// Touch sensitivity
#define TOUCH_THRESHOLD 10
#define SWIPE_THRESHOLD 100
#define LONG_PRESS_DURATION 500

// Colors optimized for mobile displays
#define COLOR_BG_PRIMARY 0x000000
#define COLOR_BG_SECONDARY 0x1A1A1A
#define COLOR_ICON_NORMAL 0x2C2C2E
#define COLOR_ICON_SELECTED 0x007AFF
#define COLOR_DOCK_BG 0x1C1C1E
#define COLOR_TEXT_PRIMARY 0xFFFFFF
#define COLOR_TEXT_SECONDARY 0x8E8E93
#define COLOR_SEARCH_BG 0x2C2C2E
#define COLOR_ACCENT 0x007AFF

static launcher_state_t launcher_state;
static touch_state_t last_touch;
static int current_page = 0;
static int total_pages = 0;
static int drag_offset_x = 0;
static bool is_dragging = false;
static bool is_long_pressing = false;
static int long_press_timer = 0;

void launcher_init(void) {
    memset(&launcher_state, 0, sizeof(launcher_state_t));
    
    // Initialize mobile-specific state
    launcher_state.grid_cols = MOBILE_GRID_COLS;
    launcher_state.grid_rows = MOBILE_GRID_ROWS;
    launcher_state.icon_size = MOBILE_ICON_SIZE;
    
    // Add essential mobile apps
    launcher_add_app("Phone", "/icons/phone.png", "/apps/phone");
    launcher_add_app("Messages", "/icons/messages.png", "/apps/messages");
    launcher_add_app("Mail", "/icons/mail.png", "/apps/mail");
    launcher_add_app("Safari", "/icons/safari.png", "/apps/safari");
    launcher_add_app("Camera", "/icons/camera.png", "/apps/camera");
    launcher_add_app("Photos", "/icons/photos.png", "/apps/photos");
    launcher_add_app("Maps", "/icons/maps.png", "/apps/maps");
    launcher_add_app("Weather", "/icons/weather.png", "/apps/weather");
    launcher_add_app("Clock", "/icons/clock.png", "/apps/clock");
    launcher_add_app("Calculator", "/icons/calc.png", "/apps/calculator");
    launcher_add_app("Settings", "/icons/settings.png", "/apps/settings");
    launcher_add_app("Files", "/icons/files.png", "/apps/files");
    launcher_add_app("Music", "/icons/music.png", "/apps/music");
    launcher_add_app("Notes", "/icons/notes.png", "/apps/notes");
    launcher_add_app("Contacts", "/icons/contacts.png", "/apps/contacts");
    launcher_add_app("App Store", "/icons/appstore.png", "/apps/appstore");
    
    // Calculate total pages
    int apps_per_page = MOBILE_GRID_COLS * MOBILE_GRID_ROWS;
    total_pages = (launcher_state.app_count + apps_per_page - 1) / apps_per_page;
}

int launcher_add_app(const char* name, const char* icon_path, const char* executable_path) {
    if (launcher_state.app_count >= MAX_LAUNCHER_APPS) return -1;
    
    launcher_app_t* app = &launcher_state.apps[launcher_state.app_count];
    
    // Safe string copying with bounds checking
    strncpy(app->name, name, MAX_APP_NAME_LENGTH - 1);
    app->name[MAX_APP_NAME_LENGTH - 1] = '\0';
    
    strncpy(app->icon_path, icon_path, MAX_PATH_LENGTH - 1);
    app->icon_path[MAX_PATH_LENGTH - 1] = '\0';
    
    strncpy(app->executable_path, executable_path, MAX_PATH_LENGTH - 1);
    app->executable_path[MAX_PATH_LENGTH - 1] = '\0';
    
    app->visible = 1;
    app->pinned = 0;
    app->width = MOBILE_ICON_SIZE;
    app->height = MOBILE_ICON_SIZE;
    
    // Generate attractive icon colors
    uint32_t colors[] = {0x007AFF, 0x34C759, 0xFF9500, 0xFF3B30, 0x5856D6, 0xFF2D92};
    app->icon_color = colors[launcher_state.app_count % 6];
    
    launcher_state.app_count++;
    return launcher_state.app_count - 1;
}

void draw_status_bar(void) {
    // Draw status bar background
    draw_rect(0, 0, SCREEN_WIDTH, MOBILE_STATUS_BAR_HEIGHT, COLOR_BG_SECONDARY);
    
    // Draw time (placeholder)
    draw_string(20, 8, "9:41", COLOR_TEXT_PRIMARY);
    
    // Draw battery and signal indicators
    draw_string(SCREEN_WIDTH - 80, 8, "100%", COLOR_TEXT_PRIMARY);
    draw_string(SCREEN_WIDTH - 40, 8, "📶", COLOR_TEXT_PRIMARY);
}

void draw_mobile_app_icon(int index, int x, int y, int selected, float scale) {
    if (index >= launcher_state.app_count) return;
    
    launcher_app_t* app = &launcher_state.apps[index];
    
    int icon_size = (int)(MOBILE_ICON_SIZE * scale);
    int icon_x = x + (MOBILE_ICON_SIZE - icon_size) / 2;
    int icon_y = y + (MOBILE_ICON_SIZE - icon_size) / 2;
    
    // Draw icon shadow for depth
    draw_rounded_rect(icon_x + 2, icon_y + 2, icon_size, icon_size, 
                     icon_size / 4, 0x000000AA);
    
    // Draw icon background
    uint32_t bg_color = selected ? COLOR_ICON_SELECTED : app->icon_color;
    draw_rounded_rect(icon_x, icon_y, icon_size, icon_size, 
                     icon_size / 4, bg_color);
    
    // Draw icon highlight
    draw_rounded_rect(icon_x + 2, icon_y + 2, icon_size - 4, icon_size / 3, 
                     icon_size / 4, 0xFFFFFF40);
    
    // Draw app name below icon
    int text_y = y + MOBILE_ICON_SIZE + 5;
    uint32_t text_color = selected ? COLOR_ACCENT : COLOR_TEXT_PRIMARY;
    
    // Center text under icon
    int text_width = strlen(app->name) * 6; // Approximate text width
    int text_x = x + (MOBILE_ICON_SIZE - text_width) / 2;
    draw_string(text_x, text_y, app->name, text_color);
    
    // Store position for touch handling
    app->x = x;
    app->y = y;
}

void draw_mobile_grid(void) {
    int start_x = (SCREEN_WIDTH - (MOBILE_GRID_COLS * (MOBILE_ICON_SIZE + MOBILE_ICON_SPACING) - MOBILE_ICON_SPACING)) / 2;
    int start_y = MOBILE_STATUS_BAR_HEIGHT + 40;
    
    int apps_per_page = MOBILE_GRID_COLS * MOBILE_GRID_ROWS;
    int page_start = current_page * apps_per_page;
    int page_end = MIN(page_start + apps_per_page, launcher_state.app_count);
    
    // Apply page transition offset
    int page_offset = drag_offset_x;
    
    for (int i = page_start; i < page_end; i++) {
        if (!launcher_state.apps[i].visible) continue;
        
        int app_index = i - page_start;
        int col = app_index % MOBILE_GRID_COLS;
        int row = app_index / MOBILE_GRID_COLS;
        
        int x = start_x + col * (MOBILE_ICON_SIZE + MOBILE_ICON_SPACING) + page_offset;
        int y = start_y + row * (MOBILE_ICON_SIZE + 30);
        
        // Skip icons that are off-screen during page transitions
        if (x < -MOBILE_ICON_SIZE || x > SCREEN_WIDTH) continue;
        
        int selected = (i == launcher_state.selected_app);
        float scale = selected && is_long_pressing ? 1.1f : 1.0f;
        
        draw_mobile_app_icon(i, x, y, selected, scale);
    }
}

void draw_page_indicators(void) {
    if (total_pages <= 1) return;
    
    int indicator_size = 6;
    int indicator_spacing = 12;
    int total_width = total_pages * indicator_size + (total_pages - 1) * (indicator_spacing - indicator_size);
    int start_x = (SCREEN_WIDTH - total_width) / 2;
    int y = SCREEN_HEIGHT - MOBILE_DOCK_HEIGHT - MOBILE_PAGE_INDICATOR_HEIGHT;
    
    for (int i = 0; i < total_pages; i++) {
        int x = start_x + i * indicator_spacing;
        uint32_t color = (i == current_page) ? COLOR_ACCENT : COLOR_TEXT_SECONDARY;
        draw_circle(x + indicator_size/2, y + indicator_size/2, indicator_size/2, color);
    }
}

void draw_dock(void) {
    int dock_y = SCREEN_HEIGHT - MOBILE_DOCK_HEIGHT;
    
    // Draw dock background with blur effect
    draw_rounded_rect(0, dock_y, SCREEN_WIDTH, MOBILE_DOCK_HEIGHT, 0, COLOR_DOCK_BG);
    
    // Draw dock separator line
    draw_line(0, dock_y, SCREEN_WIDTH, dock_y, COLOR_TEXT_SECONDARY);
    
    // Draw dock apps (first 4 apps are pinned to dock)
    int dock_apps = MIN(4, launcher_state.app_count);
    int dock_start_x = (SCREEN_WIDTH - (dock_apps * (MOBILE_ICON_SIZE + MOBILE_ICON_SPACING) - MOBILE_ICON_SPACING)) / 2;
    int dock_icon_y = dock_y + (MOBILE_DOCK_HEIGHT - MOBILE_ICON_SIZE) / 2;
    
    for (int i = 0; i < dock_apps; i++) {
        int x = dock_start_x + i * (MOBILE_ICON_SIZE + MOBILE_ICON_SPACING);
        int selected = (i == launcher_state.selected_app && current_page == 0);
        draw_mobile_app_icon(i, x, dock_icon_y, selected, 1.0f);
    }
}

void draw_search_interface(void) {
    if (!launcher_state.search_mode) return;
    
    // Draw search overlay
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x000000CC);
    
    // Draw search bar
    int search_y = MOBILE_STATUS_BAR_HEIGHT + 20;
    draw_rounded_rect(20, search_y, SCREEN_WIDTH - 40, MOBILE_SEARCH_HEIGHT, 
                     MOBILE_SEARCH_HEIGHT / 2, COLOR_SEARCH_BG);
    
    // Draw search icon and text
    draw_string(40, search_y + 15, "🔍", COLOR_TEXT_PRIMARY);
    draw_string(70, search_y + 15, launcher_state.search_query, COLOR_TEXT_PRIMARY);
    
    // Draw search results
    // TODO: Implement search results display
}

void launcher_ui_loop(void) {
    // Clear screen
    clear_screen(COLOR_BG_PRIMARY);
    
    // Draw status bar
    draw_status_bar();
    
    // Draw search interface if active
    if (launcher_state.search_mode) {
        draw_search_interface();
        return;
    }
    
    // Draw app grid
    draw_mobile_grid();
    
    // Draw page indicators
    draw_page_indicators();
    
    // Draw dock
    draw_dock();
    
    // Update animations
    update_animations();
    
    // Handle long press timing
    if (is_long_pressing) {
        long_press_timer++;
        if (long_press_timer > LONG_PRESS_DURATION) {
            // Enter edit mode
            launcher_state.edit_mode = 1;
        }
    }
}

void handle_launcher_touch(int x, int y, touch_event_t event) {
    switch (event) {
        case TOUCH_DOWN:
            last_touch.x = x;
            last_touch.y = y;
            is_dragging = false;
            is_long_pressing = true;
            long_press_timer = 0;
            
            // Check if touch is on an app icon
            for (int i = 0; i < launcher_state.app_count; i++) {
                launcher_app_t* app = &launcher_state.apps[i];
                if (x >= app->x && x <= app->x + MOBILE_ICON_SIZE &&
                    y >= app->y && y <= app->y + MOBILE_ICON_SIZE) {
                    launcher_state.selected_app = i;
                    animate_touch_feedback(x, y);
                    break;
                }
            }
            break;
            
        case TOUCH_MOVE:
            if (abs(x - last_touch.x) > TOUCH_THRESHOLD || 
                abs(y - last_touch.y) > TOUCH_THRESHOLD) {
                is_long_pressing = false;
                
                // Handle horizontal swipe for page navigation
                if (abs(x - last_touch.x) > abs(y - last_touch.y)) {
                    is_dragging = true;
                    drag_offset_x = x - last_touch.x;
                    
                    // Limit drag offset
                    if (drag_offset_x > SCREEN_WIDTH / 2) drag_offset_x = SCREEN_WIDTH / 2;
                    if (drag_offset_x < -SCREEN_WIDTH / 2) drag_offset_x = -SCREEN_WIDTH / 2;
                }
            }
            break;
            
        case TOUCH_UP:
            is_long_pressing = false;
            
            if (is_dragging) {
                // Handle page swipe
                if (abs(drag_offset_x) > SWIPE_THRESHOLD) {
                    if (drag_offset_x > 0 && current_page > 0) {
                        current_page--;
                        animate_page_transition(-1);
                    } else if (drag_offset_x < 0 && current_page < total_pages - 1) {
                        current_page++;
                        animate_page_transition(1);
                    }
                }
                
                // Reset drag state
                is_dragging = false;
                drag_offset_x = 0;
            } else {
                // Handle app launch
                if (launcher_state.selected_app >= 0) {
                    launcher_app_t* app = &launcher_state.apps[launcher_state.selected_app];
                    app_manager_launch(app->executable_path);
                    animate_app_launch(app->x, app->y);
                }
            }
            break;
    }
}

void launcher_handle_gesture(gesture_t gesture) {
    switch (gesture) {
        case GESTURE_SWIPE_DOWN:
            // Show search
            launcher_state.search_mode = 1;
            animate_search_appear();
            break;
            
        case GESTURE_SWIPE_UP:
            // Hide search or show app drawer
            if (launcher_state.search_mode) {
                launcher_state.search_mode = 0;
                animate_search_disappear();
            }
            break;
            
        case GESTURE_PINCH_OUT:
            // Show overview/mission control
            launcher_state.overview_mode = 1;
            break;
            
        case GESTURE_PINCH_IN:
            // Hide overview
            launcher_state.overview_mode = 0;
            break;
    }
}

void launcher_update_search(const char* query) {
    strncpy(launcher_state.search_query, query, MAX_SEARCH_LENGTH - 1);
    launcher_state.search_query[MAX_SEARCH_LENGTH - 1] = '\0';
    
    // Filter apps based on search query
    for (int i = 0; i < launcher_state.app_count; i++) {
        launcher_app_t* app = &launcher_state.apps[i];
        if (strlen(query) == 0) {
            app->visible = 1;
        } else {
            app->visible = (strstr(app->name, query) != NULL);
        }
    }
}