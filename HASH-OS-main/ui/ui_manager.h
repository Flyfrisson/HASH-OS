#ifndef HASHOS_UI_MANAGER_H
#define HASHOS_UI_MANAGER_H

void init_ui();
void render_home_screen();
void handle_touch_event(int x, int y);
void handle_keypress_event(char key);
void ui_main_loop();

#endif
