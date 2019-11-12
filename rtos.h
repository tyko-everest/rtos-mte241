#ifndef INCLUDE_RTOS_H
#define INCLUDE_RTOS_H

#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_NUM_TASKS 6
#define NUM_REGS 16
#define PCR_DEF_VAL 0x01000000

#define NUM_PRIORITIES 32
#define HIGHEST_PRIORITY 0
#define LOWEST_PRIORITY (NUM_PRIORITIES - 1)
#define DEF_PRIORITY 15

#define MAIN_STACK_BASE_ADDR (*((volatile uint32_t*) 0))
#define TASK_STACK_BASE_ADDR(x) ((MAIN_STACK_BASE_ADDR - 0x800 - (0x400 * x)))

void t1(void *arg);
void t2(void *arg);

// used to store all needed information about a task
struct _tcb {
	uint32_t *sp;
	uint32_t id, priority;
	struct _tcb *next;
};
typedef struct _tcb tcb_t;

// used to make linked lists of tcbs
typedef struct {
    tcb_t *head;
    tcb_t *tail;
} task_list_t;

// used for setting up a task
typedef struct {
	uint32_t priority;
} os_task_attribs_t;

// error codes returned by os functions
typedef enum _os_error {
	OS_OK = 0,
	OS_ERR_MEM,
	OS_ERR_ARG
} os_error_t;

// tasks must be of this type
typedef void (*os_task_func_t) (void *args);

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

// called to figure out which task should run next
void os_schedule(void);

// task list manipulation functions
void add_tail_task(tcb_t *task, task_list_t *list);
tcb_t * remove_head_task(task_list_t *list);
tcb_t * remove_tail_task(task_list_t *list);

/** os_idle_task
 * Idle task, used be scheduler when nothing else is running
 * Do not call this function manually as it will never return
 */
void os_idle_task(void *arg);


#endif /* INCLUDE_RTOS_H */
