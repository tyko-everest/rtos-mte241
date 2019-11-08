/*
 * Default main.c for rtos lab.
 * @author Andrew Morton, 2018
 */
#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define MAX_NUM_TASKS 6
#define NUM_REGS 16
#define PCR_DEF_VAL 0x01000000

#define MAIN_STACK_BASE_ADDR (*((volatile uint32_t*) 0))
#define TASK_STACK_BASE_ADDR(x) ((MAIN_STACK_BASE_ADDR - 0x800 - (0x400 * x)))


struct _tcb {
	uint32_t *sp;
	uint32_t start_addr, id;
	struct _tcb *next;
};
typedef struct _tcb tcb_t;

typedef enum _os_error {
	OS_OK = 0,
	OS_MEM_ERR
} os_error_t;
	

typedef void (*os_task_func_t) (void *args);

uint32_t msTicks = 0;
uint32_t curr_sp = 0, next_sp = 0;
tcb_t tcb_list[MAX_NUM_TASKS] = {0};

void os_kernel_init(void);
os_error_t os_add_task(os_task_func_t func_pointer, void *args);
void os_kernel_start(void);

void os_idle_task(void *arg);

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

void t1(void *arg) {
	while (1) {
		for(int i = 0; i < 1000000; i++);
		printf("task1\n");
	}
}

void t2(void *arg) {
	while(1) {
	  for(int i = 0; i < 1000000; i++);
		printf("task2\n");
	}
}

int main(void) {

	
	printf("\nStarting...\n\n");
	
	os_kernel_init();
	os_add_task(t1, NULL);
	os_add_task(t2, NULL);
	
	curr_sp = (uint32_t) tcb_list[2].sp;
	next_sp = (uint32_t) tcb_list[1].sp;
	
	os_kernel_start();
	
}

void os_kernel_init(void) {
	// initialize the saved stack pointer in each tcb
	// to the base of that task's stack
	for (int i = 0; i < MAX_NUM_TASKS; i++) {
		tcb_list[i].sp = (uint32_t*) TASK_STACK_BASE_ADDR(i);
	}
	os_add_task(os_idle_task, NULL);
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
	NVIC_SetPriority(SysTick_IRQn, 0);
	NVIC_SetPriority(PendSV_IRQn, 0xFF);
	
	// every 20 ms switch
	SysTick_Config(SystemCoreClock / 1000 * 100);
	
	// start idle task
	t1(NULL);
}

os_error_t os_add_task(os_task_func_t func_pointer, void *args) {
	// keep track of number of added tasks
	static uint32_t curr_num_tasks = 0;
	
	// can only add up to 6 tasks (including idle task)
	if (curr_num_tasks == MAX_NUM_TASKS) {
		return OS_MEM_ERR;
	}
	tcb_list[curr_num_tasks].id = curr_num_tasks;
	tcb_list[curr_num_tasks].start_addr = (uint32_t) func_pointer;
	
	// set the stack pointer to the end of this constructed stack setup
	tcb_list[curr_num_tasks].sp -= NUM_REGS;
	
	// set r0 to the address of the arguments
	tcb_list[curr_num_tasks].sp[8] = (uint32_t) args;
	// set PC to the address of the start of the task
	tcb_list[curr_num_tasks].sp[14] = (uint32_t) func_pointer;
	// set PSR to default value
	tcb_list[curr_num_tasks].sp[15] = PCR_DEF_VAL;
	// we don't care about the rest of the registers
	
	
	curr_num_tasks++;
	
	return OS_OK;
	
}

void os_idle_task(void *args) {
	while(1);
}
