#include "blocking.h"

extern task_list_t running;
extern task_list_t ready[MAX_NUM_TASKS];
extern uint32_t ready_mask;
extern bool running_handled;

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
	while (sem_list[sem_id].count == 0) {
		enqueue(running.head, sem_list[sem_id].blocked + running.head->priority,
				&sem_list[sem_id].blocked_mask);
		// running task has been put somewhere, so scheduler shouldn't also add
		// to ready
		running_handled = true;
		os_schedule();
		enable_irq();
		// interrupt will trigger here if a context switch is needed
		disable_irq();
	}
	sem_list[sem_id].count--;
	enable_irq();
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
		// only call the scheduler if the unblocked task is higher prio
		if (running.head->priority > task->priority) {
			os_schedule();
		}

	}
	enable_irq();
}

os_error_t os_new_mutex(os_semaphore_id_t *mutex_id, os_mutex_attribs_t attribs) {
	os_error_t status = os_new_semaphore(mutex_id, 1);
	if (status != OS_OK) {
		return status;
	}
	// update the mutex specific elements of the struct
	sem_list[*mutex_id].current_owner = NULL;
	sem_list[*mutex_id].owner_orig_prio = LOWEST_PRIORITY;
	sem_list[*mutex_id].mode = attribs;
	return OS_OK;
}

void os_acquire(os_mutex_id_t mutex_id) {
	disable_irq();
	
	// if you already have it acquired, you good, keep it up man!
	if (sem_list[mutex_id].mode & MUTEX_MODE_OWNER &&
			sem_list[mutex_id].current_owner != NULL &&
			sem_list[mutex_id].current_owner->id == running.head->id) {
		enable_irq();
		return;
	}
	
	// check if we need to block first
	while (sem_list[mutex_id].count == 0) {
		// add this task to correct blocked list
		enqueue(running.head, sem_list[mutex_id].blocked + running.head->priority,
				&sem_list[mutex_id].blocked_mask);
		 
		// if prio inheritance is enabled
		// see if the current mutex owner needs to acquire higher prio
		if (sem_list[mutex_id].mode & MUTEX_MODE_INHER &&
				running.head->priority < sem_list[mutex_id].current_owner->priority) {
			
			// temporarily change its priority
			sem_list[mutex_id].current_owner->priority = running.head->priority;
			// add in lower priority owner to ready list again, but at new prio
			enqueue(sem_list[mutex_id].current_owner,
					ready + running.head->priority, &ready_mask);
		}

		// let scheduler know it should not add this task to ready list
		running_handled = true;
		os_schedule();
		enable_irq();
		// context switch fill fire now
		disable_irq();
	}
	sem_list[mutex_id].count--;
	// we have now been unblocked/never blocked, so need to change ownership
	sem_list[mutex_id].current_owner = running.head;
	sem_list[mutex_id].owner_orig_prio = running.head->priority;
	enable_irq();
}

os_error_t os_release(os_mutex_id_t mutex_id) {
	// if ownership is enabled, fail and return if this is not the owner
	disable_irq();
	if (sem_list[mutex_id].mode & MUTEX_MODE_OWNER &&
			sem_list[mutex_id].current_owner != NULL &&
			running.head->id != sem_list[mutex_id].current_owner->id) {
		enable_irq();
		return OS_ERR_PERM;
	}
		
	// successfully released
	sem_list[mutex_id].count++;
	
	// reset owner to NULL if ownership is enabled
	if (sem_list[mutex_id].mode & MUTEX_MODE_OWNER) {
		sem_list[mutex_id].current_owner = NULL;
	}
				
	// get the highest priority blocked task list
	task_list_t *list = highest_priority_list(sem_list[mutex_id].blocked,
			sem_list[mutex_id].blocked_mask);
	
	// if priority inheritance is enabled
	// set its prio to the highest blocked task on this mutex
	// or its original prio, whatever prio is highest
	if (sem_list[mutex_id].mode & MUTEX_MODE_INHER) {
		
		// save the prio val coming into this part
		uint32_t init_prio = running.head->priority;
		
		// fix the prio value to either original or next highest
		// currently blocked on the mutex
		if (list != NULL && list->head->priority < running.head->priority) {
			running.head->priority = list->head->priority;
		} else {
			running.head->priority = sem_list[mutex_id].owner_orig_prio;
		}
		
		// if this section changes the priority at all, this means the tcb
		// exists already in the ready lists at the original prio
		// so it should NOT get added back to ready by the scheduler
		if (init_prio != running.head->priority) {
			// if it is still not back to its orig prio, that means another
			// task was also higher prio and blocked on it,
			// so it needs to be added onto that prio ready list
			if (running.head->priority != sem_list[mutex_id].owner_orig_prio) {
				enqueue(running.head, ready + running.head->priority, &ready_mask);
			}
			// tell the scheduler to NOT add running task to ready list
			running_handled = true;
		}
		
	}
				
	// check if there are any tasks currently blocked on it
	// if list is NULL there is nothing blocked
	if (list != NULL) {
		// add its head to the appropriate ready list
		tcb_t *task = dequeue(list, &sem_list[mutex_id].blocked_mask);
		enqueue(task, ready + task->priority, &ready_mask);
		// only call the scheduler if the unblocked task is higher prio
		if (running.head->priority > task->priority) {
			os_schedule();
		}

	}
	enable_irq();
	return OS_OK;
}

