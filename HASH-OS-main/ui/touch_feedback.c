#include "touch_feedback.h"
#include "../drivers/display4k.h"
#include "../drivers/audio_output.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Touch feedback configuration
#define MAX_TOUCH_EFFECTS 10
#define RIPPLE_MAX_RADIUS 50
#define RIPPLE_DURATION 30  // frames
#define FADE_DURATION 20    // frames

// Touch feedback types
typedef enum {
    TOUCH_NORMAL,
    TOUCH_BUTTON,
    TOUCH_LONG_PRESS,
    TOUCH_DRAG,
    TOUCH_ERROR,
    TOUCH_SUCCESS
} TouchFeedbackType;

// Touch effect structure
typedef struct {
    int x, y;                    // Position
    int current_radius;          // Current ripple radius
    int max_radius;             // Maximum ripple radius
    int duration;               // Frames remaining
    uint32_t color;             // Effect color
    TouchFeedbackType type;     // Type of feedback
    bool active;                // Is effect active
    int alpha;                  // Transparency (0-255)
} TouchEffect;

// Global touch effects array
static TouchEffect touch_effects[MAX_TOUCH_EFFECTS];
static bool feedback_enabled = true;
static bool haptic_enabled = true;
static bool sound_enabled = true;

// Color definitions
#define COLOR_NORMAL    0x00FFFF  // Cyan (your original)
#define COLOR_BUTTON    0x00FF00  // Green
#define COLOR_LONG      0xFFAA00  // Orange
#define COLOR_DRAG      0xFF00FF  // Magenta
#define COLOR_ERROR     0xFF0000  // Red
#define COLOR_SUCCESS   0x00FF00  // Green

// Your original touch feedback function
void show_touch_feedback(int x, int y) {
    draw_circle(x, y, 30, 0x00FFFF);
    play_touch_sound();
}

// Enhanced touch feedback with different types
void show_enhanced_touch_feedback(int x, int y, TouchFeedbackType type) {
    if (!feedback_enabled) return;
    
    // Find available effect slot
    int slot = find_available_effect_slot();
    if (slot == -1) return;
    
    // Initialize the effect
    TouchEffect* effect = &touch_effects[slot];
    effect->x = x;
    effect->y = y;
    effect->current_radius = 5;
    effect->type = type;
    effect->active = true;
    effect->alpha = 255;
    
    // Set type-specific properties
    switch (type) {
        case TOUCH_NORMAL:
            effect->max_radius = 30;
            effect->duration = RIPPLE_DURATION;
            effect->color = COLOR_NORMAL;
            if (sound_enabled) play_touch_sound();
            break;
            
        case TOUCH_BUTTON:
            effect->max_radius = 25;
            effect->duration = RIPPLE_DURATION - 10;
            effect->color = COLOR_BUTTON;
            if (sound_enabled) play_button_sound();
            break;
            
        case TOUCH_LONG_PRESS:
            effect->max_radius = 40;
            effect->duration = RIPPLE_DURATION + 10;
            effect->color = COLOR_LONG;
            if (sound_enabled) play_long_press_sound();
            break;
            
        case TOUCH_DRAG:
            effect->max_radius = 20;
            effect->duration = RIPPLE_DURATION - 15;
            effect->color = COLOR_DRAG;
            // No sound for drag to avoid continuous noise
            break;
            
        case TOUCH_ERROR:
            effect->max_radius = 35;
            effect->duration = RIPPLE_DURATION + 5;
            effect->color = COLOR_ERROR;
            if (sound_enabled) play_error_sound();
            break;
            
        case TOUCH_SUCCESS:
            effect->max_radius = 40;
            effect->duration = RIPPLE_DURATION + 10;
            effect->color = COLOR_SUCCESS;
            if (sound_enabled) play_success_sound();
            break;
    }
    
    // Trigger haptic feedback if enabled
    if (haptic_enabled) {
        trigger_haptic_feedback(type);
    }
}

// Update and render all active touch effects
void update_touch_effects() {
    for (int i = 0; i < MAX_TOUCH_EFFECTS; i++) {
        TouchEffect* effect = &touch_effects[i];
        
        if (!effect->active) continue;
        
        // Update effect animation
        effect->duration--;
        effect->current_radius += 2;
        
        // Fade out effect
        if (effect->duration < FADE_DURATION) {
            effect->alpha = (effect->duration * 255) / FADE_DURATION;
        }
        
        // Render the effect
        render_touch_effect(effect);
        
        // Deactivate finished effects
        if (effect->duration <= 0 || effect->current_radius > effect->max_radius) {
            effect->active = false;
        }
    }
}

// Render a single touch effect
void render_touch_effect(TouchEffect* effect) {
    if (!effect->active) return;
    
    // Calculate color with alpha
    uint32_t render_color = apply_alpha(effect->color, effect->alpha);
    
    switch (effect->type) {
        case TOUCH_NORMAL:
        case TOUCH_BUTTON:
            // Simple expanding circle
            draw_circle_outline(effect->x, effect->y, effect->current_radius, render_color, 3);
            break;
            
        case TOUCH_LONG_PRESS:
            // Double circle for long press
            draw_circle_outline(effect->x, effect->y, effect->current_radius, render_color, 2);
            draw_circle_outline(effect->x, effect->y, effect->current_radius - 10, render_color, 2);
            break;
            
        case TOUCH_DRAG:
            // Small filled circle for drag
            draw_circle(effect->x, effect->y, effect->current_radius / 2, render_color);
            break;
            
        case TOUCH_ERROR:
            // Pulsing red circle
            int pulse_radius = effect->current_radius + (effect->duration % 6) - 3;
            draw_circle_outline(effect->x, effect->y, pulse_radius, render_color, 4);
            break;
            
        case TOUCH_SUCCESS:
            // Expanding green circle with sparkle effect
            draw_circle_outline(effect->x, effect->y, effect->current_radius, render_color, 3);
            draw_sparkle_effect(effect->x, effect->y, effect->current_radius);
            break;
    }
}

