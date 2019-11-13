#include "kernel.h"

extern task_list_t running;
extern task_list_t ready[NUM_PRIORITIES];
extern tcb_t tcb_list[MAX_NUM_TASKS];
extern uint32_t ready_mask;
extern uint32_t curr_sp, next_sp;


void os_kernel_init(void) {
	// initialize the saved stack pointer in each tcb
	// to the base of that task's stack
	for (int i = 0; i < MAX_NUM_TASKS; i++) {
		tcb_list[i].sp = (uint32_t*) TASK_STACK_BASE_ADDR(i);
	}
	// add the idle task as task 0
	os_task_attribs_t idle_attribs = {LOWEST_PRIORITY};
	os_add_task(os_idle_task, NULL, &idle_attribs);
	// init the running list to point to the idle task
	running.head = &tcb_list[0];
	running.tail = &tcb_list[0];
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
	// default attribs accepted
	if (attribs == NULL) {
		os_task_attribs_t def_priorities = {DEF_PRIORITY};
		attribs = &def_priorities;

	} else {
		// check the parameters are valid
		if (attribs->priority > LOWEST_PRIORITY) {
			return OS_ERR_ARG;
		}
	}
	tcb_list[curr_num_tasks].priority = attribs->priority;
	
	tcb_list[curr_num_tasks].id = curr_num_tasks;

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
	enqueue(tcb_list + curr_num_tasks, ready + attribs->priority, &ready_mask);

	uint32_t bitshift = (LOWEST_PRIORITY - attribs->priority);
	ready_mask |= 1 << bitshift;
	
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
	
	// temporary verification of context switching
	curr_sp = (uint32_t) tcb_list[2].sp;
	next_sp = (uint32_t) tcb_list[1].sp;
	
	// call idle();
	os_idle_task(NULL);
}

void os_idle_task(void *args) {
	while(1);
}
