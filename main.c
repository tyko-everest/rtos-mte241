#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "blocking.h"
#include "test_queue.h"

#include "rtos.h"

// for testing
extern uint32_t prev_sp, curr_sp;
extern task_list_t running;
extern task_list_t ready[NUM_PRIORITIES];
extern uint32_t ready_mask;

void delay() {
	for (int i = 0; i < 1000000; i++);
}

os_semaphore_id_t highest_blocker, medium_blocker;
os_mutex_id_t mutex;

void t1(void *arg) {
	
	delay();
	
	// change its own priority
	__disable_irq();
	running.head->priority = 1;
	__enable_irq();
	// release medium task
	os_signal(medium_blocker);
	
	delay();
	
	// change its own priority
	__disable_irq();
	running.head->priority = 0;
	__enable_irq();
	// release highest task
	os_signal(highest_blocker);
	
	// change its own priority
	// so it never runs again
	__disable_irq();
	running.head->priority = 3;
	__enable_irq();
	
	while (1);
}

void t2(void *arg) {
	os_wait(highest_blocker);
	printf("Highest Released\n");
	while (1) {
		os_acquire(mutex);
		printf("highest task ran\n");
		
		
	}
}

void t3(void *arg) {
	os_wait(medium_blocker);
	printf("Medium Released\n");
	while (1) {
		printf("medium task ran\n");
		delay();
	}
}

void t4(void *arg) {
	printf("Lowest Released\n");
	while (1) {
		os_acquire(mutex);
		printf("lowest task ran\n");
		delay();
		os_release(mutex);
	}
}

uint32_t a = 0, b = 0, c = 0;

os_semaphore_id_t sem;
os_mutex_id_t mut;

void t5(void *arg) {
	while (1) {
		//for (int i = 0; i < 20; i++);
		os_acquire(mutex);
		for (int i = 0; i < 10; i++) {
			printf("Task 1\n");
		}
//		a++;
//		c++;
		//for (int i = 0; i < 40; i++);
		os_release(mutex);
	}
}

void t6(void *arg) {
	while (1) {
		//for (int i = 0; i < 20; i++);
		os_acquire(mutex);
		for (int i = 0; i < 10; i++) {
			printf("Task 2\n");
		}

//		b++;
//		c++;
		//for (int i = 0; i < 40; i++);
		os_release(mutex);
	}
}

int main(void) {
	printf("\nStarting RTOS\n\n");
	
	os_kernel_init();
	
	// task used to start higher priority tasks at later times
	os_task_attribs_t t1_attribs = {2};
	// highest priority task that relies on mutex shared with lower prio task
	os_task_attribs_t t2_attribs = {0};
	// medium priority task that takes a long time
	os_task_attribs_t t3_attribs = {1};
	// lowest priority task that shares the mutex with highest prio task
	os_task_attribs_t t4_attribs = {2};
	
//	os_add_task(t1, NULL, &t1_attribs);	
//	os_add_task(t2, NULL, &t2_attribs);
//	os_add_task(t3, NULL, &t3_attribs);
//	os_add_task(t4, NULL, &t4_attribs);
	
	os_add_task(t5, NULL, NULL);
	os_add_task(t6, NULL, NULL);
//	os_error_t state = os_new_mutex(&mut, 0);
	os_error_t state = os_new_semaphore(&sem, 1);

//	os_new_semaphore(&highest_blocker, 0);
//	os_new_semaphore(&medium_blocker, 0);
 	os_new_mutex(&mutex, MUTEX_MODE_INHER | MUTEX_MODE_OWNER);
	
	os_kernel_start();
}

