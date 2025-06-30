#include "../drivers/display4k.h"

void settings_ui_loop() {
    clear_screen(0x000000); // Black background
    draw_string(100, 100, "Settings Running", 0xFFFF00);

    // Simulated settings options
    draw_string(150, 200, "🔊 Sound Settings", 0xFFFFFF);
    draw_string(150, 250, "🌐 Network Settings", 0xFFFFFF);
    draw_string(150, 300, "🔋 Display Settings", 0xFFFFFF);

    // Simulated delay
    for (volatile int i = 0; i < 50000000; i++);
}
