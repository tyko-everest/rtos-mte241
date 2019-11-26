#ifndef INCLUDE_TEST_SUITE_H
#define INCLUDE_TEST_SUITE_H

#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "blocking.h"
#include "rtos.h"
#include "scheduler.h"

void t1(void *arg);
void t2(void *arg);
void t3(void *arg);

bool test_queues(void);
void test_context_switch(void);

void sem_t1(void *arg);
void sem_t2(void *arg);
void test_semaphores(void);

void test_fpp(void);

void mutex_t1(void *arg);
void mutex_t2(void *arg);

#endif /* INCLUDE_TEST_SUITE_H */
