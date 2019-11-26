#ifndef INCLUDE_SCHEDULER_H
#define INCLUDE_SCHEDULER_H

#include <LPC17xx.h>
#include <stdbool.h>

#include "rtos.h"

#define TIMESLICE_MS 5

// find next task to move to running list && update global variable holding list of 
void os_schedule(void);

void os_yield(void);

uint32_t get_ticks(void);

#endif /* INCLUDE_SCHEDULER_H */
