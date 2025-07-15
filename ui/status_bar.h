#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <stdint.h>
#include <stdbool.h>

// System status structure (forward declaration)
typedef struct SystemStatus SystemStatus;

// Basic status bar functions
void render_status_bar();

// Enhanced status bar functions
void render_enhanced_status_bar();
void render_notification_area();

// Status bar components
void draw_battery_icon(int x, int y);
void draw_wifi_icon(int x, int y);
void draw_bluetooth_icon(int x, int y);
void draw_volume_icon(int x, int y);
void draw_quick_toggle(int x, int y, const char* label, bool enabled);

// System status update functions
void update_battery_status(int level, bool charging);
void update_time(int hour, int minute);
void update_wifi_status(bool connected, int strength);
void update_volume(int level, bool silent);
void set_date_string(const char* date);

// Status bar management
void init_status_bar();
SystemStatus* get_system_status();

#endif // STATUS_BAR_H
