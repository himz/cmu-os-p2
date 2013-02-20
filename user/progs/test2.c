#include <stdio.h>
#include <thread.h>
#include <mutex_type.h>
#include <mutex.h>
#include <simics.h>
#include <syscall.h>



/* Define a global mutex */
mutex_t mp ;
int tid1 = 0;
int tid2 = 0;
int tid3 = 0;
int test = 5;
int tid =0;
void *
thread_run(void *input_args)
{
    int input = 0;

    input = *((int *)(input_args));

    mutex_lock( &mp );
    test++;
    sleep(5);
    test--;
    lprintf("test = %d", test);
    mutex_unlock( &mp );

    while(1);
    
    return (NULL); 

}


int main()
{
    int rc = 0;
    int rm = 0;
    tid1 = 0;
    tid2 = 0;
    tid3 = 0;
    int args = 1;

    rc = thr_init(10);
    rm = mutex_init(&mp);
    lprintf("[APP_%s],  after thr_init  rc = %d, thread_run: %p\n", __FUNCTION__, rc, thread_run);

    tid1 = thr_create(thread_run, (void *)(&(args)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid);
    tid2 = thr_create(thread_run, (void *)(&(args)));

    tid3= thr_create(thread_run, (void *)(&(args)));

    while(1);
    return 0;
}
