#ifndef INCLUDE_SCHEDULER_H
#define INCLUDE_SCHEDULER_H

#include "rtos.h"

// move finished task to end of list for specific priority

// find next task to move to running list && update global variable holding list of 
void os_schedule(void);

// move next task to the running list

#endif /* INCLUDE_SCHEDULER_H */
