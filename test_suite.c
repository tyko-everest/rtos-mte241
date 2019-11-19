#include "test_suite.h"

extern task_list_t running;
extern uint32_t ready_mask;
extern task_list_t ready[NUM_PRIORITIES];

os_semaphore_id_t sem;

uint32_t a = 0, b = 0, c = 0;

void delay() {
	for (int i = 0; i < 1000; i++);
}

void t1(void *arg) {
	while (1) {
		os_wait(sem);
		a++;
		c++;
		os_signal(sem);
		
		delay();
	}
}

void t2(void *arg) {
	while (1) {
		os_wait(sem);
		b++;
		c++;
		os_signal(sem);
		
		delay();
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

void test_semaphores(void) {
	
	os_kernel_init();
	os_task_attribs_t t1_attribs = {1};
	os_task_attribs_t t2_attribs = {1};
	
	os_add_task(t1, NULL, &t1_attribs);	
	os_add_task(t2, NULL, &t2_attribs);

	os_new_semaphore(&sem, 1);
		
	os_kernel_start();
	
}
