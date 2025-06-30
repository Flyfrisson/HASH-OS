#ifndef HASHOS_CONFIG_PARSER_H
#define HASHOS_CONFIG_PARSER_H

// Configuration structure
typedef struct {
    char boot_theme[32];
    int screen_brightness;
    char default_audio_profile[32];
} SystemConfig;

void parse_config();
SystemConfig get_system_config();

#endif
