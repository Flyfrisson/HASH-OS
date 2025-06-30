#include "status_bar.h"
#include "../drivers/display4k.h"

void render_status_bar() {
    draw_rect(0, 0, 3840, 100, 0x333333);
    draw_text(100, 50, "HASH OS", 0xFFFFFF);
    draw_text(3000, 50, "Battery: 80%", 0xFFFFFF);
    draw_text(3400, 50, "Time: 10:30", 0xFFFFFF);
    // Future: Connect real battery and time modules
}
