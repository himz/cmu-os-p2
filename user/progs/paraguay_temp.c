/** @file 410user/progs/paraguay.c
 *  @author mjsulliv
 *  @brief Tests condition variables in an interesting way.
 *  @public yes
 *  @for p2
 *  @covers cond_wait,cond_signal
 *  @status done
 *
 *  This makes sure that condition variables work properly even if
 *  signaled without the associated lock being held.
 */

/* Includes */
#include <syscall.h>
#include <stdlib.h>
#include <thread.h>
#include <mutex.h>
#include <cond.h>
#include "410_tests.h"
#include <report.h>
#include <test.h>
#include <stdio.h>

DEF_TEST_NAME("paraguay:");

#define MISBEHAVE_MAX   64

#define STACK_SIZE 4096

#define ERR REPORT_FAILOUT_ON_ERR

mutex_t lock1, lock2;
cond_t cvar1, cvar2;

int slept1 = 0; /* Whether thread1 has gotten to sleep on cvar1 */
int signaled1 = 0; /* Set right before main thread signals cvar1 */
int slept2 = 0; /* Whether thread1 has gotten to sleep on cvar2 */
int signaled2 = 0; /* Set right before main thread signals cvar2 */

void *thread1(void *dummy)
{
	int failed = 0;

	/* go to sleep on cvar1 */
	mutex_lock(&lock1);
	slept1 = 1;
	report_misc("thread1 sleeping on cvar1");
	cond_wait(&cvar1, &lock1);

	if (!signaled1) {
		failed = 1;
		report_misc("woke up from cvar1 without a signal!");
		report_end(END_FAIL);
	}

    printf("thread1 sleeping on cvar2 PREV\n");
    cond_wait(&cvar2, &lock2);
    printf("thread1 sleeping on cvar2 AFTER\n");

	mutex_unlock(&lock1);

	/* go to sleep on cvar1 */
	mutex_lock(&lock2);
	slept2 = 1;

	printf("thread1 sleeping on cvar2\n");
	cond_wait(&cvar2, &lock2);
    printf("thread1 sleeping on cvar2\n");

	if (!signaled2) {
		failed = 1;
		report_misc("woke up from cvar2 without a signal!");
		report_end(END_FAIL);
	}

	mutex_unlock(&lock2);

	if (!failed) {
		report_end(END_SUCCESS);
	}
	
	return NULL;
}
	

int main(void)
{
	report_start(START_CMPLT);

	//assuredly_misbehave((rand() % 521) % MISBEHAVE_MAX);

	ERR(thr_init(STACK_SIZE));
	ERR(mutex_init(&lock1));
	ERR(mutex_init(&lock2));
	ERR(cond_init(&cvar1));
	ERR(cond_init(&cvar2));

	ERR(thr_create(thread1, NULL));
	report_misc("thread1 created");

	/* Wait for thread1 to get to sleep on cvar1. */
	mutex_lock(&lock1);

    while (!slept1) {

        mutex_unlock(&lock1);
        thr_yield(-1);
        mutex_lock(&lock1);
    }

	/* Indicate that we are about to signal */
	signaled1 = 1;
	mutex_unlock(&lock1);

	/* Signal. Note that we know for sure that thread1 is asleep on
	 * cvar1 (assuming correct cond vars and mutexes...) */
    printf("cvar1 B4 signaled \n");
	cond_signal(&cvar1);
	printf("cvar1 After signaled \n");

	sleep(10);

    printf("[DBG_%s], main thread after sleep \n", __FUNCTION__);
	
	/* Now do it all again for the second set of things. */
	/* Wait for thread1 to get to sleep on cvar2. */
	mutex_lock(&lock2);

	while (!slept2) {

		mutex_unlock(&lock2);
        printf("[DBG_%s], main thread before yield \n", __FUNCTION__);
		thr_yield(-1);
        printf("[DBG_%s], main thread after yield \n", __FUNCTION__);
		mutex_lock(&lock2);
	}

	/* Indicate that we are about to signal */
	signaled2 = 1;
	mutex_unlock(&lock2);

	/* Signal. Note that we know for sure that thread1 is asleep on
	 * cvar1 (assuming correct cond vars and mutexes...) */
	//cond_signal(&cvar2);
	report_misc("cvar2 signaled");

	/* We report success from the other thread. */

	return 0;
}
