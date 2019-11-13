#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"

int main(void) {
	printf("\nStarting RTOS\n\n");
	
	os_kernel_init();
	
	os_kernel_start();
}

