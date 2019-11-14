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
	uint32_t blocked_mask;
	uint32_t count;
} os_semaphore_t;

typedef uint32_t os_semaphore_id_t;

// if it returns an error, sem is set to point to NULL
os_error_t os_new_semaphore(os_semaphore_id_t *sem_id, uint32_t init_count);

// wait on a semaphore
void os_wait(os_semaphore_id_t sem_id);

// signal a semaphore
void os_signal(os_semaphore_id_t sem_id);

#endif /* INCLUDE_BLOCKING_H */
