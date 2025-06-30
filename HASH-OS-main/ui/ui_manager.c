#include "ui_manager.h"
#include "../drivers/display4k.h"
#include "../drivers/touch_input.h"
#include "../drivers/virtual_keyboard.h"
#include "launcher.h"

// Initialize UI system
void init_ui() {
    render_home_screen();
}

// Render the home screen layout
void render_home_screen() {
    clear_screen(0x222222); // Dark background
    draw_launcher_icons();  // Display app icons
}

// Handle touch inputs at UI level
void handle_touch_event(int x, int y) {
    handle_launcher_touch(x, y);
}

// Handle keypress events at UI level (optional, can expand later)
void handle_keypress_event(char key) {
    // Example: Display pressed key at top corner
    char message[2] = { key, '\0' };
    draw_string(20, 20, message, 0x00FF00); // Green text
}

// Main UI Event Loop
void ui_main_loop() {
    int touch_x = -1, touch_y = -1;

    while (1) {
        if (get_touch_input(&touch_x, &touch_y)) {
            handle_touch_event(touch_x, touch_y);
        }

        char key = get_virtual_key();
        handle_keypress_event(key); // You can process the key here
    }

} // ✅ This closing brace was missing
