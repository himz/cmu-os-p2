#include <stdio.h>
#include <thread.h>
#include <mutex_type.h>
#include <mutex.h>
#include <cond.h>
#include <sem.h>
#include <simics.h>
#include <syscall.h>

#define NUM_THREADS  3
#define TCOUNT 10
#define COUNT_LIMIT 12

void *doSomething1();
void *doSomething2();
void *doSomething3();

sem_t sem;

int main() {
    // initialize semaphore to 2
    sem_init(&sem, 2);
    int args1 = 1, rc =0;
    rc = thr_init(10);
    int thread1, thread2, thread3;

    thread1 = thr_create(doSomething1, (void *)(&(args1)));
    thread2 = thr_create(doSomething2, (void *)(&(args1)));
    thread3 = thr_create(doSomething3, (void *)(&(args1)));

    thr_join(thread1, NULL);
    thr_join(thread2, NULL);
    thr_join(thread3, NULL);

    return 0;
}

void doSomething(char c) {
    int i, time;
    for (i = 0; i < 3; i++) {

        // P operation
        sem_wait(&sem);

            // generate random amount of time (< 30 seconds)
            time =  5 ;//(int) ((double) rand() / RAND_MAX * 30);

            printf("Thread %c enters and sleeps for %d seconds...\n", c, time);

            sleep(time);

            printf("Thread %c leaves the critical section\n", c);

            // V operation
            sem_signal(&sem);
        
    }
}

void *doSomething1() {
    // thread A
    doSomething('A');

    return 0;
}

void *doSomething2() {
    // thread B
    doSomething('B');

    return 0;
}

void *doSomething3() {
    // thread C
    doSomething('C');

    return 0;
}
