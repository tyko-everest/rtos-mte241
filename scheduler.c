#include "scheduler.h"

extern task_list_t running;
extern task_list_t ready[NUM_PRIORITIES];
extern uint32_t ready_mask;
extern uint32_t curr_sp;
extern uint32_t next_sp;

void os_schedule(bool blocked) {
	
	//__disable_irq();
	
	// dequeue running task from running list
	tcb_t* prev_task = dequeue(&running, NULL);
	
	// If the running task wasn't blocked, it will be placed in the correct list
	if (!blocked) {
		// Find task_list to add prev_running_task to
		task_list_t* prev_task_list = ready + prev_task->priority;
		// enqueue prev_task task to it's priority level list
		enqueue(prev_task, prev_task_list, &ready_mask);
	}
	
	// Find task_list to remove next_task from
	task_list_t* next_task_list = highest_priority_list(ready, ready_mask);
	// dequeue next_task
	tcb_t* next_task = dequeue(next_task_list, &ready_mask);
	// enqueue next_task into running queue
	enqueue(next_task, &running, NULL);
	
	// manipulate curr_sp and next_sp
	curr_sp = (uint32_t)prev_task->sp;
	next_sp = (uint32_t)next_task->sp;
	
	if (curr_sp != next_sp) {
		// set a pending context switch
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}
	
	//__enable_irq();
}

void SysTick_Handler(void) {
	__disable_irq();
	os_schedule(false);
	__enable_irq();
}
