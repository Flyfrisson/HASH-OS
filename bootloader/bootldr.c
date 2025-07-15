// bootldr.h - Bootloader header file
#ifndef BOOTLDR_H
#define BOOTLDR_H

#include <stdint.h>
#include <stddef.h>

// Basic type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// VGA color constants
enum vga_color {
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
    VGA_COLOR_WHITE = 15,
};

// Progress bar structure
typedef struct {
    int x, y;
    int width;
    int min_val, max_val;
    int current_val;
    char label[32];
    uint8_t active;
} ProgressBar;

#define MAX_PROGRESS_BARS 4

// Function prototypes
void BootMain(void);
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
uint16_t vga_entry(unsigned char uc, uint8_t color);

#endif // BOOTLDR_H

// display.h - Display functions header
#ifndef DISPLAY_H
#define DISPLAY_H

#include "bootldr.h"

// Display function prototypes
void InitDisplay(void);
void ClearScreen(void);
void SetCursor(int x, int y);
void PutChar(char c);
void PutString(const char* str);
void SetColor(enum vga_color fg, enum vga_color bg);

// Progress bar functions
void InitProgressBar(int id, int max_val, const char* label);
void UpdateProgressBar(int id, int value);
void DrawProgressBar(int id);

#endif // DISPLAY_H

// display.c - Display implementation
#include "display.h"

// Global variables
static uint16_t* vga_buffer = (uint16_t*) VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 7; // Light grey on black
static ProgressBar progress_bars[MAX_PROGRESS_BARS];

// String length function
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// String copy function
static void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void InitDisplay(void) {
    // Initialize progress bars
    for (int i = 0; i < MAX_PROGRESS_BARS; i++) {
        progress_bars[i].active = 0;
    }
    
    // Set default color
    current_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Clear screen
    ClearScreen();
    
    // Set cursor to top-left
    cursor_x = 0;
    cursor_y = 0;
}

void ClearScreen(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', current_color);
        }
    }
}

void SetCursor(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
    }
}

void SetColor(enum vga_color fg, enum vga_color bg) {
    current_color = vga_entry_color(fg, bg);
}

void PutChar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7; // Tab to next 8-character boundary
    } else {
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
        
        const size_t index = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[index] = vga_entry(c, current_color);
        cursor_x++;
    }
    
    // Scroll screen if we've reached the bottom
    if (cursor_y >= VGA_HEIGHT) {
        // Scroll up
        for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
            }
        }
        
        // Clear bottom line
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
        }
        
        cursor_y = VGA_HEIGHT - 1;
    }
}

void PutString(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        PutChar(str[i]);
    }
}

void InitProgressBar(int id, int max_val, const char* label) {
    if (id < 0 || id >= MAX_PROGRESS_BARS) return;
    
    ProgressBar* bar = &progress_bars[id];
    bar->x = 5;
    bar->y = cursor_y + 1;
    bar->width = 50;
    bar->min_val = 0;
    bar->max_val = max_val;
    bar->current_val = 0;
    bar->active = 1;
    strcpy(bar->label, label);
    
    // Move cursor past the progress bar area
    cursor_y += 3;
}

void UpdateProgressBar(int id, int value) {
    if (id < 0 || id >= MAX_PROGRESS_BARS) return;
    if (!progress_bars[id].active) return;
    
    progress_bars[id].current_val = value;
    if (progress_bars[id].current_val > progress_bars[id].max_val) {
        progress_bars[id].current_val = progress_bars[id].max_val;
    }
    
    DrawProgressBar(id);
}

void DrawProgressBar(int id) {
    if (id < 0 || id >= MAX_PROGRESS_BARS) return;
    if (!progress_bars[id].active) return;
    
    ProgressBar* bar = &progress_bars[id];
    
    // Save current cursor position
    int saved_x = cursor_x;
    int saved_y = cursor_y;
    uint8_t saved_color = current_color;
    
    // Draw label
    SetCursor(bar->x, bar->y);
    SetColor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    PutString(bar->label);
    PutString(": ");
    
    // Calculate progress percentage
    int progress = (bar->current_val * bar->width) / bar->max_val;
    
    // Draw progress bar frame
    SetCursor(bar->x, bar->y + 1);
    PutChar('[');
    
    // Draw progress bar fill
    for (int i = 0; i < bar->width; i++) {
        if (i < progress) {
            SetColor(VGA_COLOR_GREEN, VGA_COLOR_GREEN);
            PutChar(' ');
        } else {
            SetColor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
            PutChar(' ');
        }
    }
    
    SetColor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    PutChar(']');
    
    // Draw percentage
    SetCursor(bar->x + bar->width + 3, bar->y + 1);
    int percentage = (bar->current_val * 100) / bar->max_val;
    
    // Simple integer to string conversion for percentage
    if (percentage == 100) {
        PutString("100%");
    } else if (percentage >= 10) {
        PutChar('0' + (percentage / 10));
        PutChar('0' + (percentage % 10));
        PutChar('%');
    } else {
        PutChar(' ');
        PutChar('0' + percentage);
        PutChar('%');
    }
    
    // Restore cursor position and color
    SetCursor(saved_x, saved_y);
    current_color = saved_color;
}

// bootloader.c - Main bootloader implementation
#include "bootldr.h"
#include "display.h"

void BootMain(void) {
    // Initialize display system
    InitDisplay();
    
    // Display welcome message
    SetColor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    PutString("===============================================\n");
    PutString("       Welcome to HASH OS Bootloader v1.0     \n");
    PutString("===============================================\n\n");
    
    SetColor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    PutString("Initializing system components...\n\n");
    
    // Initialize and run boot progress
    InitProgressBar(0, 100, "Boot Progress");
    
    // Simulate various boot stages
    const char* boot_stages[] = {
        "Initializing hardware...",
        "Loading system drivers...",
        "Mounting file systems...",
        "Starting system services...",
        "Finalizing boot process..."
    };
    
    int stage_progress[] = {20, 40, 60, 80, 100};
    int current_stage = 0;
    
    for (int i = 0; i <= 100; i++) {
        UpdateProgressBar(0, i);
        
        // Display current boot stage
        if (current_stage < 5 && i >= stage_progress[current_stage]) {
            SetColor(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
            PutString(boot_stages[current_stage]);
            PutString("\n");
            current_stage++;
        }
        
        // Simulate boot delay
        for (volatile int delay = 0; delay < 100000; delay++);
    }
    
    // Boot complete message
    SetColor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    PutString("\nBoot Complete!\n");
    
    SetColor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    PutString("HASH OS is ready to use.\n\n");
    
    SetColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    PutString("System halted. Press reset to restart.\n");
    
    // Halt system - infinite loop
    while (1) {
        // In a real bootloader, this might jump to the kernel
        // or wait for interrupts
        __asm__("hlt");
    }
}

// For GCC compilation, you might need a simple entry point
void _start(void) {
    BootMain();
}

// If using a linker script, you might need these symbols
extern void __stack_top;
void* __stack_chk_guard = (void*)0xdeadbeef;

void __stack_chk_fail(void) {
    PutString("Stack smashing detected!\n");
    while(1);
}