#ifndef INCLUDE_BLOCKING_H
#define INCLUDE_BLOCKING_H

#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rtos.h"
#include "scheduler.h"

#define MAX_SEMAPHORES 8

typedef struct {
	task_list_t blocked[NUM_PRIORITIES];
	uint32_t count;
} os_semaphore_t;

// if it returns an error, sem is set to NULL
os_error_t os_new_semaphore(os_semaphore_t *sem, uint32_t init_count);

// wait on a semaphore
void os_wait(os_semaphore_t * sem);

// signal a semaphore
void os_signal(os_semaphore_t *sem);

#endif /* INCLUDE_BLOCKING_H */
