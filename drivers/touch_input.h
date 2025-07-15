#ifndef TOUCH_INPUT_H
#define TOUCH_INPUT_H

#include <stdbool.h>  // ✅ Add this for 'bool' type support

// Simple touch event structure
typedef struct {
    int x;
    int y;
    int is_pressed; // 1 = touch pressed, 0 = released
} TouchEvent;

// Initialize the touch input system
void init_touch_input();

// Simulate reading a touch event (in real hardware, this would connect to drivers)
TouchEvent get_touch_event();

// Manually set touch event (for testing without real hardware)
void set_simulated_touch(int x, int y, int is_pressed);

// ✅ ADD THIS FUNCTION PROTOTYPE
bool get_touch_input(int *x, int *y);

#endif // TOUCH_INPUT_H
