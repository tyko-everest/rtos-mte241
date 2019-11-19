#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "blocking.h"
#include "test_queue.h"

#include "rtos.h"

extern uint32_t prev_sp, curr_sp;
extern task_list_t ready[NUM_PRIORITIES];
extern uint32_t ready_mask;

os_semaphore_id_t sem;
os_mutex_id_t mutex;

void delay() {
	for (int i = 0; i < 100; i++);
}

uint32_t a = 0, b = 0, c = 0;

void t1(void *arg) {
	while (1) {
		os_acquire(mutex);
		a++;
		c++;
		os_release(mutex);
		
		delay();
	}
}

void t2(void *arg) {
	while (1) {
		os_acquire(mutex);
		b++;
		c++;
		os_release(mutex);
	
		delay();
	}
}

void t3(void *arg) {
	while (1) {
		printf("task\n");
		delay();
		
	}
}

int main(void) {
	printf("\nStarting RTOS\n\n");
	
	os_kernel_init();
	
	//os_add_task(t1, NULL, NULL);
	//os_add_task(t2, NULL, NULL);
	//print_list_contents(ready);
	//os_add_task(t2, NULL, NULL);
//	print_list_contents(ready);

	//dequeue(ready + DEF_PRIORITY, &ready_mask);
	//print_list_contents(ready);

	//dequeue(ready + DEF_PRIORITY, &ready_mask);
	//print_list_contents(ready);

	//dequeue(ready + DEF_PRIORITY, &ready_mask);
	//print_list_contents(ready);

	//dequeue(ready + DEF_PRIORITY, &ready_mask);
	
	os_task_attribs_t t1_attribs = {1};
	os_task_attribs_t t2_attribs = {1};
	os_task_attribs_t t3_attribs = {1};
	
	os_add_task(t1, NULL, &t1_attribs);	
	//print_list_contents(ready);

	os_add_task(t2, NULL, &t2_attribs);
	//print_list_contents(ready);

	//os_add_task(t3, NULL, &t3_attribs);
	//print_list_contents(ready);

	//os_new_semaphore(&sem, 1);
	os_new_mutex(&mutex);
	
	//test_queues();
	os_kernel_start();
}

