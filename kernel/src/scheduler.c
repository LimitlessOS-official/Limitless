#include <scheduler.h>
#include <string.h>
#include "hal/hal_kernel.h"
#include <mm/mm.h>
#include "debug.h"
#include "mm/mm.h"

// --- Scheduler Internals ---

// Array of ready queues, one for each priority level
static task_t* ready_queues[NUM_PRIORITY_LEVELS];

// The currently running task
static task_t* current_task;

// A simple task ID counter
static int next_task_id = 1;

// The idle task, runs when no other task is ready
static task_t* idle_task;

// Assembly function for context switching
extern void switch_context(cpu_state_t* old, cpu_state_t* new);

// --- Idle Task ---

void idle_task_entry() {
    while (1) {
        terminal_writestring("\n[IDLE] Halting CPU.");
        asm volatile("hlt");
        for (volatile int i = 0; i < 10000000; i++); // Delay
    }
}

// --- Scheduler Implementation ---

int scheduler_init(void) {
    // 1. Initialize the ready queues
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        ready_queues[i] = NULL;
    }

    // 2. Create the idle task
    idle_task = create_task(idle_task_entry);
    idle_task->priority = -1; // Lowest possible priority
    current_task = idle_task;

    // 3. Set up the timer for preemption (e.g., every 10ms)
    // This requires a HAL function to register an interrupt handler.
    // Assuming hal_timer_register_handler exists.
    // hal_timer_register_handler(schedule, 10);
}

task_t* create_task(void (*entry)(void)) {
    // Allocate memory for the task struct and its stack
    task_t* task = (task_t*)kmalloc(sizeof(task_t));
    if (!task) return NULL;

    void* stack = kmalloc(PAGE_SIZE);
    if (!stack) {
        kfree(task);
        return NULL;
    }

    // Initialize the task struct
    memset(task, 0, sizeof(task_t));
    task->id = next_task_id++;
    task->stack = stack;
    task->state = TASK_READY;
    task->priority = 0; // Highest priority by default
    task->quantum = 10; // Default time slice

    // Initialize the CPU context for the new task.
    // The stack needs to be set up to look like it was in the middle
    // of a call to switch_context.
    uint32_t* stack_ptr = (uint32_t*)((uintptr_t)stack + PAGE_SIZE);

    // When the new task is switched to, `ret` will be called.
    // This will pop the entry point address into EIP.
    *--stack_ptr = (uint32_t)entry;

    // The `popl %ebp` in switch_context needs a value to pop.
    *--stack_ptr = 0; // Dummy EBP

    // The `popl` instructions for callee-saved registers.
    *--stack_ptr = 0; // EDI
    *--stack_ptr = 0; // ESI
    *--stack_ptr = 0; // EBX

    // Save the stack pointer in the task's context. This is what
    // `switch_context` will load into ESP.
    task->context.esp = (uint32_t)stack_ptr;

    // Add the task to the highest priority ready queue
    task->next = ready_queues[0];
    ready_queues[0] = task;

    return task;
}

void schedule() {
    terminal_writestring("\n--- schedule() called ---");

    // 1. Find the highest-priority ready task
    task_t* next_task = NULL;
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        if (ready_queues[i]) {
            next_task = ready_queues[i];
            ready_queues[i] = next_task->next; // Dequeue
            terminal_writestring("\n[SCHED] Found next task with ID: ");
            print_dec(next_task->id);
            break;
        }
    }

    // If no ready tasks, run the idle task
    if (!next_task) {
        next_task = idle_task;
        terminal_writestring("\n[SCHED] No ready tasks. Selecting idle task.");
    }

    // 2. Handle the previously running task (if it's not the idle task)
    if (current_task && current_task != idle_task) {
        // Re-queue the current task
        current_task->state = TASK_READY;
        current_task->next = ready_queues[current_task->priority];
        ready_queues[current_task->priority] = current_task;
        terminal_writestring("\n[SCHED] Re-queuing previous task ID: ");
        print_dec(current_task->id);
    }
    
    // 3. Switch to the new task
    if (next_task != current_task) {
        task_t* old_task = current_task;
        current_task = next_task;
        current_task->state = TASK_RUNNING;
        
        terminal_writestring("\n[SCHED] Switching context from task ");
        if(old_task) print_dec(old_task->id); else terminal_writestring("NULL");
        terminal_writestring(" to task ");
        if(current_task) print_dec(current_task->id); else terminal_writestring("NULL");
        terminal_writestring("\n--- End of schedule() ---");

        switch_context(&old_task->context, &current_task->context);
    } else {
        terminal_writestring("\n[SCHED] No task switch needed.");
        terminal_writestring("\n--- End of schedule() ---");
    }
}

void switch_to_task(task_t* task) {
    if (!task || task == current_task) return;

    task_t* old_task = current_task;
    current_task = task;

    old_task->state = TASK_READY;
    current_task->state = TASK_RUNNING;

    switch_context(&old_task->context, &current_task->context);
}
