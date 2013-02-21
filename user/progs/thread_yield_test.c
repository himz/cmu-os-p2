#include <stdio.h>
#include <thread.h>
#include <simics.h>
#include <syscall.h>

void *
thread_run(void *input_args)
{
    int input = 0;

    input = *((int *)(input_args));

    sleep(5);

    printf("Inside thread_run, input: %d\n", input);

    thr_exit(NULL);

    return (NULL);
}

int main()
{
    int rc = 0;
    int tid = 0;

    int args = 1;
    //int args2 = 2;

    rc = thr_init(10);

    lprintf("[APP_%s],  after thr_init  rc = %d\n", __FUNCTION__, rc);

    tid = thr_create(thread_run, (void *)(&(args)));

    thr_yield(tid);

    return 0;
}
