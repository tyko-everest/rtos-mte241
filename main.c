#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "blocking.h"

extern uint32_t prev_sp, curr_sp;

os_semaphore_id_t sem;

void delay() {
	for (int i = 0; i < 100000; i++);
}

void t1(void *arg) {
	while (1) {
		os_wait(sem);
		printf("task1\n");
		os_signal(sem);
		
		delay();
		
	}
}

void t2(void *arg) {
	while (1) {
		os_wait(sem);
		printf("task2\n");
		os_signal(sem);
		
		delay();
		
	}
}

void t3(void *arg) {
	while (1) {
		//os_wait(sem);
		printf("task3\n");
		//os_signal(sem);
		
		delay();
		
	}
}

int main(void) {
	printf("\nStarting RTOS\n\n");
	
	os_kernel_init();
	
	os_task_attribs_t t1_attribs = {1};
	os_task_attribs_t t2_attribs = {1};
	os_task_attribs_t t3_attribs = {1};
	
	os_add_task(t1, NULL, &t1_attribs);
	os_add_task(t2, NULL, &t2_attribs);
	//os_add_task(t3, NULL, &t3_attribs);
	
	os_new_semaphore(&sem, 1);
	
	os_kernel_start();
}

