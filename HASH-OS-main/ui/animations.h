// animations.h
#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <stdint.h>

// Animation types
typedef enum {
    ANIM_FADE_IN,
    ANIM_FADE_OUT,
    ANIM_SLIDE_LEFT,
    ANIM_SLIDE_RIGHT,
    ANIM_SLIDE_UP,
    ANIM_SLIDE_DOWN,
    ANIM_SCALE,
    ANIM_BOUNCE,
    ANIM_PULSE
} animation_type_t;

// Animation state
typedef struct {
    int start_time;
    int duration;
    int current_frame;
    int total_frames;
    animation_type_t type;
    int x, y, width, height;
    uint32_t color;
    float progress;
    int active;
} animation_t;

// Animation functions
void animate_icon_press(int x, int y, int width, int height);
void animate_window_open(int x, int y, int width, int height);
void animate_window_close(int x, int y, int width, int height);
void animate_fade_transition(int x, int y, int width, int height, uint32_t from_color, uint32_t to_color);
void animate_slide_transition(int from_x, int from_y, int to_x, int to_y, int width, int height);
void animate_bounce_icon(int x, int y, int width, int height);
void animate_pulse_notification(int x, int y, int radius);

// Animation system
void init_animation_system(void);
void update_animations(void);
animation_t* create_animation(animation_type_t type, int x, int y, int width, int height, int duration);
void destroy_animation(animation_t* anim);

#endif