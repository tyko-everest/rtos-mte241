#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rtos.h"

int main(void) {

	
	printf("\nStarting...\n\n");
	
	os_kernel_init();
	os_add_task(t1, NULL, NULL);
	os_add_task(t2, NULL, NULL);
	
	os_kernel_start();
	highest_priority_list(NULL);
}
