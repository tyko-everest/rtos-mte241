#include "test_suite.h"

extern task_list_t running;
extern uint32_t ready_mask;
extern task_list_t ready[NUM_PRIORITIES];
extern uint32_t prev_sp, curr_sp;

os_semaphore_id_t sem;
os_semaphore_id_t highest_blocker, medium_blocker;
os_mutex_id_t mutex, print_mutex;

uint32_t a, b, c, d;

void delay() {
	for (int i = 0; i < 1000000; i++);
}

void context_t1(void *arg) {
	while (1) {
		printf("Task 1\n");
	}
}

void context_t2(void *arg) {
	while (1) {
		printf("Task 2\n");
	}
}

void t1(void *arg) {
	while (1) {
		os_acquire(print_mutex);
		printf("Task 1\n");
		os_release(print_mutex);
		os_yield();
	}
}

void t2(void *arg) {
	while (1) {
		os_acquire(print_mutex);
		printf("Task 2\n");
		os_release(print_mutex);
		os_yield();
	}
}

void t3(void *arg) {
	while (1) {
		os_acquire(print_mutex);
		printf("Task 3\n");
		os_release(print_mutex);
		os_yield();
	}
}

// Testing the Functionalitity of Queues
bool test_queues(void) {
	
	os_kernel_init();
	int list_count[NUM_PRIORITIES] = {0};
	
	os_task_attribs_t t1_attribs = {1};
	os_task_attribs_t t2_attribs = {1};
	
	os_add_task(t1, NULL, &t1_attribs);
	list_contents(ready, list_count);
	if (list_count[1] != 1) {
		return false;
	}
	
	os_add_task(t2, NULL, &t2_attribs);
	list_contents(ready, list_count);
	if (list_count[1] != 2) {
		return false;
	}

	os_add_task(t2, NULL, &t2_attribs);
	list_contents(ready, list_count);
	if (list_count[1] != 3) {
		return false;
	}
	
	if (highest_priority_list(ready, ready_mask)->head->priority != t2_attribs.priority){
		return false;
	}

	// Test Dequeue
	if (dequeue(ready + DEF_PRIORITY, &ready_mask) != NULL) {
		return false;
	}
	if (dequeue(ready + t2_attribs.priority, &ready_mask) == NULL) {
		return false;
	}	
	if (dequeue(ready + t2_attribs.priority, &ready_mask) == NULL) {
		return false;
	}	
	if (dequeue(ready + t2_attribs.priority, &ready_mask) == NULL) {
		return false;
	}
	if (dequeue(ready + t2_attribs.priority, &ready_mask) != NULL) {
		return false;
	}
	return true;
}


void test_context_switch(void) {
	os_kernel_init();
	os_add_task(context_t1, NULL, NULL);	
	os_add_task(context_t2, NULL, NULL);
	os_kernel_start();
}

// a, b, and c will increment since they are of higher priority; d will not.
void test_fpp(void) {
	
	os_kernel_init();
	os_task_attribs_t t1_attribs = {1};
	os_task_attribs_t t2_attribs = {1};
	os_task_attribs_t t3_attribs = {10};
	
	os_add_task(t1, NULL, &t1_attribs);	
	os_add_task(t2, NULL, &t2_attribs);
	os_add_task(t3, NULL, &t3_attribs);
	
	os_new_mutex(&print_mutex, 0);

	os_kernel_start();
}

void sem_t1(void *arg) {
	while (1) {
		os_wait(sem);
		for(int i = 0; i < 10; i++) {
			printf("Task 1\n");
		}
		os_signal(sem);
		
		delay();
	}
}

void sem_t2(void *arg) {
	while (1) {
		os_wait(sem);
		for(int i = 0; i < 10; i++) {
			printf("Task 2\n");
		}
		os_signal(sem);
		
		delay();
	}
}

