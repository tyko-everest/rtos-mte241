#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rtos.h"


void t1(void *arg) {
	while (1) {
		for(int i = 0; i < 1000000; i++);
		printf("task1\n");
	}
}

void t2(void *arg) {
	while(1) {
	  for(int i = 0; i < 1000000; i++);
		printf("task2\n");
	}
}

int main(void) {

	
	printf("\nStarting...\n\n");
	
	os_kernel_init();
	os_add_task(t1, NULL, NULL);
	os_add_task(t2, NULL, NULL);
	
	curr_sp = (uint32_t) tcb_list[2].sp;
	next_sp = (uint32_t) tcb_list[1].sp;
	
	os_kernel_start();
	
}
