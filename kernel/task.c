#include "task.h"

#define MAX_TASKS 4
#define STACK_SIZE 1024

Task tasks[MAX_TASKS];
int current_task = -1;
int task_count = 0;

unsigned int stacks[MAX_TASKS][STACK_SIZE];

void init_tasks() {
    task_count = 0;
    current_task = -1;
}

void create_task(void (*task_entry)()) {
    if (task_count >= MAX_TASKS)
        return;

    tasks[task_count].id = task_count;
    tasks[task_count].state = TASK_READY;
    tasks[task_count].task_entry = task_entry;

    // Set stack pointer to top of allocated stack
    tasks[task_count].stack_pointer = &stacks[task_count][STACK_SIZE - 1];

    task_count++;
}

void schedule() {
    while (1) {
        current_task = (current_task + 1) % task_count;

        if (tasks[current_task].state == TASK_FINISHED)
            continue;

        tasks[current_task].state = TASK_RUNNING;
        tasks[current_task].task_entry();
        tasks[current_task].state = TASK_FINISHED;
    }
}

void yield() {
    // In real preemptive system: save stack, do context switch
    // For now: just return to schedule to switch tasks
}
