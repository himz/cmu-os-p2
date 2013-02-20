#include <stdio.h>
#include <thread.h>
#include <simics.h>
#include <syscall.h>

void *
thread_run2(void *input_args)
{
    int input = 0;

    input = *((int *)(input_args));

    sleep(5);

    printf("Inside thread_run, input: %d\n", input);
    printf("Thread 2 waiting on thread 1");

    thr_join(input, NULL);

    //while(1);
    return (NULL);
}



void *
thread_run1(void *input_args)
{
    int input = 0;

    input = *((int *)(input_args));

    sleep(5);

    printf("Inside thread_run, input: %d\n", input);

    while(1);
    return (NULL);
}


int main()
{
    int rc = 0;
    int tid1 = 0;
    int tid2 = 0;

    int args1 = 1;
    //int args2 = 2;

    rc = thr_init(10);

    lprintf("[APP_%s],  after thr_init  rc = %d\n", __FUNCTION__, rc);

    tid1 = thr_create(thread_run1, (void *)(&(args1)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid1);

    tid2 = thr_create(thread_run2, (void *)(&(tid1)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid2);

    printf("Waiting on  thread 2 \n");

    thr_join(tid2, NULL);

    printf("Done with thread 2 \n");

    printf("Waiting on  thread 1 \n");

    thr_join(tid1, NULL);

    printf("Done with thread 1 \n");

    return 0;
}
