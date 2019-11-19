#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "blocking.h"
#include "test_suite.h"

#include "rtos.h"


int main(void) {
	printf("\nStarting RTOS\n\n");
//	if(test_queues())
//		printf("success");
//	else
//		printf("fail");
	
	test_semaphores();
}

