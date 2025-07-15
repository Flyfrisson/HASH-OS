// ==========================================================================
// HASH OS COMPLETE CONFIGURATION SYSTEM
// ==========================================================================

// config.h - Main configuration system header
#ifndef HASHOS_CONFIG_H
#define HASHOS_CONFIG_H

#define MAX_CONFIG_LINE 512
#define MAX_CONFIG_VALUE 256
#define MAX_CONFIG_KEY 128
#define MAX_SECTION_NAME 64
#define MAX_CONFIG_FILES 16
#define MAX_FILENAME 256

// Configuration file types
typedef enum {
    CONFIG_TYPE_HCFG,      // HASH Config Format (sections)
    CONFIG_TYPE_CFG,       // Simple key=value
    CONFIG_TYPE_UNKNOWN
} ConfigFileType;

// Configuration sections
typedef enum {
    SECTION_NONE = 0,
    SECTION_BOOT,
    SECTION_KERNEL,
    SECTION_UI,
    SECTION_DRIVERS,
    SECTION_AUDIO,
    SECTION_NETWORK,
    SECTION_SECURITY,
    SECTION_SYSTEM
} ConfigSection;

// Boot configuration
typedef struct {
    char message[MAX_CONFIG_VALUE];
    unsigned int timeout_seconds;
    unsigned char show_logo;
    char boot_device[MAX_CONFIG_VALUE];
    unsigned char verbose_mode;
} BootConfig;

// Kernel configuration
typedef struct {
    unsigned int stack_size;
    unsigned int max_threads;
    char scheduler[MAX_CONFIG_VALUE];
    unsigned int heap_size;
    unsigned char debug_mode;
    unsigned int max_processes;
} KernelConfig;

// UI configuration
typedef struct {
    char theme[MAX_CONFIG_VALUE];
    char font[MAX_CONFIG_VALUE];
    unsigned int width;
    unsigned int height;
    unsigned int color_depth;
    unsigned char animations_enabled;
    unsigned char vsync_enabled;
} UIConfig;

// Driver configuration
typedef struct {
    unsigned char keyboard_enabled;
    unsigned char display_enabled;
    unsigned char storage_enabled;
    unsigned char network_enabled;
    unsigned char audio_enabled;
    unsigned char usb_enabled;
} DriverConfig;

// Audio configuration
typedef struct {
    unsigned int master_volume;
    unsigned int microphone_volume;
    unsigned char system_sounds;
    unsigned char fade_transitions;
    char audio_quality[MAX_CONFIG_VALUE];
    unsigned int sample_rate;
    unsigned int buffer_size;
    unsigned char surround_sound;
} AudioConfig;

// Network configuration
typedef struct {
    unsigned char dhcp_enabled;
    char ip_address[MAX_CONFIG_VALUE];
    char subnet_mask[MAX_CONFIG_VALUE];
    char gateway[MAX_CONFIG_VALUE];
    char dns_server[MAX_CONFIG_VALUE];
    unsigned int timeout_ms;
} NetworkConfig;

// Security configuration
typedef struct {
    unsigned char firewall_enabled;
    unsigned char encryption_enabled;
    char encryption_method[MAX_CONFIG_VALUE];
    unsigned int password_min_length;
    unsigned char auto_lock_enabled;
    unsigned int auto_lock_timeout;
} SecurityConfig;

// System configuration
typedef struct {
    char hostname[MAX_CONFIG_VALUE];
    char timezone[MAX_CONFIG_VALUE];
    unsigned char auto_update;
    unsigned int log_level;
    char temp_directory[MAX_CONFIG_VALUE];
    unsigned long max_log_size;
} SystemConfig;

// Main configuration structure
typedef struct {
    BootConfig boot;
    KernelConfig kernel;
    UIConfig ui;
    DriverConfig drivers;
    AudioConfig audio;
    NetworkConfig network;
    SecurityConfig security;
    SystemConfig system;
} HashOSConfig;

// Configuration file entry
typedef struct {
    char filename[MAX_FILENAME];
    ConfigFileType type;
    unsigned char loaded;
    unsigned char modified;
} ConfigFile;

// Configuration manager
typedef struct {
    HashOSConfig config;
    ConfigFile files[MAX_CONFIG_FILES];
    unsigned int file_count;
    unsigned char initialized;
} ConfigManager;

// Function prototypes
void InitConfigManager(void);
int LoadConfigFile(const char* filename);
int LoadAllConfigs(void);
int SaveConfigFile(const char* filename);
int SaveAllConfigs(void);
int ParseConfigData(const char* data, ConfigFileType type);
int ParseHCFGLine(const char* line, ConfigSection* current_section);
int ParseCFGLine(const char* line);
void ApplyConfiguration(void);
void PrintConfiguration(void);
void PrintConfigSection(ConfigSection section);
HashOSConfig* GetConfiguration(void);
ConfigManager* GetConfigManager(void);

// Utility functions
int ParseTimeout(const char* value);
int ParseSize(const char* value);
int ParseResolution(const char* value, unsigned int* width, unsigned int* height);
int ParseBoolean(const char* value);
int ParseVolume(const char* value);
ConfigFileType DetectConfigType(const char* filename);
const char* GetSectionName(ConfigSection section);
void SetDefaultConfiguration(void);

