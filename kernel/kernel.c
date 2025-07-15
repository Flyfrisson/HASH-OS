// kernel/kernel.c - Corrected HASH OS Kernel with improved error handling

#include <stddef.h>
#include <stdint.h>
#include "config_parser.h"
#include "display4k.h"
#include "../drivers/driver.h"
#include "fs.h"
#include "ui_manager.h"
#include "app_manager.h"
#include "../drivers/audio_manager.h"
#include "../ui/file_explorer.h"
#include "../ui/settings.h"
#include "../ui/launcher.h"

// Constants
#define MIN_FRAMEBUFFER_ADDRESS 0x100000
#define MAX_FRAMEBUFFER_ADDRESS 0xFFFFFFFF
#define DEFAULT_SCREEN_WIDTH    1024
#define DEFAULT_SCREEN_HEIGHT   768
#define TARGET_4K_WIDTH         3840
#define TARGET_4K_HEIGHT        2160
#define MIN_APPS_REQUIRED       2
#define MAX_PRIORITY            10
#define MIN_PRIORITY            0

// Panic screen colors
#define PANIC_BG_COLOR     0xFF0000
#define PANIC_TEXT_COLOR   0xFFFFFF
#define SHUTDOWN_BG_COLOR  0x000080
#define RECOVERY_BG_COLOR  0x808000

// Global framebuffer pointer
// Use exactly the same type as declared in display4k.h
uint32_t* framebuffer = NULL;  // Define it globally


// Match with display4k.h declaration

// Screen dimensions
static unsigned int screen_width = DEFAULT_SCREEN_WIDTH;
static unsigned int screen_height = DEFAULT_SCREEN_HEIGHT;

// System state
static int graphics_initialized = 0;
static int drivers_initialized = 0;
static int filesystem_initialized = 0;
static int apps_initialized = 0;

// Function declarations
void kernel_panic(const char* message);
void null_ui_loop(void);
void null_background_loop(void);
int get_screen_dimensions(unsigned int* width, unsigned int* height);
int register_app_safe(const char* name, void (*ui_func)(void), void (*bg_func)(void), int priority);
int validate_framebuffer(unsigned int address);
int init_graphics(unsigned int framebuffer_address);
int init_system_apps(void);
int system_health_check(void);
void run_scheduler(void);
void kernel_shutdown(void);
void emergency_recovery(void);

// Panic screen
void kernel_panic(const char* message) {
    (void)message;
    if (framebuffer && graphics_initialized) {
        unsigned int total_pixels = screen_width * screen_height;
        for (unsigned int i = 0; i < total_pixels; i++) {
            framebuffer[i] = PANIC_BG_COLOR;
        }

        unsigned int rect_start_x = screen_width / 8;
        unsigned int rect_start_y = screen_height / 4;
        unsigned int rect_width   = screen_width * 3 / 4;
        unsigned int rect_height  = screen_height / 8;

        for (unsigned int y = rect_start_y; y < rect_start_y + rect_height && y < screen_height; y++) {
            for (unsigned int x = rect_start_x; x < rect_start_x + rect_width && x < screen_width; x++) {
                framebuffer[y * screen_width + x] = PANIC_TEXT_COLOR;
            }
        }
    }

    while (1) {
        __asm__ volatile ("cli\n\thlt");
    }
}

// Null function loops
void null_ui_loop(void) {}
void null_background_loop(void) {}

// Screen detection
int get_screen_dimensions(unsigned int* width, unsigned int* height) {
    if (!width || !height) return 0;

    *width = TARGET_4K_WIDTH;
    *height = TARGET_4K_HEIGHT;
    return 1;
}

// App registration with validation
int register_app_safe(const char* name, void (*ui_func)(void), void (*bg_func)(void), int priority) {
    int name_len = 0;
    const char* temp = name;
    while (*temp != '\0' && name_len < 256) {
        temp++;
        name_len++;
    }

    if (name_len == 0 || name_len >= 256) return -1;
    if (!ui_func && !bg_func) return -2;
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) return -3;
    if (!apps_initialized) return -4;

    void (*safe_ui_func)(void) = ui_func ? ui_func : null_ui_loop;
    void (*safe_bg_func)(void) = bg_func ? bg_func : null_background_loop;

    register_app((char*)name, safe_ui_func, safe_bg_func, priority);
    return 0;
}

// Framebuffer address validation
int validate_framebuffer(unsigned int address) {
    if (address == 0) return 0;
    if (address < MIN_FRAMEBUFFER_ADDRESS) return 0;
    if (address > MAX_FRAMEBUFFER_ADDRESS - (TARGET_4K_WIDTH * TARGET_4K_HEIGHT * 4)) return 0;
    if (address % 4 != 0) return 0;
    return 1;
}

