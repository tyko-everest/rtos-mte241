#include "rtos.h"

// used by the scheduler to manage context switches
uint32_t curr_sp = 0, next_sp = 0;
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
	
	// get address of current tasks's sp
	LDR R0,=__cpp(&curr_sp)
	// store current end of task's stack pointer
	STR SP, [R0]
	
	// get sp of next task's stack into register
	LDR R0,=__cpp(&next_sp)
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

void SysTick_Handler(void) {
	// set a pending context switch
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void enqueue(tcb_t *task, task_list_t *list) {
    // check task is not NULL
    if (task == NULL) {
        return;
    }
    // check if the list is empty
    if (list->head == NULL) {
        list->head = task;
        list->tail = task;
    // if not append to existing items
    } else {
        list->tail->next = task;
        task->next = NULL;
        list->tail = task;
    }
}

// returns the pointer of the removed tcb
tcb_t * dequeue(task_list_t *list) {
    tcb_t *ret_tcb;
    // check if it is empty
    if (list->head == NULL) {
        ret_tcb = NULL;
    // check if it has only one tcb left
    } else if (list->head == list->tail) {
        ret_tcb = list->head;
        list->head = NULL;
        list->tail = NULL;
    // more then one item left
    } else {
        ret_tcb = list->head;
        // advance the head to the second item in the list
        list->head = list->head->next;
    }
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
	return NULL;
	//while(1);
}