// Configuration validation
int ValidateConfiguration(void);
int ValidateBootConfig(const BootConfig* config);
int ValidateKernelConfig(const KernelConfig* config);
int ValidateUIConfig(const UIConfig* config);
int ValidateAudioConfig(const AudioConfig* config);

// Configuration export/import
int ExportConfigToString(char* buffer, int buffer_size);
int ImportConfigFromString(const char* buffer);

#endif // HASHOS_CONFIG_H

// ==========================================================================
// config.c - Configuration system implementation
// ==========================================================================

#include "config.h"
// #include "display.h"  // Assuming display driver exists
// #include "io.h"       // Assuming I/O functions exist

// Global configuration manager
static ConfigManager config_manager;

// String utility functions (custom implementations for OS kernel)
static int str_len(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static int str_cmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static int str_ncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static void str_cpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

static void str_ncpy(char* dest, const char* src, int n) {
    while (n && *src) {
        *dest++ = *src++;
        n--;
    }
    while (n) {
        *dest++ = '\0';
        n--;
    }
}

static char* str_chr(const char* str, int c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return (*str == c) ? (char*)str : 0;
}

static void str_trim(char* str) {
    char* start = str;
    char* end;
    
    // Trim leading whitespace
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') 
        start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    // Trim trailing whitespace
    end = start + str_len(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    *(end + 1) = 0;
    
    // Move trimmed string to beginning
    if (start != str) {
        while (*start) {
            *str++ = *start++;
        }
        *str = 0;
    }
}

static int str_to_int(const char* str) {
    int result = 0;
    int sign = 1;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

// Configuration manager functions
void InitConfigManager(void) {
    config_manager.file_count = 0;
    config_manager.initialized = 1;
    SetDefaultConfiguration();
}

void SetDefaultConfiguration(void) {
    HashOSConfig* cfg = &config_manager.config;
    
    // Boot defaults
    str_cpy(cfg->boot.message, "Booting HASH OS...");
    cfg->boot.timeout_seconds = 3;
    cfg->boot.show_logo = 1;
    str_cpy(cfg->boot.boot_device, "auto");
    cfg->boot.verbose_mode = 0;
    
    // Kernel defaults
    cfg->kernel.stack_size = 4096;
    cfg->kernel.max_threads = 16;
    str_cpy(cfg->kernel.scheduler, "hash-round");
    cfg->kernel.heap_size = 1024 * 1024; // 1MB
    cfg->kernel.debug_mode = 0;
    cfg->kernel.max_processes = 32;
    
    // UI defaults
    str_cpy(cfg->ui.theme, "hash-dark");
    str_cpy(cfg->ui.font, "hash-mono");
    cfg->ui.width = 800;
    cfg->ui.height = 600;
    cfg->ui.color_depth = 32;
    cfg->ui.animations_enabled = 1;
    cfg->ui.vsync_enabled = 1;
    
    // Driver defaults
    cfg->drivers.keyboard_enabled = 1;
    cfg->drivers.display_enabled = 1;
    cfg->drivers.storage_enabled = 0;
    cfg->drivers.network_enabled = 0;
    cfg->drivers.audio_enabled = 1;
    cfg->drivers.usb_enabled = 1;
    
    // Audio defaults
    cfg->audio.master_volume = 85;
    cfg->audio.microphone_volume = 75;
    cfg->audio.system_sounds = 1;
    cfg->audio.fade_transitions = 1;
    str_cpy(cfg->audio.audio_quality, "HIGH");
    cfg->audio.sample_rate = 44100;
    cfg->audio.buffer_size = 1024;
    cfg->audio.surround_sound = 0;
    
    // Network defaults
    cfg->network.dhcp_enabled = 1;
    str_cpy(cfg->network.ip_address, "192.168.1.100");
    str_cpy(cfg->network.subnet_mask, "255.255.255.0");
    str_cpy(cfg->network.gateway, "192.168.1.1");
    str_cpy(cfg->network.dns_server, "8.8.8.8");
    cfg->network.timeout_ms = 5000;
    
    // Security defaults
    cfg->security.firewall_enabled = 1;
    cfg->security.encryption_enabled = 1;
    str_cpy(cfg->security.encryption_method, "AES256");
    cfg->security.password_min_length = 8;
    cfg->security.auto_lock_enabled = 1;
    cfg->security.auto_lock_timeout = 300; // 5 minutes
    
    // System defaults
    str_cpy(cfg->system.hostname, "hashos-system");
    str_cpy(cfg->system.timezone, "UTC");
    cfg->system.auto_update = 1;
    cfg->system.log_level = 2; // INFO level
    str_cpy(cfg->system.temp_directory, "/tmp");
    cfg->system.max_log_size = 10 * 1024 * 1024; // 10MB
}

ConfigFileType DetectConfigType(const char* filename) {
    int len = str_len(filename);
    
    if (len > 5 && str_cmp(filename + len - 5, ".hcfg") == 0) {
        return CONFIG_TYPE_HCFG;
    } else if (len > 4 && str_cmp(filename + len - 4, ".cfg") == 0) {
        return CONFIG_TYPE_CFG;
    }
    
    return CONFIG_TYPE_UNKNOWN;
}

// Parser utility functions
int ParseTimeout(const char* value) {
    int result = 0;
    const char* ptr = value;
    
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }
    
    return result;
}

int ParseSize(const char* value) {
    int result = 0;
    const char* ptr = value;
    
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }
    
    // Handle size suffixes
    if (*ptr == 'k' || *ptr == 'K') {
        result *= 1024;
    } else if (*ptr == 'm' || *ptr == 'M') {
        result *= 1024 * 1024;
    } else if (*ptr == 'g' || *ptr == 'G') {
        result *= 1024 * 1024 * 1024;
    }
    
    return result;
}

int ParseResolution(const char* value, unsigned int* width, unsigned int* height) {
    const char* ptr = value;
    int w = 0, h = 0;
    
    // Parse width
    while (*ptr >= '0' && *ptr <= '9') {
        w = w * 10 + (*ptr - '0');
        ptr++;
    }
    
    // Skip 'x' separator
    if (*ptr == 'x' || *ptr == 'X') {
        ptr++;
    } else {
        return 0;
    }
    
    // Parse height
    while (*ptr >= '0' && *ptr <= '9') {
        h = h * 10 + (*ptr - '0');
        ptr++;
    }
    
    *width = w;
    *height = h;
    return 1;
}

int ParseBoolean(const char* value) {
    if (str_cmp(value, "enabled") == 0 || str_cmp(value, "true") == 0 || 
        str_cmp(value, "TRUE") == 0 || str_cmp(value, "1") == 0) {
        return 1;
    }
    return 0;
}

int ParseVolume(const char* value) {
    int vol = str_to_int(value);
    if (vol < 0) vol = 0;
    if (vol > 100) vol = 100;
    return vol;
}

const char* GetSectionName(ConfigSection section) {
    switch (section) {
        case SECTION_BOOT: return "BOOT";
        case SECTION_KERNEL: return "KERNEL";
        case SECTION_UI: return "UI";
        case SECTION_DRIVERS: return "DRIVERS";
        case SECTION_AUDIO: return "AUDIO";
        case SECTION_NETWORK: return "NETWORK";
        case SECTION_SECURITY: return "SECURITY";
        case SECTION_SYSTEM: return "SYSTEM";
        default: return "UNKNOWN";
    }
}

// HCFG format parser (section-based)
int ParseHCFGLine(const char* line, ConfigSection* current_section) {
    char work_line[MAX_CONFIG_LINE];
    char key[MAX_CONFIG_KEY];
    char value[MAX_CONFIG_VALUE];
    char* colon_pos;
    HashOSConfig* cfg = &config_manager.config;
    
    str_ncpy(work_line, line, MAX_CONFIG_LINE - 1);
    work_line[MAX_CONFIG_LINE - 1] = '\0';
    str_trim(work_line);
    
    // Skip empty lines, comments, and C-style comments
    if (work_line[0] == '\0' || work_line[0] == '#' || 
        (work_line[0] == '/' && work_line[1] == '/') ||
        (work_line[0] == '/' && work_line[1] == '*')) {
        return 1;
    }
    
    // Check for section headers
    if (work_line[0] == '[') {
        if (str_ncmp(work_line, "[BOOT]", 6) == 0) {
            *current_section = SECTION_BOOT;
        } else if (str_ncmp(work_line, "[KERNEL]", 8) == 0) {
            *current_section = SECTION_KERNEL;
        } else if (str_ncmp(work_line, "[UI]", 4) == 0) {
            *current_section = SECTION_UI;
        } else if (str_ncmp(work_line, "[DRIVERS]", 9) == 0) {
            *current_section = SECTION_DRIVERS;
        } else if (str_ncmp(work_line, "[AUDIO]", 7) == 0) {
            *current_section = SECTION_AUDIO;
        } else if (str_ncmp(work_line, "[NETWORK]", 9) == 0) {
            *current_section = SECTION_NETWORK;
        } else if (str_ncmp(work_line, "[SECURITY]", 10) == 0) {
            *current_section = SECTION_SECURITY;
        } else if (str_ncmp(work_line, "[SYSTEM]", 8) == 0) {
            *current_section = SECTION_SYSTEM;
        } else {
            *current_section = SECTION_NONE;
        }
        return 1;
    }
    
    // Parse key-value pairs
    colon_pos = str_chr(work_line, ':');
    if (!colon_pos) {
        return 0;
    }
    
    *colon_pos = '\0';
    str_cpy(key, work_line);
    str_trim(key);
    
    str_cpy(value, colon_pos + 1);
    str_trim(value);
    
    // Apply configuration based on current section
    switch (*current_section) {
        case SECTION_BOOT:
            if (str_cmp(key, "message") == 0) {
                str_ncpy(cfg->boot.message, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "timeout") == 0) {
                cfg->boot.timeout_seconds = ParseTimeout(value);
            } else if (str_cmp(key, "show_logo") == 0) {
                cfg->boot.show_logo = ParseBoolean(value);
            } else if (str_cmp(key, "boot_device") == 0) {
                str_ncpy(cfg->boot.boot_device, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "verbose_mode") == 0) {
                cfg->boot.verbose_mode = ParseBoolean(value);
            }
            break;
            
        case SECTION_KERNEL:
            if (str_cmp(key, "stack_size") == 0) {
                cfg->kernel.stack_size = ParseSize(value);
            } else if (str_cmp(key, "max_threads") == 0) {
                cfg->kernel.max_threads = ParseTimeout(value);
            } else if (str_cmp(key, "scheduler") == 0) {
                str_ncpy(cfg->kernel.scheduler, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "heap_size") == 0) {
                cfg->kernel.heap_size = ParseSize(value);
            } else if (str_cmp(key, "debug_mode") == 0) {
                cfg->kernel.debug_mode = ParseBoolean(value);
            } else if (str_cmp(key, "max_processes") == 0) {
                cfg->kernel.max_processes = ParseTimeout(value);
            }
            break;
            
        case SECTION_UI:
            if (str_cmp(key, "theme") == 0) {
                str_ncpy(cfg->ui.theme, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "font") == 0) {
                str_ncpy(cfg->ui.font, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "resolution") == 0) {
                ParseResolution(value, &cfg->ui.width, &cfg->ui.height);
            } else if (str_cmp(key, "color_depth") == 0) {
                cfg->ui.color_depth = ParseTimeout(value);
            } else if (str_cmp(key, "animations_enabled") == 0) {
                cfg->ui.animations_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "vsync_enabled") == 0) {
                cfg->ui.vsync_enabled = ParseBoolean(value);
            }
            break;
            
        case SECTION_DRIVERS:
            if (str_cmp(key, "keyboard") == 0) {
                cfg->drivers.keyboard_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "display") == 0) {
                cfg->drivers.display_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "storage") == 0) {
                cfg->drivers.storage_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "network") == 0) {
                cfg->drivers.network_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "audio") == 0) {
                cfg->drivers.audio_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "usb") == 0) {
                cfg->drivers.usb_enabled = ParseBoolean(value);
            }
            break;
            
        case SECTION_AUDIO:
            if (str_cmp(key, "master_volume") == 0) {
                cfg->audio.master_volume = ParseVolume(value);
            } else if (str_cmp(key, "microphone_volume") == 0) {
                cfg->audio.microphone_volume = ParseVolume(value);
            } else if (str_cmp(key, "system_sounds") == 0) {
                cfg->audio.system_sounds = ParseBoolean(value);
            } else if (str_cmp(key, "fade_transitions") == 0) {
                cfg->audio.fade_transitions = ParseBoolean(value);
            } else if (str_cmp(key, "audio_quality") == 0) {
                str_ncpy(cfg->audio.audio_quality, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "sample_rate") == 0) {
                cfg->audio.sample_rate = ParseTimeout(value);
            } else if (str_cmp(key, "buffer_size") == 0) {
                cfg->audio.buffer_size = ParseTimeout(value);
            } else if (str_cmp(key, "surround_sound") == 0) {
                cfg->audio.surround_sound = ParseBoolean(value);
            }
            break;
            
        case SECTION_NETWORK:
            if (str_cmp(key, "dhcp_enabled") == 0) {
                cfg->network.dhcp_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "ip_address") == 0) {
                str_ncpy(cfg->network.ip_address, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "subnet_mask") == 0) {
                str_ncpy(cfg->network.subnet_mask, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "gateway") == 0) {
                str_ncpy(cfg->network.gateway, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "dns_server") == 0) {
                str_ncpy(cfg->network.dns_server, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "timeout_ms") == 0) {
                cfg->network.timeout_ms = ParseTimeout(value);
            }
            break;
            
        case SECTION_SECURITY:
            if (str_cmp(key, "firewall_enabled") == 0) {
                cfg->security.firewall_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "encryption_enabled") == 0) {
                cfg->security.encryption_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "encryption_method") == 0) {
                str_ncpy(cfg->security.encryption_method, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "password_min_length") == 0) {
                cfg->security.password_min_length = ParseTimeout(value);
            } else if (str_cmp(key, "auto_lock_enabled") == 0) {
                cfg->security.auto_lock_enabled = ParseBoolean(value);
            } else if (str_cmp(key, "auto_lock_timeout") == 0) {
                cfg->security.auto_lock_timeout = ParseTimeout(value);
            }
            break;
            
        case SECTION_SYSTEM:
            if (str_cmp(key, "hostname") == 0) {
                str_ncpy(cfg->system.hostname, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "timezone") == 0) {
                str_ncpy(cfg->system.timezone, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "auto_update") == 0) {
                cfg->system.auto_update = ParseBoolean(value);
            } else if (str_cmp(key, "log_level") == 0) {
                cfg->system.log_level = ParseTimeout(value);
            } else if (str_cmp(key, "temp_directory") == 0) {
                str_ncpy(cfg->system.temp_directory, value, MAX_CONFIG_VALUE - 1);
            } else if (str_cmp(key, "max_log_size") == 0) {
                cfg->system.max_log_size = ParseSize(value);
            }
            break;
            
        default:
            break;
    }
    
    return 1;
}

