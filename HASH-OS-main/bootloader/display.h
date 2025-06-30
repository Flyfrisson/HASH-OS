// display.h - Your exact header file#include "display.h"
#include "fonts.h"

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// VGA color constants
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15
} vga_color_t;

// Progress bar structure
typedef struct {
    int x, y;
    int width;
    unsigned int limit;
    unsigned int current;
    char title[32];
    unsigned char active;
} ProgressBar;

#define MAX_PROGRESS_BARS 4

// Global variables with proper initialization
static volatile unsigned short* vga_buffer = (volatile unsigned short*) VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color = 0x07; // Light grey on black
static ProgressBar progress_bars[MAX_PROGRESS_BARS] = {0};

// Utility functions
static int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

static unsigned char make_color(unsigned char fg, unsigned char bg) {
    return fg | (bg << 4);
}

static unsigned short make_vgaentry(unsigned char c, unsigned char color) {
    return (unsigned short) c | ((unsigned short) color << 8);
}

void InitDisplay() {
    // Initialize progress bars
    for (int i = 0; i < MAX_PROGRESS_BARS; i++) {
        progress_bars[i].active = 0;
    }
    
    // Set default color
    current_color = make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Clear screen and reset cursor
    ClearScreen();
    cursor_x = 0;
    cursor_y = 0;
}

void ClearScreen() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            vga_buffer[index] = make_vgaentry(' ', current_color);
        }
    }
}

void SetCursorPosition(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
    }
}

void SetTextColor(unsigned char foreground, unsigned char background) {
    current_color = make_color(foreground, background);
}

static void ScrollScreen() {
    // Move all lines up by one
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the bottom line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = make_vgaentry(' ', current_color);
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

void PutChar(unsigned char aChar) {
    switch (aChar) {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            break;
        case '\r':
            cursor_x = 0;
            break;
        case '\t':
            cursor_x = (cursor_x + 8) & ~7; // Tab to next 8-character boundary
            break;
        case '\b':
            if (cursor_x > 0) {
                cursor_x--;
                const int index = cursor_y * VGA_WIDTH + cursor_x;
                vga_buffer[index] = make_vgaentry(' ', current_color);
            }
            break;
        default:
            // Handle line wrap
            if (cursor_x >= VGA_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
            
            const int index = cursor_y * VGA_WIDTH + cursor_x;
            vga_buffer[index] = make_vgaentry(aChar, current_color);
            cursor_x++;
            break;
    }
    
    // Handle screen scrolling
    if (cursor_y >= VGA_HEIGHT) {
        ScrollScreen();
    }
}

void PutString(const char* str) {
    if (!str) return;
    
    for (int i = 0; str[i] != '\0'; i++) {
        PutChar(str[i]);
    }
}


void UpdateProgressBar(int id, unsigned int progress) {
    if (id < 0 || id >= MAX_PROGRESS_BARS) return;
    if (!progress_bars[id].active) return;
    
    progress_bars[id].current = progress;
    if (progress_bars[id].current > progress_bars[id].limit) {
        progress_bars[id].current = progress_bars[id].limit;
    }
    
    DrawProgressBar(id);
}

// Test/Demo function (optional - can be removed)
void TestDisplay() {
    InitDisplay();
    
    PutString("HASH OS Display Driver Test\n");
    PutString("==========================\n\n");
    
    PutString("Testing character output: ");
    for (unsigned char c = 'A'; c <= 'Z'; c++) {
        PutChar(c);
    }
    PutString("\n\n");
    
    PutString("Testing progress bar:\n");
    InitProgressBar(0, 100, "Test Progress");
    
    for (unsigned int i = 0; i <= 100; i += 5) {
        UpdateProgressBar(0, i);
        // Small delay simulation
        for (volatile int delay = 0; delay < 50000; delay++);
    }
    
    PutString("\nTest complete!\n");
}