#include "test_suite.h"

extern task_list_t running;
extern uint32_t ready_mask;
extern task_list_t ready[NUM_PRIORITIES];

os_semaphore_id_t sem;

uint32_t a, b, c, d;

void delay() {
	for (int i = 0; i < 1000; i++);
}

void t1(void *arg) {
	while (1) {
		a++;
		c++;
	}
}

void t2(void *arg) {
	while (1) {
		b++;
		c++;
	}
}

void t3(void *arg) {
	while (1) {
		d++;
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

// Task to Exit Task Scheduling after certain amount of time
void timer_task(void *arg) {
	while(get_ticks() != 5000);
}

// t1 increments a, t2 increments b, if context switching occurs then both
// would increment. Most of the time each task gets pre-empted during the delay
// function which would push the variables onto the stack.
void test_context_switch(void) {
	a = 0; b = 0; c = 0;
	os_kernel_init();
	os_add_task(t1, NULL, NULL);	
	os_add_task(t2, NULL, NULL);
	os_add_task(timer_task, NULL, NULL);
	os_kernel_start();
}

void sem_t1(void *arg) {
	while (1) {
		os_wait(sem);
		for(int i = 0; i < 10; i++)
			printf("Task 1\n");
		a++;
		c++;
		os_signal(sem);
		
		delay();
	}
}

void sem_t2(void *arg) {
	while (1) {
		os_wait(sem);
		for(int i = 0; i < 10; i++)
			printf("Task 2\n");
		b++;
		d++;
		os_signal(sem);
		
		delay();
	}
}

void sem_t3(void *arg) {
	while (1) {
		os_wait(sem);
		for(int i = 0; i < 10; i++)
		c++;
		d++;
		os_signal(sem);
		
		delay();
	}
}

void test_semaphores(void) {
	a = 0; b = 0; c = 0; d = 0;
	
	os_kernel_init();
	os_task_attribs_t t1_attribs = {1};
	os_task_attribs_t t2_attribs = {1};
	
	os_add_task(sem_t1, NULL, NULL);	
	os_add_task(sem_t2, NULL, NULL);

	os_new_semaphore(&sem, 1);
		
	os_kernel_start();
}
// a, b, and c will increment since they are of higher priority; d will not.
void test_fpp(void) {
	
	a = 0; b = 0; c = 0; d = 0;
	
	os_kernel_init();
	os_task_attribs_t t1_attribs = {1};
	os_task_attribs_t t2_attribs = {1};
	os_task_attribs_t t3_attribs = {10};
	
	os_add_task(t1, NULL, &t1_attribs);	
	os_add_task(t2, NULL, &t2_attribs);
	os_add_task(t3, NULL, &t3_attribs);

	os_kernel_start();
}