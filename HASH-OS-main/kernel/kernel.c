// kernel/kernel.c - Corrected HASH OS Kernel with improved error handling
#include "config_parser.h"
#include "display4k.h"
#include "../drivers/driver.h" 
#include "fs.h"
#include "ui_manager.h"
#include "app_manager.h"
#include "../drivers/audio_manager.h"
// Add app headers
#include "../ui/file_explorer.h"
#include "../ui/settings.h"
#include "../ui/launcher.h"

// Constants
#define MIN_FRAMEBUFFER_ADDRESS 0x100000  // 1MB minimum
#define MAX_FRAMEBUFFER_ADDRESS 0xFFFFFFFF
#define DEFAULT_SCREEN_WIDTH    1024
#define DEFAULT_SCREEN_HEIGHT   768
#define TARGET_4K_WIDTH         3840
#define TARGET_4K_HEIGHT        2160
#define MIN_APPS_REQUIRED       2
#define MAX_PRIORITY            10
#define MIN_PRIORITY            0

// Panic screen colors
#define PANIC_BG_COLOR     0xFF0000  // Red
#define PANIC_TEXT_COLOR   0xFFFFFF  // White
#define SHUTDOWN_BG_COLOR  0x000080  // Blue
#define RECOVERY_BG_COLOR  0x808000  // Yellow

// Global framebuffer pointer - marked as volatile for hardware access
static volatile unsigned int* framebuffer = NULL;

// Screen dimensions (protected with validation)
static unsigned int screen_width = DEFAULT_SCREEN_WIDTH;
static unsigned int screen_height = DEFAULT_SCREEN_HEIGHT;

// System state tracking
static int graphics_initialized = 0;
static int drivers_initialized = 0;
static int filesystem_initialized = 0;
static int apps_initialized = 0;

// Forward declarations
void kernel_panic(const char* message);
void null_ui_loop(void);
void null_background_loop(void);
int get_screen_dimensions(unsigned int* width, unsigned int* height);
int register_app_safe(const char* name, void (*ui_func)(void), void (*bg_func)(void), int priority);
int validate_framebuffer(unsigned int address);
int init_graphics(unsigned int framebuffer_address);
int init_system_apps(void);
int system_health_check(void);
void kernel_shutdown(void);
void emergency_recovery(void);

// Kernel panic implementation with improved safety
void kernel_panic(const char* message) {
    // Disable interrupts immediately to prevent further issues
    asm volatile("cli");
    
    // Clear screen if framebuffer is available
    if (framebuffer && graphics_initialized) {
        // Use a simple loop to fill screen with panic color
        unsigned int total_pixels = screen_width * screen_height;
        for (unsigned int i = 0; i < total_pixels; i++) {
            framebuffer[i] = PANIC_BG_COLOR;
        }
        
        // Draw a white rectangle for visibility (simple implementation)
        unsigned int rect_start_x = screen_width / 8;
        unsigned int rect_start_y = screen_height / 4;
        unsigned int rect_width = screen_width * 3 / 4;
        unsigned int rect_height = screen_height / 8;
        
        for (unsigned int y = rect_start_y; y < rect_start_y + rect_height && y < screen_height; y++) {
            for (unsigned int x = rect_start_x; x < rect_start_x + rect_width && x < screen_width; x++) {
                if (y * screen_width + x < screen_width * screen_height) {
                    framebuffer[y * screen_width + x] = PANIC_TEXT_COLOR;
                }
            }
        }
    }
    
    // Halt the system
    while (1) {
        asm volatile("hlt");
    }
}

// Null loop implementations
void null_ui_loop(void) {
    // Intentionally empty - for background-only services
}

void null_background_loop(void) {
    // Intentionally empty - for UI-only applications
}

// Get screen dimensions with proper error handling
int get_screen_dimensions(unsigned int* width, unsigned int* height) {
    if (!width || !height) {
        return 0; // Invalid parameters
    }
    
    // TODO: Implement actual screen detection via graphics driver
    // This would typically query the graphics hardware or firmware
    
    // For now, attempt to detect 4K capability
    // In a real implementation, this would involve:
    // 1. Querying EDID data from monitor
    // 2. Checking graphics card capabilities
    // 3. Reading BIOS/UEFI graphics settings
    
    *width = TARGET_4K_WIDTH;
    *height = TARGET_4K_HEIGHT;
    
    return 1; // Success
}