// Find available effect slot
int find_available_effect_slot() {
    for (int i = 0; i < MAX_TOUCH_EFFECTS; i++) {
        if (!touch_effects[i].active) {
            return i;
        }
    }
    return -1; // No available slots
}

// Apply alpha transparency to color
uint32_t apply_alpha(uint32_t color, int alpha) {
    // Simple alpha blending (you might want to implement proper alpha blending)
    if (alpha < 128) {
        // Fade to darker version
        uint32_t r = ((color >> 16) & 0xFF) * alpha / 255;
        uint32_t g = ((color >> 8) & 0xFF) * alpha / 255;
        uint32_t b = (color & 0xFF) * alpha / 255;
        return (r << 16) | (g << 8) | b;
    }
    return color;
}

// Draw circle outline with thickness
void draw_circle_outline(int cx, int cy, int radius, uint32_t color, int thickness) {
    for (int t = 0; t < thickness; t++) {
        // Draw multiple circles for thickness
        draw_circle_border(cx, cy, radius - t, color);
    }
}

// Draw circle border (single pixel width)
void draw_circle_border(int cx, int cy, int radius, uint32_t color) {
    // Bresenham's circle algorithm for outline
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (y >= x) {
        // Draw 8 points for each calculated point
        draw_pixel(cx + x, cy + y, color);
        draw_pixel(cx - x, cy + y, color);
        draw_pixel(cx + x, cy - y, color);
        draw_pixel(cx - x, cy - y, color);
        draw_pixel(cx + y, cy + x, color);
        draw_pixel(cx - y, cy + x, color);
        draw_pixel(cx + y, cy - x, color);
        draw_pixel(cx - y, cy - x, color);
        
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

// Draw sparkle effect for success feedback
void draw_sparkle_effect(int cx, int cy, int radius) {
    // Draw small sparkles around the circle
    for (int i = 0; i < 8; i++) {
        int angle = i * 45; // 8 sparkles, 45 degrees apart
        int sparkle_x = cx + (radius * cos_lookup(angle)) / 256;
        int sparkle_y = cy + (radius * sin_lookup(angle)) / 256;
        draw_pixel(sparkle_x, sparkle_y, COLOR_SUCCESS);
        draw_pixel(sparkle_x + 1, sparkle_y, COLOR_SUCCESS);
        draw_pixel(sparkle_x, sparkle_y + 1, COLOR_SUCCESS);
    }
}

// Simple lookup tables for cos/sin (you might want to implement proper trig functions)
int cos_lookup(int angle) {
    static const int cos_table[] = {256, 181, 0, -181, -256, -181, 0, 181};
    return cos_table[(angle / 45) % 8];
}

int sin_lookup(int angle) {
    static const int sin_table[] = {0, 181, 256, 181, 0, -181, -256, -181};
    return sin_table[(angle / 45) % 8];
}

// Audio feedback functions (implement based on your audio system)
void play_button_sound() {
    // Short, sharp click
    play_tone(800, 50); // 800Hz for 50ms
}

void play_long_press_sound() {
    // Lower pitched, longer sound
    play_tone(400, 200);
}

void play_error_sound() {
    // Double beep for error
    play_tone(300, 100);
    delay_ms(50);
    play_tone(300, 100);
}

void play_success_sound() {
    // Pleasant ascending tone
    play_tone(600, 100);
    delay_ms(50);
    play_tone(800, 100);
}

// Haptic feedback functions
void trigger_haptic_feedback(TouchFeedbackType type) {
    switch (type) {
        case TOUCH_NORMAL:
        case TOUCH_BUTTON:
            haptic_light_tap();
            break;
        case TOUCH_LONG_PRESS:
            haptic_medium_buzz();
            break;
        case TOUCH_ERROR:
            haptic_double_tap();
            break;
        case TOUCH_SUCCESS:
            haptic_success_pattern();
            break;
        case TOUCH_DRAG:
            // No haptic for drag to avoid continuous vibration
            break;
    }
}

// Touch feedback settings
void set_feedback_enabled(bool enabled) {
    feedback_enabled = enabled;
}

void set_haptic_enabled(bool enabled) {
    haptic_enabled = enabled;
}

void set_sound_enabled(bool enabled) {
    sound_enabled = enabled;
}

// Initialize touch feedback system
void init_touch_feedback() {
    // Clear all effects
    memset(touch_effects, 0, sizeof(touch_effects));
    
    // Set default settings
    feedback_enabled = true;
    haptic_enabled = true;
    sound_enabled = true;
}

// Convenience functions for common touch types
void show_button_feedback(int x, int y) {
    show_enhanced_touch_feedback(x, y, TOUCH_BUTTON);
}

void show_error_feedback(int x, int y) {
    show_enhanced_touch_feedback(x, y, TOUCH_ERROR);
}

void show_success_feedback(int x, int y) {
    show_enhanced_touch_feedback(x, y, TOUCH_SUCCESS);
}

void show_drag_feedback(int x, int y) {
    show_enhanced_touch_feedback(x, y, TOUCH_DRAG);
}