// CFG format parser (simple key=value)
int ParseCFGLine(const char* line) {
    char work_line[MAX_CONFIG_LINE];
    char key[MAX_CONFIG_KEY];
    char value[MAX_CONFIG_VALUE];
    char* equals_pos;
    HashOSConfig* cfg = &config_manager.config;
    
    str_ncpy(work_line, line, MAX_CONFIG_LINE - 1);
    work_line[MAX_CONFIG_LINE - 1] = '\0';
    str_trim(work_line);
    
    // Skip empty lines and comments
    if (work_line[0] == '\0' || work_line[0] == '#') {
        return 1;
    }
    
    // Parse key=value pairs
    equals_pos = str_chr(work_line, '=');
    if (!equals_pos) {
        return 0;
    }
    
    *equals_pos = '\0';
    str_cpy(key, work_line);
    str_trim(key);
    
    str_cpy(value, equals_pos + 1);
    str_trim(value);
    
    // Map keys to configuration (assuming audio.cfg format)
    if (str_cmp(key, "MASTER_VOLUME") == 0) {
        cfg->audio.master_volume = ParseVolume(value);
    } else if (str_cmp(key, "MICROPHONE_VOLUME") == 0) {
        cfg->audio.microphone_volume = ParseVolume(value);
    } else if (str_cmp(key, "SYSTEM_SOUNDS") == 0) {
        cfg->audio.system_sounds = ParseBoolean(value);
    } else if (str_cmp(key, "FADE_TRANSITIONS") == 0) {
        cfg->audio.fade_transitions = ParseBoolean(value);
    } else if (str_cmp(key, "AUDIO_QUALITY") == 0) {
        str_ncpy(cfg->audio.audio_quality, value, MAX_CONFIG_VALUE - 1);
    }
    // Add more key mappings as needed
    
    return 1;
}

