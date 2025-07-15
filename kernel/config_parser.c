#include "config_parser.h"
#include "../drivers/display4k.h"

SystemConfig system_config;

void parse_config() {
    // For now, hardcoded config (can be replaced with file-based later)
    // In future: Load from a config file stored in the filesystem.

    // Simulated config values
    system_config.screen_brightness = 85;
    system_config.boot_theme[0] = 'D'; system_config.boot_theme[1] = 'a';
    system_config.boot_theme[2] = 'r'; system_config.boot_theme[3] = 'k';
    system_config.boot_theme[4] = '\0';

    system_config.default_audio_profile[0] = 'S'; system_config.default_audio_profile[1] = 't';
    system_config.default_audio_profile[2] = 'a'; system_config.default_audio_profile[3] = 'n';
    system_config.default_audio_profile[4] = 'd'; system_config.default_audio_profile[5] = 'a';
    system_config.default_audio_profile[6] = 'r'; system_config.default_audio_profile[7] = 'd';
    system_config.default_audio_profile[8] = '\0';

    // Visual confirmation via colored rectangles
    // Example: If brightness > 80, show green block
    if (system_config.screen_brightness > 80) {
        draw_rect(50, 50, 100, 100, 0x00FF00); // Green block for high brightness
    } else {
        draw_rect(50, 50, 100, 100, 0xFF0000); // Red block for low brightness
    }

    // Future: Apply themes, audio profiles
}

SystemConfig get_system_config() {
    return system_config;
}
