#ifdef TEST_QUEUE

#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"

#include "rtos.h"
#include "scheduler.h"

extern task_list_t *running;
extern uint32_t ready_mask;
extern task_list_t ready[NUM_PRIORITIES];

void t1(void *arg) {
	while (1);
}

void t2(void *arg) {
	while (1);
}

void test_queues(void) {
	
	os_kernel_init();
	
	highest_priority_list(running, ready_mask);
	os_add_task(t1, NULL, NULL);
	os_add_task(t2, NULL, NULL);
	os_add_task(t2, NULL, NULL);
	
	dequeue(ready + DEF_PRIORITY, &ready_mask);
	dequeue(ready + DEF_PRIORITY, &ready_mask);
	dequeue(ready + DEF_PRIORITY, &ready_mask);
	dequeue(ready + DEF_PRIORITY, &ready_mask);

	os_kernel_start();
}

#endif
