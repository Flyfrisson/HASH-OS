#ifndef TOUCH_FEEDBACK_H
#define TOUCH_FEEDBACK_H

#include <stdint.h>
#include <stdbool.h>

// Touch feedback types
typedef enum {
    TOUCH_NORMAL,
    TOUCH_BUTTON,
    TOUCH_LONG_PRESS,
    TOUCH_DRAG,
    TOUCH_ERROR,
    TOUCH_SUCCESS
} TouchFeedbackType;

// Touch effect structure (forward declaration)
typedef struct TouchEffect TouchEffect;

// Basic touch feedback (your original function)
void show_touch_feedback(int x, int y);

// Enhanced touch feedback functions
void show_enhanced_touch_feedback(int x, int y, TouchFeedbackType type);
void update_touch_effects();

// Convenience functions for common feedback types
void show_button_feedback(int x, int y);
void show_error_feedback(int x, int y);
void show_success_feedback(int x, int y);
void show_drag_feedback(int x, int y);

// Effect rendering functions
void render_touch_effect(TouchEffect* effect);
void draw_circle_outline(int cx, int cy, int radius, uint32_t color, int thickness);
void draw_circle_border(int cx, int cy, int radius, uint32_t color);
void draw_sparkle_effect(int cx, int cy, int radius);

// Audio feedback functions
void play_button_sound();
void play_long_press_sound();
void play_error_sound();
void play_success_sound();

// Haptic feedback functions
void trigger_haptic_feedback(TouchFeedbackType type);
void haptic_light_tap();
void haptic_medium_buzz();
void haptic_double_tap();
void haptic_success_pattern();

// Utility functions
int find_available_effect_slot();
uint32_t apply_alpha(uint32_t color, int alpha);
int cos_lookup(int angle);
int sin_lookup(int angle);

// Settings functions
void set_feedback_enabled(bool enabled);
void set_haptic_enabled(bool enabled);
void set_sound_enabled(bool enabled);

// System functions
void init_touch_feedback();

// Audio system integration (implement these in your audio driver)
void play_tone(int frequency, int duration_ms);
void delay_ms(int ms);
void draw_pixel(int x, int y, uint32_t color);

#endif // TOUCH_FEEDBACK_H