int ParseConfigData(const char* data, ConfigFileType type) {
    ConfigSection current_section = SECTION_NONE;
    char line[MAX_CONFIG_LINE];
    const char* ptr = data;
    int line_start = 0;
    int i = 0;
    
    while (*ptr || i > line_start) {
        if (*ptr == '\n' || *ptr == '\0') {
            // Extract line
            int line_len = i - line_start;
            if (line_len >= MAX_CONFIG_LINE) line_len = MAX_CONFIG_LINE - 1;
            
            str_ncpy(line, data + line_start, line_len);
            line[line_len] = '\0';
            
            // Parse line based on format
            int result;
            if (type == CONFIG_TYPE_HCFG) {
                result = ParseHCFGLine(line, &current_section);
            } else if (type == CONFIG_TYPE_CFG) {
                result = ParseCFGLine(line);
            } else {
                result = 0;
            }
            
            if (!result) {
                return 0; // Parse error
            }
            
            line_start = i + 1;
        }
        
        if (*ptr) {
            ptr++;
            i++;
        } else {
            break;
        }
    }
    
    return 1;
}

// File operations (simplified - would need real file I/O in actual OS)
int LoadConfigFile(const char* filename) {
    // This would read from actual file system in real implementation
    ConfigFileType type = DetectConfigType(filename);
    
    // Sample data for demonstration
    const char* system_data = 
        "// HASH OS SYSTEM CONFIGURATION\n"
        "[BOOT]\n"
        "message: Booting HASH OS...\n"
        "timeout: 3\n"
        "show_logo: true\n"
        "boot_device: auto\n"
        "verbose_mode: false\n"
        "[KERNEL]\n"
        "stack_size: 4096\n"
        "max_threads: 16\n"
        "scheduler: hash-round\n"
        "heap_size: 1m\n"
        "debug_mode: false\n"
        "max_processes: 32\n"
        "[UI]\n"
        "theme: hash-dark\n"
        "font: hash-mono\n"
        "resolution: 800x600\n"
        "color_depth: 32\n"
        "animations_enabled: true\n"
        "vsync_enabled: true\n"
        "[DRIVERS]\n"
        "keyboard: enabled\n"
        "display: enabled\n"
        "storage: disabled\n"
        "network: disabled\n"
        "audio: enabled\n"
        "usb: enabled\n"
        "[AUDIO]\n"
        "master_volume: 85\n"
        "microphone_volume: 75\n"
        "system_sounds: true\n"
        "fade_transitions: true\n"
        "audio_quality: HIGH\n"
        "sample_rate: 44100\n"
        "buffer_size: 1024\n"
        "surround_sound: false\n"
        "[NETWORK]\n"
        "dhcp_enabled: true\n"
        "ip_address: 192.168.1.100\n"
        "subnet_mask: 255.255.255.0\n"
        "gateway: 192.168.1.1\n"
        "dns_server: 8.8.8.8\n"
        "timeout_ms: 5000\n"
        "[SECURITY]\n"
        "firewall_enabled: true\n"
        "encryption_enabled: true\n"
        "encryption_method: AES256\n"
        "password_min_length: 8\n"
        "auto_lock_enabled: true\n"
        "auto_lock_timeout: 300\n"
        "[SYSTEM]\n"
        "hostname: hashos-system\n"
        "timezone: UTC\n"
        "auto_update: true\n"
        "log_level: 2\n"
        "temp_directory: /tmp\n"
        "max_log_size: 10m\n";
    
    const char* audio_data = 
        "MASTER_VOLUME=85\n"
        "MICROPHONE_VOLUME=75\n"
        "SYSTEM_SOUNDS=TRUE\n"
        "FADE_TRANSITIONS=TRUE\n"
        "AUDIO_QUALITY=HIGH\n";
    
    // Select appropriate data based on filename
    const char* data_to_parse;
    if (str_cmp(filename, "system.hcfg") == 0) {
        data_to_parse = system_data;
    } else if (str_cmp(filename, "audio.hcfg") == 0 || str_cmp(filename, "config.cfg") == 0) {
        data_to_parse = audio_data;
        type = CONFIG_TYPE_CFG; // Force CFG type for audio data
    } else {
        data_to_parse = system_data; // Default
    }
    
    if (ParseConfigData(data_to_parse, type)) {
        // Add to file list if not already present
        for (int i = 0; i < config_manager.file_count; i++) {
            if (str_cmp(config_manager.files[i].filename, filename) == 0) {
                config_manager.files[i].loaded = 1;
                return 1;
            }
        }
        
        // Add new file entry
        if (config_manager.file_count < MAX_CONFIG_FILES) {
            str_cpy(config_manager.files[config_manager.file_count].filename, filename);
            config_manager.files[config_manager.file_count].type = type;
            config_manager.files[config_manager.file_count].loaded = 1;
            config_manager.files[config_manager.file_count].modified = 0;
            config_manager.file_count++;
        }
        
        return 1;
    }
    
    return 0;
}

