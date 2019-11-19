#include "rtos.h"

// used by the scheduler to manage context switches
uint32_t **curr_sp, **next_sp;
// memory for tcbs
tcb_t tcb_list[MAX_NUM_TASKS] = {0};
// running "list", only even one item
task_list_t running;
// ready lists, one for every priority
task_list_t ready[NUM_PRIORITIES];
// bitmap to store which ready lists are non-empty
// necessary for use of 0(1) time use of CLZ in scheduler
uint32_t ready_mask = 0;

// useful info
// https://www.adamh.cz/blog/2016/07/context-switch-on-the-arm-cortex-m0/
// https://github.com/adamheinrich/os.h/blob/blog_2016_07/src/os_pendsv_handler.s
__asm void PendSV_Handler(void) {
	
	// backup current MSP
	MRS R3, MSP
	
	// get current psp value into sp
	MRS R2, PSP
	MOV SP, R2
	
	PUSH {R11}
	PUSH {R10}
	PUSH {R9}
	PUSH {R8}
	PUSH {R7}
	PUSH {R6}
	PUSH {R5}
	PUSH {R4}
	
	// get address of pointer to curr_sp
	LDR R1,=__cpp(&curr_sp)
	// get pointer to curr_sp
	LDR R0, [R1]
	// store current end of task's stack pointer
	STR SP, [R0]
	
	// get address of pointer to next stack
	LDR R1,=__cpp(&next_sp)
	// get pointer to next stack
	LDR R0, [R1]
	LDR SP, [R0]
		
	POP {R4}
	POP {R5}
	POP {R6}
	POP {R7}
	POP {R8}
	POP {R9}
	POP {R10}
	POP {R11}
	
	// set the psp to point to middle of the stack frame
	// hardware will handle the rest after
	MOV R2, SP
	MSR PSP, R2
	
	// return msp val to sp
	MOV SP, R3
	
  // return from handler
	BX		LR
}

void enqueue(tcb_t *task, task_list_t *list, uint32_t *mask) {
	// disable IRQ to ensure function runs fully
	disable_irq();
	// check task is not NULL
	if (task == NULL) {
		return;
	}
	// check if the list is empty
	if (list->head == NULL) {
		list->head = task;
		list->tail = task;
		
		//update mask
		if (mask != NULL) {
			uint32_t bitshift = (LOWEST_PRIORITY - task->priority);
			*mask |= 1 << bitshift;
		}
	// if not append to existing items
	} else {
		list->tail->next = task;
		task->next = NULL;
		list->tail = task;
	}
	enable_irq();
}

// returns the pointer of the removed tcb
tcb_t * dequeue(task_list_t *list, uint32_t *mask) {
	// disable IRQ to ensure function runs fully
	disable_irq();
	tcb_t *ret_tcb;
	// check if it is empty
	if (list->head == NULL) {
		ret_tcb = NULL;
	// check if it has only one tcb left
	} else if (list->head == list->tail) {
		ret_tcb = list->head;
		list->head = NULL;
		list->tail = NULL;
		
		//update mask
		if (mask != NULL) {
			uint32_t bitshift = (LOWEST_PRIORITY - ret_tcb->priority);
			*mask &= ~(1 << bitshift);
		}
	// more then one item left
	} else {
		ret_tcb = list->head;
		// advance the head to the second item in the list
		list->head = list->head->next;
	}
	enable_irq();
	return ret_tcb;
}

task_list_t* highest_priority_list(task_list_t* list, uint32_t priority_mask) {
	
	if (list == NULL) {
		return NULL;
	}
	
	uint32_t leading_zeroes = 0;
	
	__asm {
		CLZ leading_zeroes, priority_mask
	}
	
	// see if there is nothing in any of the ready lists
	if (leading_zeroes == 32) {
		return NULL;
	} else {
		return list + leading_zeroes;
	}
}

// Modifies the passed array
void list_contents(task_list_t *list, int* count) {
	for (int i = 0; i < NUM_PRIORITIES; i++) {
		uint32_t num_tasks_in_list = 0;
		task_list_t *next_list = list + i;
		tcb_t *task = next_list->head;
		while(task != NULL) {
			task = task->next;
			num_tasks_in_list++;
		}
		count[i] = num_tasks_in_list;
	}
}

void print_list(task_list_t *list) {
	int count[NUM_PRIORITIES] = {0};
	list_contents(list, count);

	for (int i = 0; i < 3; i++) {
		printf("Priority: %d, Count: %d\n", i, count[i]);
	}
	printf("\n");
}

uint32_t num_disables = 0;

void disable_irq() {
	__disable_irq();
	num_disables++;
}

void enable_irq() {
	num_disables--;
	if (num_disables == 0) {
		__enable_irq();
	}
}

