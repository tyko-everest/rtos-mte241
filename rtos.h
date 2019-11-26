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
	OS_ERR_ARG,
	OS_ERR_PERM
} os_error_t;

// tasks must be of this type
typedef void (*os_task_func_t) (void *args);

// task list manipulation functions
void enqueue(tcb_t *task, task_list_t *list, uint32_t *mask);
tcb_t * dequeue(task_list_t *list, uint32_t *mask);

// given an array of task lists, it returns a pointer to
// the highest priority non-empty list
task_list_t* highest_priority_list(task_list_t* list, uint32_t priority_mask);

void print_list_contents(task_list_t *list);

// use these to make nested interrupts disabling work
void disable_irq(void);
void enable_irq(void);


#endif /* INCLUDE_RTOS_H */
