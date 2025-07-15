// ui_manager.h - UI Manager Header File
// =============================================================================
#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

// Screen dimensions (4K resolution)
#define SCREEN_WIDTH 3840
#define SCREEN_HEIGHT 2160
#define SCREEN_BPP 32

// Color definitions
#define COLOR_BLACK     0x000000
#define COLOR_WHITE     0xFFFFFF
#define COLOR_GRAY      0x808080
#define COLOR_DARK_GRAY 0x222222
#define COLOR_GREEN     0x00FF00
#define COLOR_BLUE      0x0000FF
#define COLOR_RED       0xFF0000

// UI State
typedef enum {
    UI_STATE_HOME,
    UI_STATE_APP_RUNNING,
    UI_STATE_SETTINGS,
    UI_STATE_SHUTDOWN
} ui_state_t;

typedef struct {
    bool is_initialized;
    ui_state_t current_state;
    int last_touch_x;
    int last_touch_y;
    uint32_t frame_count;
} ui_context_t;

// Function declarations
bool init_ui(void);
void cleanup_ui(void);
void render_home_screen(void);
void handle_touch_event(int x, int y);
void handle_keypress_event(char key);
void ui_main_loop(void);
void set_ui_state(ui_state_t new_state);
ui_state_t get_ui_state(void);

#endif // UI_MANAGER_H