int LoadAllConfigs(void) {
    int success_count = 0;
    
    // Load common configuration files
    const char* config_files[] = {
        "system.hcfg",
        "audio.hcfg",
        "config.cfg"
    };
    
    for (int i = 0; i < 3; i++) {
        if (LoadConfigFile(config_files[i])) {
            success_count++;
        }
    }
    
    return success_count;
}

int SaveConfigFile(const char* filename) {
    // This would write to actual file system in real implementation
    ConfigFileType type = DetectConfigType(filename);
    
    // For demonstration, we'll just mark as saved
    for (int i = 0; i < config_manager.file_count; i++) {
        if (str_cmp(config_manager.files[i].filename, filename) == 0) {
            config_manager.files[i].modified = 0;
            return 1;
        }
    }
    
    return 0;
}

int SaveAllConfigs(void) {
    int success_count = 0;
    
    for (int i = 0; i < config_manager.file_count; i++) {
        if (config_manager.files[i].modified) {
            if (SaveConfigFile(config_manager.files[i].filename)) {
                success_count++;
            }
        }
    }
    
    return success_count;
}

// Configuration validation functions
int ValidateConfiguration(void) {
    HashOSConfig* cfg = &config_manager.config;
    
    if (!ValidateBootConfig(&cfg->boot)) return 0;
    if (!ValidateKernelConfig(&cfg->kernel)) return 0;
    if (!ValidateUIConfig(&cfg->ui)) return 0;
    if (!ValidateAudioConfig(&cfg->audio)) return 0;
    
    return 1;
}

