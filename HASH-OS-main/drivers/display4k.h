#ifndef DISPLAY4K_H
#define DISPLAY4K_H

#include <stdint.h>

#define SCREEN_WIDTH 3840
#define SCREEN_HEIGHT 2160

// Consistent framebuffer declaration
extern uint32_t *framebuffer;

// Core display functions
void init_display4k();
void clear_screen(uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void plot_pixel(int x, int y, uint32_t color); // Alias for consistency

// Text rendering functions
void draw_char(int x, int y, char ch, uint32_t color);
void draw_string(int x, int y, const char *str, uint32_t color);

// Shape drawing functions
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_filled_rect(int x, int y, int width, int height, uint32_t color);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);

// Utility functions
int is_pixel_valid(int x, int y);
void set_framebuffer(uint32_t *fb);

#endif // DISPLAY4K_H