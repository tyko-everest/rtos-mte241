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
	//test_context_switch();
	//test_fpp();
	//test_semaphores();
	//test_semaphores_2();
	
	//test_mutex_ownership();	
	test_prio_inher();
}


