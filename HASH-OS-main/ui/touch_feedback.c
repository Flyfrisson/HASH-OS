#include "touch_feedback.h"
#include "../drivers/display4k.h"
#include "../drivers/audio_output.h"

void show_touch_feedback(int x, int y) {
    draw_circle(x, y, 30, 0x00FFFF);
    play_touch_sound();
}
