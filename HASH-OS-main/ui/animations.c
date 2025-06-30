// animations.c
#include "animations.h"
#include "../drivers/display4k.h"
#include <math.h>

#define MAX_ANIMATIONS 32
static animation_t animations[MAX_ANIMATIONS];
static int animation_count = 0;
static int system_ticks = 0;

void init_animation_system(void) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        animations[i].active = 0;
    }
    animation_count = 0;
    system_ticks = 0;
}

animation_t* create_animation(animation_type_t type, int x, int y, int width, int height, int duration) {
    if (animation_count >= MAX_ANIMATIONS) return NULL;
    
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!animations[i].active) {
            animations[i].type = type;
            animations[i].x = x;
            animations[i].y = y;
            animations[i].width = width;
            animations[i].height = height;
            animations[i].duration = duration;
            animations[i].start_time = system_ticks;
            animations[i].current_frame = 0;
            animations[i].total_frames = duration;
            animations[i].progress = 0.0f;
            animations[i].active = 1;
            animation_count++;
            return &animations[i];
        }
    }
    return NULL;
}

void destroy_animation(animation_t* anim) {
    if (anim && anim->active) {
        anim->active = 0;
        animation_count--;
    }
}

float ease_in_out(float t) {
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

float ease_bounce(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

void update_animations(void) {
    system_ticks++;
    
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!animations[i].active) continue;
        
        animation_t* anim = &animations[i];
        int elapsed = system_ticks - anim->start_time;
        
        if (elapsed >= anim->duration) {
            destroy_animation(anim);
            continue;
        }
        
        anim->progress = (float)elapsed / (float)anim->duration;
        anim->current_frame = elapsed;
        
        // Apply animation based on type
        switch (anim->type) {
            case ANIM_FADE_IN: {
                float alpha = ease_in_out(anim->progress);
                uint32_t fade_color = (anim->color & 0x00FFFFFF) | ((uint32_t)(alpha * 255) << 24);
                draw_rect(anim->x, anim->y, anim->width, anim->height, fade_color);
                break;
            }
            case ANIM_SCALE: {
                float scale = ease_in_out(anim->progress);
                int scaled_width = (int)(anim->width * scale);
                int scaled_height = (int)(anim->height * scale);
                int offset_x = (anim->width - scaled_width) / 2;
                int offset_y = (anim->height - scaled_height) / 2;
                draw_rect(anim->x + offset_x, anim->y + offset_y, scaled_width, scaled_height, anim->color);
                break;
            }
            case ANIM_BOUNCE: {
                float bounce = ease_bounce(anim->progress);
                int offset_y = (int)(20 * (1.0f - bounce));
                draw_rect(anim->x, anim->y - offset_y, anim->width, anim->height, anim->color);
                break;
            }
            case ANIM_PULSE: {
                float pulse = (sinf(anim->progress * 3.14159f * 4) + 1.0f) / 2.0f;
                uint32_t pulse_color = (anim->color & 0x00FFFFFF) | ((uint32_t)(pulse * 255) << 24);
                draw_circle(anim->x, anim->y, (int)(anim->width * pulse), pulse_color);
                break;
            }
        }
    }
}

void animate_icon_press(int x, int y, int width, int height) {
    // Create shrink animation
    for (int i = 0; i < 5; i++) {
        draw_rounded_rect(x + i, y + i, width - 2 * i, height - 2 * i, 20, 0x999999);
    }
    
    // Final pressed state
    draw_rounded_rect(x + 2, y + 2, width - 4, height - 4, 18, 0x555555);
    
    // Create bounce back animation
    create_animation(ANIM_BOUNCE, x, y, width, height, 15);
}

void animate_window_open(int x, int y, int width, int height) {
    create_animation(ANIM_SCALE, x, y, width, height, 20);
    create_animation(ANIM_FADE_IN, x, y, width, height, 20);
}

void animate_window_close(int x, int y, int width, int height) {
    animation_t* scale_anim = create_animation(ANIM_SCALE, x, y, width, height, 15);
    if (scale_anim) {
        // Reverse scale animation for closing
        scale_anim->progress = 1.0f;
    }
}

void animate_bounce_icon(int x, int y, int width, int height) {
    create_animation(ANIM_BOUNCE, x, y, width, height, 30);
}

void animate_pulse_notification(int x, int y, int radius) {
    create_animation(ANIM_PULSE, x, y, radius, radius, 60);
}