// Safe app registration with comprehensive validation
int register_app_safe(const char* name, void (*ui_func)(void), void (*bg_func)(void), int priority) {
    // Validate name parameter
    if (!name) {
        return -1; // Null name pointer
    }
    
    // Check name length (prevent buffer overflows in app manager)
    int name_len = 0;
    const char* temp = name;
    while (*temp != '\0' && name_len < 256) { // Reasonable limit
        temp++;
        name_len++;
    }
    
    if (name_len == 0 || name_len >= 256) {
        return -2; // Invalid name length
    }
    
    // Validate function pointers
    if (!ui_func && !bg_func) {
        return -3; // App must have at least one function
    }
    
    // Validate priority range
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        return -4; // Priority out of range
    }
    
    // Ensure app manager is initialized before registering
    if (!apps_initialized) {
        return -5; // App manager not ready
    }
    
    // Use null functions for missing implementations
    void (*safe_ui_func)(void) = ui_func ? ui_func : null_ui_loop;
    void (*safe_bg_func)(void) = bg_func ? bg_func : null_background_loop;
    
    // Call the actual registration function
    return register_app(name, safe_ui_func, safe_bg_func, priority);
}

// Enhanced framebuffer validation
int validate_framebuffer(unsigned int address) {
    // Check for null address
    if (address == 0) {
        return 0;
    }
    
    // Check minimum address (avoid low memory regions)
    if (address < MIN_FRAMEBUFFER_ADDRESS) {
        return 0;
    }
    
    // Check maximum address (avoid wrap-around)
    if (address > MAX_FRAMEBUFFER_ADDRESS - (TARGET_4K_WIDTH * TARGET_4K_HEIGHT * 4)) {
        return 0; // Not enough space for 4K framebuffer
    }
    
    // Check alignment (framebuffers should be aligned)
    if (address % 4 != 0) {
        return 0; // Not 32-bit aligned
    }
    
    // TODO: In a real OS, perform additional checks:
    // - Verify address is in valid memory range
    // - Check if memory is mapped and accessible
    // - Verify memory is not reserved by other systems
    
    return 1; // Validation passed
}

