#include <stdio.h>
#include <thread.h>
#include <simics.h>
#include <syscall.h>

void *
thread_run(void *input_args)
{
    int input = 0;
    int tid = thr_getid();

    input = *((int *)(input_args));

    //sleep(5);

    printf("Inside thread_run, input: %d, tid: %d\n", input, tid);

    //while(1);
    return (NULL);
}


int main()
{
    int rc = 0;
    int tid1 = 0;
    int tid2 = 0;

    int args1 = 1;
    int args2 = 2;

    rc = thr_init(10);

    lprintf("[APP_%s],  after thr_init  rc = %d, thread_run: %p\n", __FUNCTION__, rc, thread_run);

    tid1 = thr_create(thread_run, (void *)(&(args1)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid1);

    tid2 = thr_create(thread_run, (void *)(&(args2)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid2);

    thr_join(tid1, NULL);

    printf("Done with thread 1 \n");

    thr_join(tid2, NULL);

    printf("Done with thread 2 \n");


    //sleep(100);

    //while(1);

#if 0
    int rc = 0;
    char *base_ptr = (char *)(0xffff3ffc);

    rc = new_pages(base_ptr, 4096);

    lprintf("Inside main, after new_pages, rc: %d\n", rc);
#endif

    return 0;
}
