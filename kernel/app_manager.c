#include "app_manager.h"

// Maximum number of apps supported (native + third-party)
#define MAX_APPS 10

// Global app list
App apps[MAX_APPS];
int app_count = 0;

// Initialize app registry
void init_apps() {
    app_count = 0;
}

// Register an app into the system
void register_app( const char *name, void (*ui_loop)(), void (*background_loop)(), int is_system_app) {
    if (app_count >= MAX_APPS)
        return;

    apps[app_count].id = app_count;

    // Simple string copy
    int i = 0;
    while (name[i] != '\0' && i < 31) {
        apps[app_count].name[i] = name[i];
        i++;
    }
    apps[app_count].name[i] = '\0'; // Null terminate

    // First app starts as active, others start as paused
    apps[app_count].state = (app_count == 0) ? TASK_UI_ACTIVE : TASK_UI_PAUSED;

    apps[app_count].ui_loop = ui_loop;
    apps[app_count].background_loop = background_loop;
    apps[app_count].is_system_app = is_system_app;

    app_count++;
}

// Switch active app by ID
void switch_app(int new_app_id) {
    for (int i = 0; i < app_count; i++) {
        if (i == new_app_id) {
            apps[i].state = TASK_UI_ACTIVE;
        } else if (apps[i].state == TASK_UI_ACTIVE) {
            apps[i].state = TASK_UI_PAUSED;
        }
        // Background services are not affected
    }
}

// Main app/task scheduler
void run_scheduler() {
    while (1) {
        for (int i = 0; i < app_count; i++) {
            // Run UI loop for active app
            if (apps[i].state == TASK_UI_ACTIVE) {
                apps[i].ui_loop();
            }
            // Run background service (if provided)
            else if (apps[i].state == TASK_BACKGROUND) {
                apps[i].background_loop();
            }
        }
    }
}
