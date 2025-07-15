#ifndef SPLASH_H
#define SPLASH_H

#include <stdint.h>
#include <stdbool.h>

// Basic splash screen functions
void render_splash_screen();

// Enhanced splash screen functions
void render_enhanced_splash_screen();
void render_splash_with_logo();
void render_splash_fade_out(int fade_level);

// Splash screen components
void draw_boot_progress_bar();
void draw_loading_animation();
void draw_logo();

// Splash screen management
void init_splash_screen();
void update_boot_progress(int progress);
bool splash_screen_timeout();

// System time function (implement in your system)
uint32_t get_system_time();

#endif // SPLASH_H
