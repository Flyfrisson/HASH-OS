#include <stdint.h>
#include "display4k.h"
#include "virtual_keyboard.h"
#include "touch_input.h"

// Virtual key layout (simple grid example)
struct VirtualKey {
    char label;
    int x, y, width, height;
};

struct VirtualKey keys[] = {
    { 'A', 100, 1800, 100, 100 },
    { 'B', 250, 1800, 100, 100 },
    { 'C', 400, 1800, 100, 100 },
    { 'D', 550, 1800, 100, 100 },
    { 'E', 700, 1800, 100, 100 },
    { 'F', 850, 1800, 100, 100 },
    { 'G', 1000, 1800, 100, 100 },
    { 'H', 1150, 1800, 100, 100 },
    { 'I', 1300, 1800, 100, 100 },
    { 'J', 1450, 1800, 100, 100 },
    // Second row
    { 'K', 100, 1920, 100, 100 },
    { 'L', 250, 1920, 100, 100 },
    { 'M', 400, 1920, 100, 100 },
    { 'N', 550, 1920, 100, 100 },
    { 'O', 700, 1920, 100, 100 },
    { 'P', 850, 1920, 100, 100 },
    { 'Q', 1000, 1920, 100, 100 },
    { 'R', 1150, 1920, 100, 100 },
    { 'S', 1300, 1920, 100, 100 },
    { 'T', 1450, 1920, 100, 100 },
    // Third row
    { 'U', 100, 2040, 100, 100 },
    { 'V', 250, 2040, 100, 100 },
    { 'W', 400, 2040, 100, 100 },
    { 'X', 550, 2040, 100, 100 },
    { 'Y', 700, 2040, 100, 100 },
    { 'Z', 850, 2040, 100, 100 },
    { ' ', 1000, 2040, 300, 100 }, // Space bar
};

#define NUM_KEYS (sizeof(keys) / sizeof(keys[0]))

// Static variables for keyboard state
static bool keyboard_initialized = false;
static char last_pressed_key = 0;

// ✅ MISSING FUNCTION IMPLEMENTATION
// Initialize the virtual keyboard system
void init_virtual_keyboard() {
    keyboard_initialized = true;
    last_pressed_key = 0;
}

// Draw virtual keyboard
void draw_virtual_keyboard() {
    if (!keyboard_initialized) {
        init_virtual_keyboard();
    }
    
    for (int i = 0; i < NUM_KEYS; i++) {
        draw_rect(keys[i].x, keys[i].y, keys[i].width, keys[i].height, 0x333333); // Key background
        // Optional: Draw key label using display_print
        // display_print_char(keys[i].x + 40, keys[i].y + 40, keys[i].label, 0xFFFFFF);
    }
}

// Detect which key is pressed based on touch coordinates
char detect_virtual_key(int touch_x, int touch_y) {
    for (int i = 0; i < NUM_KEYS; i++) {
        if (touch_x >= keys[i].x && touch_x <= (keys[i].x + keys[i].width) &&
            touch_y >= keys[i].y && touch_y <= (keys[i].y + keys[i].height)) {
            return keys[i].label;
        }
    }
    return 0; // No key pressed
}

// ✅ MISSING FUNCTION IMPLEMENTATION
// Handle a virtual key press event
void handle_virtual_key_press(char key) {
    if (key != 0) {
        last_pressed_key = key;
        // Add visual feedback here if needed
        // For example: highlight the pressed key
        
        // Process the key press (add to input buffer, etc.)
        // This would typically interface with your text input system
    }
}

// ✅ MISSING FUNCTION IMPLEMENTATION
// Simulate key press (for testing without real hardware)
void simulate_key_press(char key) {
    handle_virtual_key_press(key);
}

// ✅ MISSING FUNCTION IMPLEMENTATION
// Get the current virtual key being pressed
char get_virtual_key() {
    int touch_x, touch_y;
    
    // Check if there's currently a touch input
    if (get_touch_input(&touch_x, &touch_y)) {
        char key = detect_virtual_key(touch_x, touch_y);
        if (key != 0) {
            handle_virtual_key_press(key);
            return key;
        }
    }
    
    return 0; // No key currently pressed
}