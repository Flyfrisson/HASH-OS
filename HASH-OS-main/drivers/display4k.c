#include <stdint.h>
#include <string.h>
#include "display4k.h"
#include "fonts.h"

// Consistent framebuffer declaration
uint32_t *framebuffer = NULL;

// Initialize display system
void init_display4k() {
    // In real implementation, this would initialize display hardware
    // For now, assume framebuffer is allocated elsewhere
    if (framebuffer) {
        clear_screen(0x000000); // Clear to black
    }
}

// Set framebuffer pointer
void set_framebuffer(uint32_t *fb) {
    framebuffer = fb;
}

// Check if pixel coordinates are valid
int is_pixel_valid(int x, int y) {
    return (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT);
}

// Draw single pixel with bounds checking
void draw_pixel(int x, int y, uint32_t color) {
    if (framebuffer && is_pixel_valid(x, y)) {
        framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

// Alias for draw_pixel for API consistency
void plot_pixel(int x, int y, uint32_t color) {
    draw_pixel(x, y, color);
}

// Draw character using bitmap font
void draw_char(int x, int y, char ch, uint32_t color) {
    if (!framebuffer || ch < 32 || ch > 127) {
        return; // Invalid character or no framebuffer
    }
    
    unsigned char *glyph = font8x10[ch - 32];
    
    for (int row = 0; row < 10; row++) {
        unsigned char row_data = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) {
                draw_pixel(x + col, y + row, color);
            }
        }
    }
}

// Draw string with newline support
void draw_string(int x, int y, const char *str, uint32_t color) {
    if (!str) return;
    
    int cursor_x = x;
    int cursor_y = y;
    
    while (*str) {
        if (*str == '\n') {
            cursor_y += 12;  // Move to next line
            cursor_x = x;    // Reset to start of line
        } else if (*str == '\r') {
            cursor_x = x;    // Carriage return
        } else if (*str == '\t') {
            cursor_x = ((cursor_x - x) / 32 + 1) * 32 + x; // Tab to next 32-pixel boundary
        } else if (*str >= 32 && *str <= 127) {
            draw_char(cursor_x, cursor_y, *str, color);
            cursor_x += 9;   // Move to next character position
        }
        str++;
    }
}

// Clear entire screen to specified color
void clear_screen(uint32_t color) {
    if (!framebuffer) return;
    
    if (color == 0) {
        // Optimized clear for black
        memset(framebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    } else {
        // Fill with specified color
        uint32_t *fb = framebuffer;
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            *fb++ = color;
        }
    }
}

// Draw rectangle outline
void draw_rect(int x, int y, int width, int height, uint32_t color) {
    if (width <= 0 || height <= 0) return;
    
    // Top and bottom edges
    for (int i = 0; i < width; i++) {
        draw_pixel(x + i, y, color);                    // Top edge
        draw_pixel(x + i, y + height - 1, color);       // Bottom edge
    }
    
    // Left and right edges
    for (int i = 0; i < height; i++) {
        draw_pixel(x, y + i, color);                    // Left edge
        draw_pixel(x + width - 1, y + i, color);        // Right edge
    }
}

// Draw filled rectangle
void draw_filled_rect(int x, int y, int width, int height, uint32_t color) {
    if (width <= 0 || height <= 0) return;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            draw_pixel(x + j, y + i, color);
        }
    }
}

// Draw line using Bresenham's algorithm
void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1, y = y1;
    
    while (1) {
        draw_pixel(x, y, color);
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}