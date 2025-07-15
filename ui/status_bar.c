#include "status_bar.h"
#include "../drivers/display4k.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Status bar configuration
#define STATUS_BAR_HEIGHT   100
#define STATUS_BAR_WIDTH    3840
#define STATUS_BAR_COLOR    0x333333
#define TEXT_COLOR          0xFFFFFF
#define ICON_COLOR          0x00AAFF
#define WARNING_COLOR       0xFFAA00
#define CRITICAL_COLOR      0xFF0000

// System status structure
typedef struct {
    int battery_level;      // 0-100%
    bool is_charging;
    bool wifi_connected;
    int wifi_strength;      // 0-4 bars
    bool bluetooth_enabled;
    int hour;
    int minute;
    char date_string[32];
    bool airplane_mode;
    int volume_level;       // 0-100%
    bool silent_mode;
    int cpu_usage;          // 0-100%
    int memory_usage;       // 0-100%
} SystemStatus;

// Global system status
static SystemStatus system_status = {
    .battery_level = 80,
    .is_charging = false,
    .wifi_connected = true,
    .wifi_strength = 3,
    .bluetooth_enabled = true,
    .hour = 10,
    .minute = 30,
    .date_string = "Jan 1, 2025",
    .airplane_mode = false,
    .volume_level = 70,
    .silent_mode = false,
    .cpu_usage = 25,
    .memory_usage = 60
};

// Your original status bar implementation
void render_status_bar() {
    draw_rect(0, 0, 3840, 100, 0x333333);
    draw_text(100, 50, "HASH OS", 0xFFFFFF);
    draw_text(3000, 50, "Battery: 80%", 0xFFFFFF);
    draw_text(3400, 50, "Time: 10:30", 0xFFFFFF);
    // Future: Connect real battery and time modules
}

// Enhanced status bar with dynamic content
void render_enhanced_status_bar() {
    // Clear status bar area
    draw_rect(0, 0, STATUS_BAR_WIDTH, STATUS_BAR_HEIGHT, STATUS_BAR_COLOR);
    
    // Left section: OS name and system info
    draw_text(20, 35, "HASH OS", TEXT_COLOR);
    
    if (system_status.cpu_usage > 80) {
        draw_text(20, 65, "CPU High", WARNING_COLOR);
    }
    
    // Center section: Date
    draw_text(STATUS_BAR_WIDTH/2 - 80, 50, system_status.date_string, TEXT_COLOR);
    
    // Right section: System status icons and info
    int right_x = STATUS_BAR_WIDTH - 50;
    
    // Time
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", 
             system_status.hour, system_status.minute);
    right_x -= 120;
    draw_text(right_x, 35, time_str, TEXT_COLOR);
    
    // Battery
    right_x -= 150;
    draw_battery_icon(right_x, 25);
    
    // WiFi
    if (system_status.wifi_connected) {
        right_x -= 80;
        draw_wifi_icon(right_x, 25);
    }
    
    // Bluetooth
    if (system_status.bluetooth_enabled) {
        right_x -= 60;
        draw_bluetooth_icon(right_x, 30);
    }
    
    // Volume/Silent
    right_x -= 80;
    draw_volume_icon(right_x, 30);
}

// Draw battery icon with level indicator
void draw_battery_icon(int x, int y) {
    uint32_t battery_color = TEXT_COLOR;
    
    // Choose color based on battery level
    if (system_status.battery_level < 20) {
        battery_color = CRITICAL_COLOR;
    } else if (system_status.battery_level < 40) {
        battery_color = WARNING_COLOR;
    }
    
    // Battery outline
    draw_rect(x, y, 40, 20, battery_color);
    draw_rect(x + 40, y + 6, 4, 8, battery_color); // Battery tip
    
    // Battery fill based on level
    int fill_width = (36 * system_status.battery_level) / 100;
    if (fill_width > 0) {
        draw_rect(x + 2, y + 2, fill_width, 16, battery_color);
    }
    
    // Charging indicator
    if (system_status.is_charging) {
        draw_text(x - 20, y + 25, "CHG", ICON_COLOR);
    }
    
    // Battery percentage
    char battery_text[8];
    snprintf(battery_text, sizeof(battery_text), "%d%%", system_status.battery_level);
    draw_text(x - 10, y + 25, battery_text, TEXT_COLOR);
}