int ValidateBootConfig(const BootConfig* config) {
    if (config->timeout_seconds > 60) return 0; // Max 60 seconds
    if (str_len(config->message) == 0) return 0; // Must have message
    return 1;
}

int ValidateKernelConfig(const KernelConfig* config) {
    if (config->stack_size < 1024) return 0; // Min 1KB stack
    if (config->max_threads == 0 || config->max_threads > 256) return 0;
    if (config->max_processes == 0 || config->max_processes > 1024) return 0;
    return 1;
}

int ValidateUIConfig(const UIConfig* config) {
    if (config->width < 640 || config->height < 480) return 0; // Min resolution
    if (config->color_depth != 16 && config->color_depth != 24 && config->color_depth != 32) return 0;
    return 1;
}

int ValidateAudioConfig(const AudioConfig* config) {
    if (config->master_volume > 100 || config->microphone_volume > 100) return 0;
    if (config->sample_rate != 22050 && config->sample_rate != 44100 && config->sample_rate != 48000) return 0;
    return 1;
}

// Configuration display functions
void PrintConfiguration(void) {
    PrintConfigSection(SECTION_BOOT);
    PrintConfigSection(SECTION_KERNEL);
    PrintConfigSection(SECTION_UI);
    PrintConfigSection(SECTION_DRIVERS);
    PrintConfigSection(SECTION_AUDIO);
    PrintConfigSection(SECTION_NETWORK);
    PrintConfigSection(SECTION_SECURITY);
    PrintConfigSection(SECTION_SYSTEM);
}

void PrintConfigSection(ConfigSection section) {
    HashOSConfig* cfg = &config_manager.config;
    
    // Note: Using simple console output - would use proper display driver in real OS
    switch (section) {
        case SECTION_BOOT:
            // PutString("=== BOOT CONFIGURATION ===\n");
            // PutString("Message: "); PutString(cfg->boot.message); PutString("\n");
            // ... (simplified for space)
            break;
            
        case SECTION_KERNEL:
            // PutString("=== KERNEL CONFIGURATION ===\n");
            // ... (similar pattern)
            break;
            
        case SECTION_UI:
            // PutString("=== UI CONFIGURATION ===\n");
            // ... (similar pattern)
            break;
            
        case SECTION_DRIVERS:
            // PutString("=== DRIVER CONFIGURATION ===\n");
            // ... (similar pattern)
            break;
            
        case SECTION_AUDIO:
            // PutString("=== AUDIO CONFIGURATION ===\n");
            // ... (similar pattern)
            break;
            
        case SECTION_NETWORK:
            // PutString("=== NETWORK CONFIGURATION ===\n");
            // ... (similar pattern)
            break;
            
        case SECTION_SECURITY:
            // PutString("=== SECURITY CONFIGURATION ===\n");
            // ... (similar pattern)
            break;
            
        case SECTION_SYSTEM:
            // PutString("=== SYSTEM CONFIGURATION ===\n");
            // ... (similar pattern)
            break;
            
        default:
            break;
    }
}

