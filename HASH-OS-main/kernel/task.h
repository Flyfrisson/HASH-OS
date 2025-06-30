#ifndef HASHOS_TASK_H
#define HASHOS_TASK_H

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_FINISHED
} TaskState;

typedef struct {
    int id;
    unsigned int *stack_pointer;
    TaskState state;
    void (*task_entry)();
} Task;

void init_tasks();
void create_task(void (*task_entry)());
void schedule();
void yield();

#endif