// Draw WiFi icon with signal strength
void draw_wifi_icon(int x, int y) {
    uint32_t wifi_color = system_status.wifi_connected ? ICON_COLOR : 0x666666;
    
    // Draw WiFi bars based on signal strength
    for (int i = 0; i < 4; i++) {
        if (i < system_status.wifi_strength) {
            int bar_height = 8 + (i * 4);
            draw_rect(x + (i * 8), y + (20 - bar_height), 6, bar_height, wifi_color);
        } else {
            int bar_height = 8 + (i * 4);
            draw_rect(x + (i * 8), y + (20 - bar_height), 6, bar_height, 0x444444);
        }
    }
}

// Draw Bluetooth icon
void draw_bluetooth_icon(int x, int y) {
    uint32_t bt_color = system_status.bluetooth_enabled ? ICON_COLOR : 0x666666;
    
    // Simple Bluetooth "B" representation
    draw_text(x, y, "BT", bt_color);
}

// Draw volume icon
void draw_volume_icon(int x, int y) {
    if (system_status.silent_mode) {
        draw_text(x, y, "MUTE", WARNING_COLOR);
    } else {
        // Volume bars
        int bars = (system_status.volume_level * 3) / 100;
        for (int i = 0; i < 3; i++) {
            uint32_t bar_color = (i < bars) ? ICON_COLOR : 0x444444;
            draw_rect(x + (i * 6), y + (10 - i * 2), 4, 8 + (i * 4), bar_color);
        }
    }
}

// Notification area (expandable status bar)
void render_notification_area() {
    // Extended status bar for notifications
    draw_rect(0, 0, STATUS_BAR_WIDTH, 300, 0x222222);
    
    // Quick settings toggles
    int toggle_y = 120;
    draw_quick_toggle(200, toggle_y, "WiFi", system_status.wifi_connected);
    draw_quick_toggle(400, toggle_y, "Bluetooth", system_status.bluetooth_enabled);
    draw_quick_toggle(600, toggle_y, "Airplane", system_status.airplane_mode);
    
    // System information
    draw_text(200, 220, "System Information:", TEXT_COLOR);
    
    char sys_info[64];
    snprintf(sys_info, sizeof(sys_info), "CPU: %d%% | Memory: %d%%", 
             system_status.cpu_usage, system_status.memory_usage);
    draw_text(200, 250, sys_info, TEXT_COLOR);
}

// Draw quick toggle button
void draw_quick_toggle(int x, int y, const char* label, bool enabled) {
    uint32_t bg_color = enabled ? ICON_COLOR : 0x444444;
    uint32_t text_color = enabled ? 0x000000 : TEXT_COLOR;
    
    // Toggle background
    draw_rect(x, y, 120, 60, bg_color);
    
    // Toggle label
    draw_text(x + 10, y + 25, label, text_color);
}

// Update system status functions
void update_battery_status(int level, bool charging) {
    system_status.battery_level = level;
    system_status.is_charging = charging;
}

void update_time(int hour, int minute) {
    system_status.hour = hour;
    system_status.minute = minute;
}

void update_wifi_status(bool connected, int strength) {
    system_status.wifi_connected = connected;
    system_status.wifi_strength = strength;
}

void update_volume(int level, bool silent) {
    system_status.volume_level = level;
    system_status.silent_mode = silent;
}

void set_date_string(const char* date) {
    snprintf(system_status.date_string, sizeof(system_status.date_string), "%s", date);
}

// Get current system status
SystemStatus* get_system_status() {
    return &system_status;
}

// Initialize status bar
void init_status_bar() {
    // Initialize with default values or read from system
    system_status.battery_level = 80;
    system_status.is_charging = false;
    system_status.wifi_connected = true;
    system_status.wifi_strength = 3;
    system_status.bluetooth_enabled = true;
    system_status.hour = 10;
    system_status.minute = 30;
    snprintf(system_status.date_string, sizeof(system_status.date_string), "Jan 1, 2025");
}
