#include "blocking.h"

extern task_list_t *running;
extern task_list_t ready[MAX_NUM_TASKS];
extern uint32_t ready_mask;

os_semaphore_t sem_list[MAX_SEMAPHORES];

os_error_t os_new_semaphore(os_semaphore_id_t *sem_id, uint32_t init_count) {
	
	// all static memory so there is a fixed number of semaphores allowed
	static uint32_t num_sems = 0;
	if (num_sems >= MAX_SEMAPHORES) {
		return OS_ERR_MEM;
	}
	
	// set the id to the right index in the semaphore array
	*sem_id = num_sems;
	
	sem_list[num_sems].count = init_count;	
	// set the blocked lists to be empty
	for (int i = 0; i < NUM_PRIORITIES; i++) {
		sem_list[num_sems].blocked[i].head = NULL;
		sem_list[num_sems].blocked[i].tail = NULL;
	}
	// lists start empty
	sem_list[num_sems].blocked_mask = 0;
	
	num_sems++;
	return OS_OK;
}

void os_wait(os_semaphore_id_t sem_id) {
	__disable_irq();
	// check if we need to block first
	if (sem_list[sem_id].count == 0) {
		enqueue(running->head, sem_list[sem_id].blocked + running->head->priority, &sem_list[sem_id].blocked_mask);
		os_schedule(true);
	}
	__disable_irq();
	sem_list[sem_id].count--;
	__enable_irq();
}

void os_signal(os_semaphore_id_t sem_id) {
	__disable_irq();
	sem_list[sem_id].count++;
	// get the highest priority blocked task list
	task_list_t *list = highest_priority_list(sem_list[sem_id].blocked, sem_list[sem_id].blocked_mask);
	// check if there are any tasks currently blocked on it
	// if list is NULL there is nothing blocked
	if (list != NULL) {
		// add its head to the appropriate ready list
		tcb_t *task = dequeue(list, &sem_list[sem_id].blocked_mask);
		enqueue(task, ready + task->priority, &ready_mask);
		// see if it's priority is higher than the running task's priority
		if (task->priority > running->head->priority) {
			os_schedule(false);
		}
	}
	__enable_irq();
}

