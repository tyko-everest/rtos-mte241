#include "blocking.h"

extern task_list_t *running;

os_semaphore_t sem_list[MAX_SEMAPHORES];

os_error_t os_new_semaphore(os_semaphore_t *sem, uint32_t init_count) {
	
	// all static memory so there is a fixed number of semaphores allowed
	static uint32_t num_sems = 0;
	if (num_sems >= MAX_SEMAPHORES) {
		return OS_ERR_MEM;
	}
	
	// set the pointer to the right entry in the allocated space
	sem = sem_list + num_sems;
	
	sem->count = init_count;	
	// set the blocked lists to be empty
	for (int i = 0; i < NUM_PRIORITIES; i++) {
		sem->blocked[i].head = NULL;
		sem->blocked[i].tail = NULL;
	}
	
	return OS_OK;
}

void os_wait(os_semaphore_t *sem) {
	// check if we need to block first
	if (sem->count == 0) {
		enqueue(running->head, &(sem->blocked[running->head->priority]));
		//os_schedule();
	}
	__disable_irq();
	sem->count--;
	__enable_irq();
	return;
}

void os_signal(os_semaphore_t *sem) {
	__disable_irq();
	sem->count++;
	__enable_irq();
	//os_schedule();
}