// Initialize graphics system with comprehensive error checking
int init_graphics(unsigned int framebuffer_address) {
    // Reset initialization flag
    graphics_initialized = 0;
    
    // Validate framebuffer address
    if (!validate_framebuffer(framebuffer_address)) {
        return 0; // Invalid framebuffer address
    }
    
    // Set framebuffer pointer
    framebuffer = (volatile unsigned int *)framebuffer_address;
    
    // Get screen dimensions
    if (!get_screen_dimensions(&screen_width, &screen_height)) {
        // Fall back to default dimensions
        screen_width = DEFAULT_SCREEN_WIDTH;
        screen_height = DEFAULT_SCREEN_HEIGHT;
    }
    
    // Validate screen dimensions
    if (screen_width == 0 || screen_height == 0 || 
        screen_width > 7680 || screen_height > 4320) { // Max 8K resolution
        screen_width = DEFAULT_SCREEN_WIDTH;
        screen_height = DEFAULT_SCREEN_HEIGHT;
    }
    
    // Test framebuffer access by clearing screen
    unsigned int total_pixels = screen_width * screen_height;
    for (unsigned int i = 0; i < total_pixels; i++) {
        framebuffer[i] = 0x000000; // Black
    }
    
    // Draw test pattern with bounds checking
    unsigned int test_rect_width = screen_width / 3;
    unsigned int test_rect_height = screen_height / 6;
    unsigned int center_x = screen_width / 2;
    unsigned int center_y = screen_height / 2;
    
    // Ensure test rectangles are within bounds
    unsigned int rect_start_x = center_x - test_rect_width / 2;
    unsigned int rect_start_y = center_y - test_rect_height / 2;
    
    if (rect_start_x + test_rect_width <= screen_width &&
        rect_start_y + test_rect_height <= screen_height) {
        
        // Draw main test rectangle (orange)
        for (unsigned int y = rect_start_y; y < rect_start_y + test_rect_height; y++) {
            for (unsigned int x = rect_start_x; x < rect_start_x + test_rect_width; x++) {
                framebuffer[y * screen_width + x] = 0xFF5733;
            }
        }
        
        // Draw smaller overlay rectangle (green)
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
    return 1; // Success
}

// Initialize system applications with detailed error tracking
int init_system_apps(void) {
    int apps_registered = 0;
    int registration_errors = 0;
    
    // Initialize app manager first
    if (!init_apps()) {
        return 0; // App manager initialization failed
    }
    
    apps_initialized = 1;
    
    // Register Native UI Applications with error tracking
    int result = register_app_safe("Launcher", launcher_ui_loop, null_background_loop, 9);
    if (result >= 0) {
        apps_registered++;
    } else {
        registration_errors++;
    }
    
    result = register_app_safe("File Explorer", file_explorer_ui_loop, null_background_loop, 8);
    if (result >= 0) {
        apps_registered++;
    } else {
        registration_errors++;
    }
    
    result = register_app_safe("Settings", settings_ui_loop, null_background_loop, 7);
    if (result >= 0) {
        apps_registered++;
    } else {
        registration_errors++;
    }
    
    // Register Background Services
    result = register_app_safe("Filesystem", null_ui_loop, filesystem_background_loop, 10);
    if (result >= 0) {
        apps_registered++;
    } else {
        registration_errors++;
    }
    
    result = register_app_safe("Audio Manager", null_ui_loop, audio_manager_background_loop, 9);
    if (result >= 0) {
        apps_registered++;
    } else {
        registration_errors++;
    }
    
    // Check if minimum required apps were registered
    if (apps_registered < MIN_APPS_REQUIRED) {
        return 0; // Too few apps registered
    }
    
    return 1; // Success
}

// Enhanced system health check
int system_health_check(void) {
    // Check graphics system
    if (!graphics_initialized || !framebuffer) {
        return 0; // Graphics system failed
    }
    
    // Check if critical flags are still valid
    if (!drivers_initialized || !filesystem_initialized || !apps_initialized) {
        return 0; // Critical system component failed
    }
    
    // TODO: Add more comprehensive health checks:
    // - Memory leak detection
    // - Driver responsiveness
    // - Filesystem integrity
    // - Application status monitoring
    // - Interrupt handling verification
    
    return 1; // System appears healthy
}

// Main kernel entry point with staged initialization
void kernel_main(unsigned int framebuffer_address) {
    // Stage 1: Graphics Initialization
    if (!init_graphics(framebuffer_address)) {
        kernel_panic("Graphics initialization failed");
    }
    
    // Stage 2: Driver Initialization
    if (!init_drivers()) {
        drivers_initialized = 0;
        kernel_panic("Driver initialization failed");
    }
    drivers_initialized = 1;
    
    // Stage 3: Configuration Parsing (non-critical)
    if (!parse_config()) {
        // Configuration parsing failure is non-fatal
        // System will continue with default settings
        // TODO: Log this event when logging system is available
    }
    
    // Stage 4: Filesystem Initialization
    if (!init_filesystem()) {
        filesystem_initialized = 0;
        kernel_panic("Filesystem initialization failed");
    }
    filesystem_initialized = 1;
    
    // Stage 5: Directory Listing (verification, non-critical)
    if (!list_root_directory()) {
        // Directory listing failure is not fatal
        // Filesystem might be empty or have permission issues
    }
    
    // Stage 6: Application System Initialization
    if (!init_system_apps()) {
        kernel_panic("Application system initialization failed");
    }
    
    // Stage 7: Final system health check before starting scheduler
    if (!system_health_check()) {
        kernel_panic("System health check failed before scheduler start");
    }
    
    // Stage 8: Start the Multitasking Scheduler
    if (!run_scheduler()) {
        kernel_panic("Scheduler failed to start");
    }
    
    // If scheduler returns, something is seriously wrong
    kernel_panic("Scheduler returned unexpectedly");
}

// Graceful shutdown implementation
void kernel_shutdown(void) {
    // Disable interrupts during shutdown
    asm volatile("cli");
    
    // TODO: Implement proper shutdown sequence:
    // 1. Signal all applications to terminate
    // 2. Wait for applications to clean up
    // 3. Flush all filesystem buffers
    // 4. Shutdown drivers in reverse order
    // 5. Clear framebuffer
    // 6. Halt system
    
    if (framebuffer && graphics_initialized) {
        // Clear screen to shutdown color
        unsigned int total_pixels = screen_width * screen_height;
        for (unsigned int i = 0; i < total_pixels; i++) {
            framebuffer[i] = SHUTDOWN_BG_COLOR;
        }
    }
    
    // Reset system state
    graphics_initialized = 0;
    drivers_initialized = 0;
    filesystem_initialized = 0;
    apps_initialized = 0;
    
    // Halt system
    while (1) {
        asm volatile("hlt");
    }
}

// Emergency recovery implementation
void emergency_recovery(void) {
    // Display recovery screen
    if (framebuffer && graphics_initialized) {
        unsigned int total_pixels = screen_width * screen_height;
        for (unsigned int i = 0; i < total_pixels; i++) {
            framebuffer[i] = RECOVERY_BG_COLOR;
        }
    }
    
    // TODO: Implement recovery procedures:
    // 1. Attempt to restart failed applications
    // 2. Reset drivers if needed
    // 3. Perform memory cleanup
    // 4. Restart scheduler if necessary
    // 5. Log recovery events
    
    // For now, perform basic system health check
    if (!system_health_check()) {
        // If health check fails during recovery, panic
        kernel_panic("Emergency recovery failed");
    }
}