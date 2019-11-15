#ifdef TEST_QUEUE

#include "test_queue.h"

extern task_list_t running;
extern uint32_t ready_mask;
extern task_list_t ready[NUM_PRIORITIES];


void t1(void *arg) {
	while (1);
}

void t2(void *arg) {
	while (1);
}

void test_queues(void) {
	
	highest_priority_list(running, ready_mask);
	os_add_task(t1, NULL, NULL);
	print_list_contents(ready);
	os_add_task(t2, NULL, NULL);
	print_list_contents(ready);
	os_add_task(t2, NULL, NULL);
	print_list_contents(ready);

	dequeue(ready + DEF_PRIORITY, &ready_mask);
	print_list_contents(ready);

	dequeue(ready + DEF_PRIORITY, &ready_mask);
	print_list_contents(ready);

	dequeue(ready + DEF_PRIORITY, &ready_mask);
	print_list_contents(ready);

	dequeue(ready + DEF_PRIORITY, &ready_mask);
}

#endif