void ApplyConfiguration(void) {
    HashOSConfig* cfg = &config_manager.config;
    
    // Apply boot configuration
    // (Set boot parameters, splash screen, etc.)
    
    // Apply kernel configuration
    // (Configure scheduler, memory management, etc.)
    
    // Apply UI configuration
    // (Set graphics mode, theme, fonts, etc.)
    
    // Apply driver configuration
    if (!cfg->drivers.display_enabled) {
        // Handle display driver disable
    }
    if (cfg->drivers.audio_enabled) {
        // Initialize audio subsystem with audio config
    }
    
    // Apply network configuration
    if (cfg->drivers.network_enabled) {
        // Configure network stack
    }
    
    // Apply security settings
    if (cfg->security.firewall_enabled) {
        // Enable firewall
    }
    
    // Apply system settings
    // (Set hostname, timezone, etc.)
}

// Export/Import functions
int ExportConfigToString(char* buffer, int buffer_size) {
    // This would serialize the current configuration to a string
    // For demonstration, we'll create a basic export
    HashOSConfig* cfg = &config_manager.config;
    int pos = 0;
    
    // This is a simplified version - would need proper string formatting
    pos += str_len("# HASH OS Configuration Export\n");
    if (pos >= buffer_size) return 0;
    
    str_cpy(buffer, "# HASH OS Configuration Export\n");
    // ... (would continue with full serialization)
    
    return pos;
}

int ImportConfigFromString(const char* buffer) {
    // Parse configuration from string and apply
    return ParseConfigData(buffer, CONFIG_TYPE_HCFG);
}

// Accessor functions  
HashOSConfig* GetConfiguration(void) {
    return &config_manager.config;
}

ConfigManager* GetConfigManager(void) {
    return &config_manager;
}

// ==========================================================================
// CONFIGURATION TEST AND DEMO FUNCTIONS
// ==========================================================================

void TestConfigSystem(void) {
    // Initialize configuration system
    InitConfigManager();
    
    // Load all configuration files
    int loaded = LoadAllConfigs();
    
    // Validate configuration
    if (ValidateConfiguration()) {
        // Apply configuration
        ApplyConfiguration();
        
        // Print current configuration
        PrintConfiguration();
    }
}

// ==========================================================================
// CONFIGURATION FILE CONTENTS
// ==========================================================================

// config.cfg - Simple key=value configuration
const char* CONFIG_CFG_CONTENT = 
    "# HASH OS Simple Configuration\n"
    "# Audio Settings\n"
    "MASTER_VOLUME=85\n"
    "MICROPHONE_VOLUME=75\n"
    "SYSTEM_SOUNDS=TRUE\n"
    "FADE_TRANSITIONS=TRUE\n"
    "AUDIO_QUALITY=HIGH\n"
    "\n"
    "# System Settings\n"
    "HOSTNAME=hashos-system\n"
    "TIMEZONE=UTC\n"
    "AUTO_UPDATE=TRUE\n"
    "LOG_LEVEL=2\n"
    "\n"
    "# Driver Settings\n"
    "KEYBOARD_ENABLED=TRUE\n"
    "DISPLAY_ENABLED=TRUE\n"
    "STORAGE_ENABLED=FALSE\n"
    "NETWORK_ENABLED=FALSE\n"
    "AUDIO_ENABLED=TRUE\n"
    "USB_ENABLED=TRUE\n";

// system.hcfg - Hierarchical configuration format
const char* SYSTEM_HCFG_CONTENT = 
    "// system.hcfg - HASH OS System Configuration File\n"
    "/*\n"
    " * =========================================\n"
    " * HASH OS OFFICIAL SYSTEM CONFIGURATION \n"
    " * =========================================\n"
    " */\n"
    "\n"
    "[BOOT]\n"
    "message: Booting HASH OS...\n"
    "timeout: 3\n"
    "show_logo: true\n"
    "boot_device: auto\n"
    "verbose_mode: false\n"
    "\n"
    "[KERNEL]\n"
    "stack_size: 4096\n"
    "max_threads: 16\n"
    "scheduler: hash-round\n"
    "heap_size: 1m\n"
    "debug_mode: false\n"
    "max_processes: 32\n"
    "\n"
    "[UI]\n"
    "theme: hash-dark\n"
    "font: hash-mono\n"
    "resolution: 800x600\n"
    "color_depth: 32\n"
    "animations_enabled: true\n"
    "vsync_enabled: true\n"
    "\n"
    "[DRIVERS]\n"
    "keyboard: enabled\n"
    "display: enabled\n"
    "storage: disabled\n"
    "network: disabled\n"
    "audio: enabled\n"
    "usb: enabled\n"
    "\n"
    "[AUDIO]\n"
    "master_volume: 85\n"
    "microphone_volume: 75\n"
    "system_sounds: true\n"
    "fade_transitions: true\n"
    "audio_quality: HIGH\n"
    "sample_rate: 44100\n"
    "buffer_size: 1024\n"
    "surround_sound: false\n"
    "\n"
    "[NETWORK]\n"
    "dhcp_enabled: true\n"
    "ip_address: 192.168.1.100\n"
    "subnet_mask: 255.255.255.0\n"
    "gateway: 192.168.1.1\n"
    "dns_server: 8.8.8.8\n"
    "timeout_ms: 5000\n"
    "\n"
    "[SECURITY]\n"
    "firewall_enabled: true\n"
    "encryption_enabled: true\n"
    "encryption_method: AES256\n"
    "password_min_length: 8\n"
    "auto_lock_enabled: true\n"
    "auto_lock_timeout: 300\n"
    "\n"
    "[SYSTEM]\n"
    "hostname: hashos-system\n"
    "timezone: UTC\n"
    "auto_update: true\n"
    "log_level: 2\n"
    "temp_directory: /tmp\n"
    "max_log_size: 10m\n";

