#include <stdio.h>
#include <thread.h>
#include <simics.h>
#include <syscall.h>

void *
thread_run(void *input_args)
{
    int input = 0;

    input = *((int *)(input_args));
    
    lprintf("Inside thread_run, input: %d\n", input);

    return (NULL); 

}


int main()
{
    int rc = 0;
    int tid = 0;
    int args = 1;
    int sleep_return =100;

    rc = thr_init(10);

    lprintf("[APP_%s],  after thr_init  rc = %d, thread_run: %p\n", __FUNCTION__, rc, thread_run);

    tid = thr_create(thread_run, (void *)(&(args)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid);

    


    while(1);
    #if 0
        int rc = 0;
        char *base_ptr = (char *)(0xffff3ffc);

        rc = new_pages(base_ptr, 4096);

        lprintf("Inside main, after new_pages, rc: %d\n", rc);
    #endif

    return 0;
}