void sem_t3(void *arg) {
	while (1) {
		os_wait(sem);
		
		for(int i = 0; i < 10; i++) {
			printf("Task 3\n");
		}
		os_signal(sem);
		delay();
	}
}

void test_semaphores(void) {
	a = 0; b = 0; c = 0; d = 0;
	
	os_kernel_init();
	
	os_add_task(sem_t1, NULL, NULL);	
	os_add_task(sem_t2, NULL, NULL);
	os_add_task(sem_t3, NULL, NULL);

	os_new_semaphore(&sem, 1);
		
	os_kernel_start();
}

void sem_t4(void *arg) {
	uint32_t i = 1;
	while (1) {
		os_wait(sem);
	
		printf("Ran %d time(s)\n", i++);
	}
}

void test_semaphores_2(void) {
	a = 0; b = 0; c = 0; d = 0;
	
	os_kernel_init();
	
	os_add_task(sem_t4, NULL, NULL);

	os_new_semaphore(&sem, 5);
		
	os_kernel_start();
}

void prio_t1(void *arg) {
	
	delay();
	
	// change its own priority
	__disable_irq();
	running.head->priority = 1;
	__enable_irq();
	// release medium task
	os_signal(medium_blocker);
	
	delay();
	
	// change its own priority
	__disable_irq();
	running.head->priority = 0;
	__enable_irq();
	// release highest task
	os_signal(highest_blocker);
	
	// change its own priority
	// so it never runs again
	__disable_irq();
	running.head->priority = 3;
	__enable_irq();
	
	while (1);
}

void prio_t2(void *arg) {
	os_wait(highest_blocker);
	printf("Highest Released\n");
	while (1) {
		os_acquire(mutex);
		printf("highest task ran\n");
	}
}

void prio_t3(void *arg) {
	os_wait(medium_blocker);
	printf("Medium Released\n");
	while (1) {
		printf("medium task ran\n");
		delay();
	}
}

void prio_t4(void *arg) {
	printf("Lowest Released\n");
	while (1) {
		os_acquire(mutex);
		printf("lowest started\n");
		delay();
		printf("lowest ended\n");
		os_release(mutex);
	}
}

void test_prio_inher(void) {

	os_kernel_init();
	
	// task used to start higher priority tasks at later times
	os_task_attribs_t t1_attribs = {2};
	// highest priority task that relies on mutex shared with lower prio task
	os_task_attribs_t t2_attribs = {0};
	// medium priority task that takes a long time
	os_task_attribs_t t3_attribs = {1};
	// lowest priority task that shares the mutex with highest prio task
	os_task_attribs_t t4_attribs = {2};
	
	os_add_task(prio_t1, NULL, &t1_attribs);	
	os_add_task(prio_t2, NULL, &t2_attribs);
	os_add_task(prio_t3, NULL, &t3_attribs);
	os_add_task(prio_t4, NULL, &t4_attribs);

	os_new_semaphore(&highest_blocker, 0);
	os_new_semaphore(&medium_blocker, 0);
 	os_new_mutex(&mutex, MUTEX_MODE_INHER);
	
	os_kernel_start();

}

void mutex_owner(void *arg) {
	os_acquire(mutex);
	os_acquire(print_mutex);
	printf("Task 1 has acquired the mutex\n");
	os_release(print_mutex);
	while(1);
}

void mutex_stealer(void *arg) {
	os_error_t error = os_release(mutex);
	os_acquire(print_mutex);
	if (error == OS_ERR_PERM) {
		
		printf("Task 2 failed to steal mutex\n");
	}
	else {
		printf("Task 2 successfully stole the mutex\n");
	}
	os_release(print_mutex);
	while(1);
}

void test_mutex_ownership(void) {
	os_kernel_init();	
	os_add_task(mutex_owner, NULL, NULL);	
	os_add_task(mutex_stealer, NULL, NULL);
	os_new_mutex(&mutex, MUTEX_MODE_OWNER);
	os_new_mutex(&print_mutex, 0);

	os_kernel_start();
}

