#include "rtos.h"

uint32_t curr_sp = 0, next_sp = 0;
tcb_t tcb_list[MAX_NUM_TASKS] = {0};

task_list_t running;
task_list_t ready[NUM_PRIORITIES];

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
	
	uint32_t temp = curr_sp;
	curr_sp = next_sp;
	next_sp = temp;
	
	// set a pending context switch
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void os_kernel_init(void) {
	// initialize the saved stack pointer in each tcb
	// to the base of that task's stack
	for (int i = 0; i < MAX_NUM_TASKS; i++) {
		tcb_list[i].sp = (uint32_t*) TASK_STACK_BASE_ADDR(i);
	}
    // add the idle task as task 0
	os_add_task(os_idle_task, NULL);
    // init the running list to point to the idle task
    running.head = tcb_list[0];
    running.tail = tcb_list[0];
    // init the ready task lists so all pointers point to NULL
    for (int i = 0; i < NUM_PRIORITIES; i++) {
        ready[i].head = NULL;
        ready[i].tail = NULL;
    }
}

os_error_t os_add_task(os_task_func_t func_pointer, void *args, os_task_attribs_t *attribs) {
	// keep track of number of added tasks
	static uint32_t curr_num_tasks = 0;
	// can only add up to 6 tasks (including idle task)
	if (curr_num_tasks == MAX_NUM_TASKS) {
		return OS_ERR_MEM;
	}
    // check the parameters are valid
    if (attribs->priority < HIGHEST_PRIORITY || attribs->priority > LOWEST_PRIORITY) {
        return OS_ERR_ARG;
    }

	tcb_list[curr_num_tasks].id = curr_num_tasks;
    tcb_list[curr_num_tasks].priority = attribs->priority;
	
	// set the stack pointer to the end of this constructed stack setup
	tcb_list[curr_num_tasks].sp -= NUM_REGS;
	
	// set r0 to the address of the arguments
	tcb_list[curr_num_tasks].sp[8] = (uint32_t) args;
	// set PC to the address of the start of the task
	tcb_list[curr_num_tasks].sp[14] = (uint32_t) func_pointer;
	// set PSR to default value
	tcb_list[curr_num_tasks].sp[15] = PCR_DEF_VAL;
	// we don't care about the rest of the registers

    // add this tcb to the appropriate linked list
	add_tail_task(tcb_list + curr_num_tasks, ready + attribs->priority);

	curr_num_tasks++;
	
	return OS_OK;
	
}

void os_kernel_start() {
	// reset MSP
	__set_MSP(MAIN_STACK_BASE_ADDR);
	
	// switch to PSP
	uint32_t val = __get_CONTROL();
	// bit set is PSP, bit reset is MSP
	val |= CONTROL_SPSEL_Msk;
	__set_CONTROL(val);
	
	// set PSP to idle task base
	// idle task is task 0
	__set_PSP(TASK_STACK_BASE_ADDR(0));
	
	// setup interrupt priorties
    // SysTick is the highest, it should override anything
	NVIC_SetPriority(SysTick_IRQn, 0);
    // context switching should be the lowest priority
	NVIC_SetPriority(PendSV_IRQn, 0xFF);
	
	// every 20 ms switch
	SysTick_Config(SystemCoreClock / 1000 * 100);
	
	// start idle task
	t1(NULL);
}

void os_schedule() {
    
}

void add_head_task(tcb_t *task, task_list_t *list) {
    // check task is not NULL
    if (task == NULL) {
        return;
    }
    // check if the list is empty
    if (list->head == NULL) {
        list->head = task;
        list->tail = task;
    // if not add to start of list
    } else {
        list->head->prev = task;
        task->prev = NULL;
        task->next = list->head;
        list->head = task;
    } 
}

void add_tail_task(tcb_t *task, task_list_t *list) {
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
        task->prev = list->tail;
        task->next = NULL;
        list->tail = task;
    }
}

// returns the pointer of the removed tcb
tcb_t * remove_head_task(task_list_t *list) {
    tcb_t ret_tcb;
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
        // set its prev pointer to NULL
        list->head->prev = NULL;
    }
    return ret_tcb;
}

// returns the pointer to the removed tcb
tcb_t * remove_tail_task(task_list_t *list) {
    tcb_t ret_tcb;
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
        ret_tcb = list->tail;
        // regress the tail to the second last item in the list
        list->tail = list->tail->prev;
        // set its next pointer to NULL
        list->tail->next = NULL;
    }
    return ret_tcb;
}


void os_idle_task(void *args) {
	while(1);
}