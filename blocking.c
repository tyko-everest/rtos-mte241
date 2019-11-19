#include "blocking.h"

extern task_list_t running;
extern task_list_t ready[MAX_NUM_TASKS];
extern uint32_t ready_mask;

os_semaphore_t sem_list[MAX_SEMAPHORES];

os_error_t os_new_semaphore(os_semaphore_id_t *sem_id, uint32_t init_count) {
	// used to keep track of number of semaphores and mutexes
	static uint32_t num_sems = 0;
	
	// all static memory so there is a fixed number of semaphores allowed
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
	disable_irq();
	// check if we need to block first
	if (sem_list[sem_id].count == 0) {
		enqueue(running.head, sem_list[sem_id].blocked + running.head->priority, &sem_list[sem_id].blocked_mask);
		os_schedule(true);
	}
	enable_irq();
	// this is needed so interrupt can trigger?
	__asm {
		nop
	}
	enable_irq();
	sem_list[sem_id].count--;
	disable_irq();
}

void os_signal(os_semaphore_id_t sem_id) {
	disable_irq();
	sem_list[sem_id].count++;
	// get the highest priority blocked task list
	task_list_t *list = highest_priority_list(sem_list[sem_id].blocked,
			sem_list[sem_id].blocked_mask);
	// check if there are any tasks currently blocked on it
	// if list is NULL there is nothing blocked
	if (list != NULL) {
		// add its head to the appropriate ready list
		tcb_t *task = dequeue(list, &sem_list[sem_id].blocked_mask);
		enqueue(task, ready + task->priority, &ready_mask);
		// see if it's priority is higher than the running task's priority
		if (task->priority > running.head->priority) {
			os_schedule(false);
		}
	}
	enable_irq();
}

os_error_t os_new_mutex(os_semaphore_id_t *mutex_id) {
	os_error_t status = os_new_semaphore(mutex_id, 1);
	if (status != OS_OK) {
		return status;
	}
	// update the mutex specific elements of the struct
	sem_list[*mutex_id].current_owner = NULL;
	sem_list[*mutex_id].owner_orig_prio = LOWEST_PRIORITY;
	return OS_OK;
}

void os_acquire(os_mutex_id_t mutex_id) {
	disable_irq();
	// check if we need to block first
	if (sem_list[mutex_id].count == 0) {
		// add this task to correct blocked list
		enqueue(running.head, sem_list[mutex_id].blocked + running.head->priority,
				&sem_list[mutex_id].blocked_mask);
		// see if the current mutex owner needs to acquire higher prio
		if (running.head->priority < sem_list[mutex_id].current_owner->priority) {
			sem_list[mutex_id].current_owner->priority = running.head->priority;
		}
		// let scheduler know it should not add this task to ready list
		os_schedule(true);
	}
	enable_irq();
	
	// this is needed so interrupt can trigger?
	__asm {
		nop
	}
	
	disable_irq();
	sem_list[mutex_id].count--;
	// we have now been unblocked/never blocked, so need to change ownership
	sem_list[mutex_id].current_owner = running.head;
	sem_list[mutex_id].owner_orig_prio = running.head->priority;
	enable_irq();
}

void os_release(os_mutex_id_t mutex_id) {
	disable_irq();
	// check if any task currently owns the mutex, and then if they are the 
	// same as the owner
	if (sem_list[mutex_id].current_owner != NULL &&
			running.head->id == sem_list[mutex_id].current_owner->id) {
					
		sem_list[mutex_id].count++;
					
		// get the highest priority blocked task list
		task_list_t *list = highest_priority_list(sem_list[mutex_id].blocked,
				sem_list[mutex_id].blocked_mask);
		
		// set its prio to the highest blocked task on this mutex
		// or its original prio, whatever prio is highest
		if (list != NULL && list->head->priority < running.head->priority) {
			running.head->priority = list->head->priority;
		} else {
			running.head->priority = sem_list[mutex_id].owner_orig_prio;
		}
					
		// check if there are any tasks currently blocked on it
		// if list is NULL there is nothing blocked
		if (list != NULL) {
			// add its head to the appropriate ready list
			tcb_t *task = dequeue(list, &sem_list[mutex_id].blocked_mask);
			enqueue(task, ready + task->priority, &ready_mask);
			
			// see if it's priority is higher than the running task's priority
			if (task->priority < running.head->priority) {
				// block because a higher priority task can now run
				os_schedule(false);
			}
		}
	}
	enable_irq();
}

