
#include "display4k.h"
#include "touch_input.h"
#include "virtual_keyboard.h"

void init_drivers() {
    init_display4k();
    init_touch_input();
    init_virtual_keyboard();
}
