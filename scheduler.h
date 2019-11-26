#ifndef INCLUDE_SCHEDULER_H
#define INCLUDE_SCHEDULER_H

#include <LPC17xx.h>
#include <stdbool.h>

#include "rtos.h"

// find next task to move to running list && update global variable holding list of 
void os_schedule(void);

#endif /* INCLUDE_SCHEDULER_H */
