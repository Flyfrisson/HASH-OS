#include "splash.h"
#include "../drivers/display4k.h"

void render_splash_screen() {
    clear_screen(0x000000);
    draw_text(1200, 600, "HASH OS", 0xFFFFFF);
    draw_text(1200, 700, "Smartphone Edition", 0xFFFFFF);
    // Future: Add animated logo or boot sound
}
