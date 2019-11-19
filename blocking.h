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
	// only used for mutexes
	uint32_t owner_orig_prio;
	tcb_t *current_owner;

} os_semaphore_t;

typedef uint32_t os_semaphore_id_t;

typedef struct {
	os_semaphore_id_t sem;
	
} os_mutex_t;

typedef uint32_t os_mutex_id_t;

// if it returns an error, sem_id remains uninitialized
os_error_t os_new_semaphore(os_semaphore_id_t *sem_id, uint32_t init_count);

// wait on a semaphore
void os_wait(os_semaphore_id_t sem_id);

// signal a semaphore
void os_signal(os_semaphore_id_t sem_id);

// if it returns an error, sem_id remains uninitialized
os_error_t os_new_mutex(os_semaphore_id_t *mutex_id);

// acquire a mutex
void os_acquire(os_mutex_id_t mutex_id);

// release a mutex
void os_release(os_mutex_id_t mutex_id);

#endif /* INCLUDE_BLOCKING_H */
