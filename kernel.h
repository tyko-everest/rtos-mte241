#ifndef INCLUDE_KERNEL_H
#define INCLUDE_KERNEL_H

#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rtos.h"
#include "scheduler.h"

// used to initialize all kernel data structures
// must be first os function called
void os_kernel_init(void);

/** os_add_task
 * Adds a new task to be run when the os is started
 * @param fun_pointer Pointer to the function that is the task
 * @param args Void pointer to the function's args, use NULL if no args are needed
 */
os_error_t os_add_task(os_task_func_t func_pointer, void *args, os_task_attribs_t *attribs);

/** os_kernel_start
 * Last os function to call, started the scheduler
 * Never returns
 */
void os_kernel_start(void);

/** os_idle_task
 * Idle task, used be scheduler when nothing else is running
 * Do not call this function manually as it will never return
 */
void os_idle_task(void *arg);

#endif /* INCLUDE_KERNEL_H */
