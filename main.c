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

int main(void) {

	
	printf("\nStarting...\n\n");
	
	os_kernel_init();
	
	highest_priority_list(running, ready_mask);

	os_kernel_start();
}

