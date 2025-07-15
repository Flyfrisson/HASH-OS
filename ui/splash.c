#include "splash.h"
#include "../drivers/display4k.h"
#include <stdint.h>

// Animation state variables
static int animation_frame = 0;
static int boot_progress = 0;
static uint32_t splash_start_time = 0;

// Colors for the splash screen
#define COLOR_BLACK     0x000000
#define COLOR_WHITE     0xFFFFFF
#define COLOR_BLUE      0x0078D4
#define COLOR_GRAY      0x808080
#define COLOR_GREEN     0x00FF00

// Screen dimensions (4K)
#define SCREEN_WIDTH    3840
#define SCREEN_HEIGHT   2160
#define CENTER_X        (SCREEN_WIDTH / 2)
#define CENTER_Y        (SCREEN_HEIGHT / 2)

// Basic splash screen (your original implementation)
void render_splash_screen() {
    clear_screen(COLOR_BLACK);
    draw_text(1200, 600, "HASH OS", COLOR_WHITE);
    draw_text(1200, 700, "Smartphone Edition", COLOR_WHITE);
    // Future: Add animated logo or boot sound
}

// Enhanced splash screen with animations and progress
void render_enhanced_splash_screen() {
    clear_screen(COLOR_BLACK);
    
    // Animated title with fade-in effect
    uint32_t title_color = COLOR_WHITE;
    if (animation_frame < 30) {
        // Fade in effect (simplified)
        title_color = COLOR_GRAY;
    }
    
    // Center the main title
    draw_text(CENTER_X - 200, CENTER_Y - 100, "HASH OS", title_color);
    draw_text(CENTER_X - 300, CENTER_Y - 50, "Smartphone Edition", COLOR_BLUE);
    
    // Version info
    draw_text(CENTER_X - 100, CENTER_Y + 50, "Version 1.0", COLOR_GRAY);
    
    // Boot progress bar
    draw_boot_progress_bar();
    
    // Loading text with dots animation
    draw_loading_animation();
    
    animation_frame++;
}

// Draw a boot progress bar
void draw_boot_progress_bar() {
    int bar_width = 400;
    int bar_height = 20;
    int bar_x = CENTER_X - (bar_width / 2);
    int bar_y = CENTER_Y + 150;
    
    // Progress bar background
    draw_rect(bar_x, bar_y, bar_width, bar_height, COLOR_GRAY);
    
    // Progress bar fill
    int progress_width = (bar_width * boot_progress) / 100;
    if (progress_width > 0) {
        draw_rect(bar_x, bar_y, progress_width, bar_height, COLOR_BLUE);
    }
    
    // Progress percentage text
    char progress_text[16];
    sprintf(progress_text, "%d%%", boot_progress);
    draw_text(CENTER_X - 20, bar_y + 30, progress_text, COLOR_WHITE);
}

// Animated loading text with dots
void draw_loading_animation() {
    char loading_text[32] = "Loading";
    int dots = (animation_frame / 15) % 4; // Change dots every 15 frames
    
    for (int i = 0; i < dots; i++) {
        strcat(loading_text, ".");
    }
    
    draw_text(CENTER_X - 80, CENTER_Y + 200, loading_text, COLOR_WHITE);
}

// Simple logo placeholder (geometric shapes)
void draw_logo() {
    int logo_size = 100;
    int logo_x = CENTER_X - (logo_size / 2);
    int logo_y = CENTER_Y - 200;
    
    // Simple geometric logo - a stylized "H"
    draw_rect(logo_x, logo_y, 20, logo_size, COLOR_BLUE);
    draw_rect(logo_x + 80, logo_y, 20, logo_size, COLOR_BLUE);
    draw_rect(logo_x + 20, logo_y + 40, 60, 20, COLOR_BLUE);
}

// Splash screen with logo
void render_splash_with_logo() {
    clear_screen(COLOR_BLACK);
    
    // Draw logo
    draw_logo();
    
    // Main text
    draw_text(CENTER_X - 200, CENTER_Y - 50, "HASH OS", COLOR_WHITE);
    draw_text(CENTER_X - 300, CENTER_Y, "Smartphone Edition", COLOR_BLUE);
    
    // Copyright info
    draw_text(CENTER_X - 150, SCREEN_HEIGHT - 100, "© 2025 HASH OS Team", COLOR_GRAY);
}

// Update boot progress (call this from your boot sequence)
void update_boot_progress(int progress) {
    if (progress >= 0 && progress <= 100) {
        boot_progress = progress;
    }
}

// Initialize splash screen timing
void init_splash_screen() {
    animation_frame = 0;
    boot_progress = 0;
    splash_start_time = get_system_time(); // You'll need to implement this
}

// Check if splash screen should timeout
bool splash_screen_timeout() {
    uint32_t current_time = get_system_time();
    return (current_time - splash_start_time) > 3000; // 3 seconds
}

// Smooth fade out transition
void render_splash_fade_out(int fade_level) {
    // fade_level: 0 = fully visible, 255 = fully faded
    uint32_t alpha = 255 - fade_level;
    uint32_t faded_white = (alpha << 24) | COLOR_WHITE;
    uint32_t faded_blue = (alpha << 24) | COLOR_BLUE;
    
    clear_screen(COLOR_BLACK);
    draw_text(CENTER_X - 200, CENTER_Y - 50, "HASH OS", faded_white);
    draw_text(CENTER_X - 300, CENTER_Y, "Smartphone Edition", faded_blue);
}
