#include "scheduler.h"

extern tcb_t *tcb_list;
extern uint32_t ready_mask;
	
void os_schedule(void) {
	// find next task
	
}

task_list_t* highest_priority_list(task_list_t* list) {
	
	if (list == NULL) {
		return NULL;
	}
	
	uint32_t leading_zeroes = 0;
	
	__asm {
		CLZ leading_zeroes, ready_mask
	}
	//while(1);
}
