#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"

#include "rtos.h"
#include "scheduler.h"
extern task_list_t *running;

int main(void) {

	
	printf("\nStarting...\n\n");
	
	os_kernel_init();
	
	highest_priority_list(running);
	
	os_kernel_start();
}
