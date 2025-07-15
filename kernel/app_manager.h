#ifndef HASHOS_APP_MANAGER_H
#define HASHOS_APP_MANAGER_H

typedef enum {
    TASK_UI_ACTIVE,
    TASK_UI_PAUSED,
    TASK_BACKGROUND
} TaskState;

typedef struct {
    int id;
    char name[32];
    TaskState state;
    void (*ui_loop)();
    void (*background_loop)();
    int is_system_app; // 1 = Native app, 0 = Third-party
} App;

void init_apps();
void null_background_loop();
void null_ui_loop();
void register_app(const char *name, void (*ui_loop)(), void (*background_loop)(), int is_system_app);
void switch_app(int new_app_id);
void run_scheduler();

extern int app_count;
extern App apps[];

#endif