// Graphics initialization
int init_graphics(unsigned int framebuffer_address) {
    graphics_initialized = 0;

    if (!validate_framebuffer(framebuffer_address)) return 0;
    framebuffer = (uint32_t*)(uintptr_t)framebuffer_address;

    if (!get_screen_dimensions(&screen_width, &screen_height)) {
        screen_width = DEFAULT_SCREEN_WIDTH;
        screen_height = DEFAULT_SCREEN_HEIGHT;
    }

    if (screen_width == 0 || screen_height == 0 || screen_width > 7680 || screen_height > 4320) {
        screen_width = DEFAULT_SCREEN_WIDTH;
        screen_height = DEFAULT_SCREEN_HEIGHT;
    }

    unsigned int total_pixels = screen_width * screen_height;
    for (unsigned int i = 0; i < total_pixels; i++) {
        framebuffer[i] = 0x000000;
    }

    unsigned int test_rect_width = screen_width / 3;
    unsigned int test_rect_height = screen_height / 6;
    unsigned int center_x = screen_width / 2;
    unsigned int center_y = screen_height / 2;
    unsigned int rect_start_x = center_x - test_rect_width / 2;
    unsigned int rect_start_y = center_y - test_rect_height / 2;

    if (rect_start_x + test_rect_width <= screen_width && rect_start_y + test_rect_height <= screen_height) {
        for (unsigned int y = rect_start_y; y < rect_start_y + test_rect_height; y++) {
            for (unsigned int x = rect_start_x; x < rect_start_x + test_rect_width; x++) {
                framebuffer[y * screen_width + x] = 0xFF5733;
            }
        }

        unsigned int small_rect_width = test_rect_width / 2;
        unsigned int small_rect_height = test_rect_height / 2;
        unsigned int small_rect_x = center_x - small_rect_width / 2;
        unsigned int small_rect_y = center_y - small_rect_height / 2;

        for (unsigned int y = small_rect_y; y < small_rect_y + small_rect_height; y++) {
            for (unsigned int x = small_rect_x; x < small_rect_x + small_rect_width; x++) {
                framebuffer[y * screen_width + x] = 0x33FF57;
            }
        }
    }

    graphics_initialized = 1;
    return 1;
}

// Initialize all system apps
int init_system_apps(void) {
    int apps_registered = 0;
    int registration_errors = 0;

    init_apps();  // fixed: was previously used in if()
    apps_initialized = 1;

    int result = register_app_safe("Launcher", launcher_ui_loop, null_background_loop, 9);
    if (result >= 0) apps_registered++; else registration_errors++;

    result = register_app_safe("File Explorer", file_explorer_ui_loop, null_background_loop, 8);
    if (result >= 0) apps_registered++; else registration_errors++;

    result = register_app_safe("Settings", settings_ui_loop, null_background_loop, 7);
    if (result >= 0) apps_registered++; else registration_errors++;

    result = register_app_safe("Filesystem", null_ui_loop, filesystem_background_loop, 10);
    if (result >= 0) apps_registered++; else registration_errors++;

    result = register_app_safe("Audio Manager", null_ui_loop, audio_manager_background_loop, 9);
    if (result >= 0) apps_registered++; else registration_errors++;

    if (apps_registered < MIN_APPS_REQUIRED) return 0;
    return 1;
}

// Check system status
int system_health_check(void) {
    if (!graphics_initialized || !framebuffer) return 0;
    if (!drivers_initialized || !filesystem_initialized || !apps_initialized) return 0;
    return 1;
}

// Main kernel entry point
void kernel_main(unsigned int framebuffer_address) {
    if (!init_graphics(framebuffer_address)) {
        kernel_panic("Graphics initialization failed");
    }

    init_drivers();  // corrected: removed if()
    drivers_initialized = 1;

    parse_config();  // corrected: removed if()

    init_filesystem();  // corrected: removed if()
    filesystem_initialized = 1;

    list_root_directory();  // void call

    if (!init_system_apps()) {
        kernel_panic("Application system initialization failed");
    }

    if (!system_health_check()) {
        kernel_panic("System health check failed before scheduler start");
    }

    run_scheduler();  // fixed: do not use in if()

    kernel_panic("Scheduler returned unexpectedly");
}

// Shutdown logic
void kernel_shutdown(void) {
    asm volatile("cli");

    if (framebuffer && graphics_initialized) {
        unsigned int total_pixels = screen_width * screen_height;
        for (unsigned int i = 0; i < total_pixels; i++) {
            framebuffer[i] = SHUTDOWN_BG_COLOR;
        }
    }

    graphics_initialized = 0;
    drivers_initialized = 0;
    filesystem_initialized = 0;
    apps_initialized = 0;

    while (1) {
        asm volatile("hlt");
    }
}

// Recovery logic
void emergency_recovery(void) {
    if (framebuffer && graphics_initialized) {
        unsigned int total_pixels = screen_width * screen_height;
        for (unsigned int i = 0; i < total_pixels; i++) {
            framebuffer[i] = RECOVERY_BG_COLOR;
        }
    }

    if (!system_health_check()) {
        kernel_panic("Emergency recovery failed");
    }
}