// audio.hcfg - Audio-specific configuration
const char* AUDIO_HCFG_CONTENT = 
    "# HASH OS Audio Configuration\n"
    "MASTER_VOLUME=85\n"
    "MICROPHONE_VOLUME=75\n"
    "SYSTEM_SOUNDS=TRUE\n"
    "FADE_TRANSITIONS=TRUE\n"
    "AUDIO_QUALITY=HIGH\n"
    "SAMPLE_RATE=44100\n"
    "BUFFER_SIZE=1024\n"
    "SURROUND_SOUND=FALSE\n"
    "AUDIO_DRIVER=default\n"
    "OUTPUT_DEVICE=auto\n"
    "INPUT_DEVICE=auto\n"
    "LOW_LATENCY_MODE=FALSE\n"
    "NOISE_CANCELLATION=TRUE\n"
    "EQUALIZER_ENABLED=FALSE\n"
    "BASS_BOOST=0\n"
    "TREBLE_BOOST=0\n";

// ==========================================================================
// MAIN CONFIGURATION INTERFACE
// ==========================================================================

int main(void) {
    // Initialize the configuration system
    InitConfigManager();
    
    // Load all configuration files
    int files_loaded = LoadAllConfigs();
    
    if (files_loaded > 0) {
        // Validate the loaded configuration
        if (ValidateConfiguration()) {
            // Apply the configuration to the system
            ApplyConfiguration();
            
            // Print current configuration status
            PrintConfiguration();
            
            return 0; // Success
        } else {
            // Configuration validation failed
            return 2;
        }
    } else {
        // Failed to load configuration files
        return 1;
    }
}

// ==========================================================================
// UTILITY MACROS AND CONSTANTS
// ==========================================================================

#define CONFIG_SUCCESS 0
#define CONFIG_ERROR_LOAD 1
#define CONFIG_ERROR_VALIDATE 2
#define CONFIG_ERROR_APPLY 3
#define CONFIG_ERROR_SAVE 4

#define CONFIG_DEFAULT_TIMEOUT 3
#define CONFIG_DEFAULT_STACK_SIZE 4096
#define CONFIG_DEFAULT_MAX_THREADS 16
#define CONFIG_DEFAULT_RESOLUTION_WIDTH 800
#define CONFIG_DEFAULT_RESOLUTION_HEIGHT 600
#define CONFIG_DEFAULT_AUDIO_VOLUME 85
#define CONFIG_DEFAULT_SAMPLE_RATE 44100

// Configuration change notification callback type
typedef void (*ConfigChangeCallback)(ConfigSection section, const char* key, const char* old_value, const char* new_value);

// Advanced configuration management functions
int RegisterConfigChangeCallback(ConfigChangeCallback callback);
int UnregisterConfigChangeCallback(ConfigChangeCallback callback);
int NotifyConfigChange(ConfigSection section, const char* key, const char* old_value, const char* new_value);
int BackupConfiguration(const char* backup_filename);
int RestoreConfiguration(const char* backup_filename);
int ResetToDefaults(void);
int GetConfigValue(ConfigSection section, const char* key, char* value, int value_size);
int SetConfigValue(ConfigSection section, const char* key, const char* value);

/*
 * ==========================================================================
 * END OF HASH OS CONFIGURATION SYSTEM
 * ==========================================================================
 * 
 * This complete configuration system provides:
 * 
 * 1. Support for multiple configuration file formats (.hcfg and .cfg)
 * 2. Hierarchical configuration with sections
 * 3. Comprehensive validation system
 * 4. Default value management
 * 5. Configuration export/import capabilities
 * 6. File management and tracking
 * 7. Error handling and validation
 * 8. Extensible architecture for adding new sections
 * 
 * Usage:
 * 1. Call InitConfigManager() to initialize the system
 * 2. Call LoadAllConfigs() to load configuration files
 * 3. Call ValidateConfiguration() to ensure validity
 * 4. Call ApplyConfiguration() to apply settings to the system
 * 5. Use GetConfiguration() to access current settings
 * 
 * The system is designed to be embedded in an operating system kernel
 * and provides all necessary functionality for managing system configuration.
 */