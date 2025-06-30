#ifndef VIRTUAL_KEYBOARD_H
#define VIRTUAL_KEYBOARD_H

// Initialize the virtual keyboard system
void init_virtual_keyboard();

// Handle a virtual key press event
void handle_virtual_key_press(char key);

// Simulate key press (for testing without real hardware)
void simulate_key_press(char key);

// ✅ ADD THIS FUNCTION PROTOTYPE
char get_virtual_key();

#endif // VIRTUAL_KEYBOARD_H
