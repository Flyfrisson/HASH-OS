// =============================================================================
// ui_manager.c - UI Manager Implementation
// =============================================================================
#include "ui_manager.h"
#include "drivers/display4k.h"
#include "drivers/touch_input.h"
#include "drivers/virtual_keyboard.h"
#include "launcher.h"
#include <stdio.h>
#include <string.h>

// Global UI context
static ui_context_t g_ui_context = {0};

// Initialize UI system
bool init_ui(void) {
    printf("Initializing UI system...\n");
    
    // Initialize display driver
    if (!init_display4k()) {
        printf("Failed to initialize display driver\n");
        return false;
    }
    
    // Initialize touch input
    if (!init_touch_input()) {
        printf("Failed to initialize touch input\n");
        cleanup_display4k();
        return false;
    }
    
    // Initialize virtual keyboard
    if (!init_virtual_keyboard()) {
        printf("Failed to initialize virtual keyboard\n");
        cleanup_touch_input();
        cleanup_display4k();
        return false;
    }
    
    // Initialize launcher
    if (!init_launcher()) {
        printf("Failed to initialize launcher\n");
        cleanup_virtual_keyboard();
        cleanup_touch_input();
        cleanup_display4k();
        return false;
    }
    
    // Set initial state
    g_ui_context.is_initialized = true;
    g_ui_context.current_state = UI_STATE_HOME;
    g_ui_context.last_touch_x = -1;
    g_ui_context.last_touch_y = -1;
    g_ui_context.frame_count = 0;
    
    // Render initial screen
    render_home_screen();
    
    printf("UI system initialized successfully\n");
    return true;
}

// Cleanup UI system
void cleanup_ui(void) {
    if (!g_ui_context.is_initialized) return;
    
    printf("Cleaning up UI system...\n");
    
    cleanup_launcher();
    cleanup_virtual_keyboard();
    cleanup_touch_input();
    cleanup_display4k();
    
    g_ui_context.is_initialized = false;
    printf("UI system cleaned up\n");
}

// Render the home screen layout
void render_home_screen(void) {
    if (!g_ui_context.is_initialized) return;
    
    // Clear screen with dark background
    clear_screen(COLOR_DARK_GRAY);
    
    // Draw status bar
    draw_filled_rectangle(0, 0, SCREEN_WIDTH, 60, COLOR_BLACK);
    draw_string(20, 20, "Home Screen", COLOR_WHITE);
    
    // Draw frame counter (for debugging)
    char frame_str[32];
    snprintf(frame_str, sizeof(frame_str), "Frame: %u", g_ui_context.frame_count);
    draw_string(SCREEN_WIDTH - 200, 20, frame_str, COLOR_GREEN);
    
    // Draw launcher icons
    draw_launcher_icons();
    
    // Draw virtual keyboard if visible
    if (is_virtual_keyboard_visible()) {
        render_virtual_keyboard();
    }
    
    // Refresh display
    refresh_screen();
    g_ui_context.frame_count++;
}

// Handle touch inputs at UI level
void handle_touch_event(int x, int y) {
    if (!g_ui_context.is_initialized) return;
    
    // Validate coordinates
    if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        return;
    }
    
    // Store last touch position
    g_ui_context.last_touch_x = x;
    g_ui_context.last_touch_y = y;
    
    // Handle based on current state
    switch (g_ui_context.current_state) {
        case UI_STATE_HOME:
            // Check if virtual keyboard is visible first
            if (is_virtual_keyboard_visible()) {
                // Handle virtual keyboard touch
                char key = get_virtual_key();
                if (key != 0) {
                    handle_keypress_event(key);
                }
            } else {
                // Handle launcher touch
                handle_launcher_touch(x, y);
            }
            break;
            
        case UI_STATE_APP_RUNNING:
            // Forward to running app (placeholder)
            break;
            
        case UI_STATE_SETTINGS:
            // Handle settings touch (placeholder)
            break;
            
        default:
            break;
    }
}

// Handle keypress events at UI level
void handle_keypress_event(char key) {
    if (!g_ui_context.is_initialized || key == 0) return;
    
    // Display pressed key at top corner
    char message[64];
    snprintf(message, sizeof(message), "Key: %c", key);
    draw_string(300, 20, message, COLOR_GREEN);
    
    // Handle special keys
    switch (key) {
        case 27: // ESC key
            if (g_ui_context.current_state != UI_STATE_HOME) {
                set_ui_state(UI_STATE_HOME);
                render_home_screen();
            }
            break;
            
        case 'k': // Toggle virtual keyboard
        case 'K':
            show_virtual_keyboard(!is_virtual_keyboard_visible());
            render_home_screen();
            break;
            
        default:
            // Handle other keys as needed
            break;
    }
}

// Set UI state
void set_ui_state(ui_state_t new_state) {
    if (g_ui_context.current_state != new_state) {
        g_ui_context.current_state = new_state;
        printf("UI state changed to: %d\n", new_state);
    }
}

// Get current UI state
ui_state_t get_ui_state(void) {
    return g_ui_context.current_state;
}

// Main UI Event Loop
void ui_main_loop(void) {
    if (!g_ui_context.is_initialized) {
        printf("UI not initialized!\n");
        return;
    }
    
    printf("Starting UI main loop...\n");
    
    int touch_x = -1, touch_y = -1;
    
    while (g_ui_context.current_state != UI_STATE_SHUTDOWN) {
        // Handle touch input
        if (get_touch_input(&touch_x, &touch_y)) {
            handle_touch_event(touch_x, touch_y);
        }
        
        // Handle keyboard input
        char key = get_virtual_key();
        if (key != 0) {
            handle_keypress_event(key);
        }
        
        // Render current screen based on state
        switch (g_ui_context.current_state) {
            case UI_STATE_HOME:
                render_home_screen();
                break;
                
            case UI_STATE_APP_RUNNING:
                // Render running app (placeholder)
                break;
                
            case UI_STATE_SETTINGS:
                // Render settings screen (placeholder)
                break;
                
            default:
                break;
        }
        
        // Small delay to prevent excessive CPU usage
        // In a real system, this would be replaced with proper event handling
        usleep(16666); // ~60 FPS (16.67ms delay)
    }
    
    printf("UI main loop ended\n");
}
