#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <syscall.h>
#include <thread.h>
#include <test.h>


void *worker(void* input)
{
    if ((gettid() % 3) == 0) {
        printf("***********************************************************************\n");
        printf("* I believe our adventure through time has taken a most serious turn. *\n");
        printf("***********************************************************************\n");
    }
    return input;
}

int
main(int argc, char *argv[])
{
    int tid, error, answer=69;
    void *tstatus;

    if (thr_init(65536) < 0) {
        printf("BOGUS: thr_init() failed!");
    }
#if 0
    if (thr_init(65) < 0) {
        printf("BOGUS: thr_init() failed!");
    }
#endif


    if ((tid = thr_create(worker, (void *)answer)) < 0) {
        printf("HEINOUS: thr_create() failed!");
    }

    if ((error = thr_join(tid, &tstatus)) != 0) {
        printf("MOST NON-TRIUMPHANT: thr_join() failed!");
    }

    if ((int)tstatus != answer) {
        printf("BOGUS!!");
    }

    printf("*******************************\n");
    printf("* Be excellent to each other. *\n");
    printf("*******************************\n");
    return (0);
}

