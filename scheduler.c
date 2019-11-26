#include "scheduler.h"

extern task_list_t running;
extern task_list_t ready[NUM_PRIORITIES];
extern uint32_t ready_mask;
extern uint32_t **curr_sp;
extern uint32_t **next_sp;

bool running_handled = false;
uint32_t ms_ticks = 0;

void os_schedule() {
	disable_irq();
	
	// dequeue running task from running list
	tcb_t* curr_task = dequeue(&running, NULL);
	
	// If the running task wasn't blocked, it will be placed in the correct list
	if (!running_handled) {
		// Find task_list to add prev_running_task to
		task_list_t* curr_task_list = ready + curr_task->priority;
		// enqueue curr_task task to it's priority level list
		enqueue(curr_task, curr_task_list, &ready_mask);
	}
	running_handled = false;
	
	// Find task_list to remove next_task from
	task_list_t* next_task_list = highest_priority_list(ready, ready_mask);
	// dequeue next_task
	tcb_t* next_task = dequeue(next_task_list, &ready_mask);
	// enqueue next_task into running queue
	enqueue(next_task, &running, NULL);
	
	// manipulate curr_sp and next_sp to be pointers
	// to the corresponding tcb's stack pointer
	curr_sp = &(curr_task->sp);
	next_sp = &(next_task->sp);
	
	if (curr_sp != next_sp) {
		// set a pending context switch
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}
	
	enable_irq();
}

void os_yield(void) {
	os_schedule();
}

void SysTick_Handler(void) {
	ms_ticks++;
	if (ms_ticks % TIMESLICE_MS == 0) {
		os_schedule();
	}
}

uint32_t get_ticks(void) {
	return ms_ticks;
}
