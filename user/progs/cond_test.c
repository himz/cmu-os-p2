#include <stdio.h>
#include <thread.h>
#include <mutex_type.h>
#include <mutex.h>
#include <cond.h>
#include <simics.h>
#include <syscall.h>

#define NUM_THREADS  3
#define TCOUNT 10
#define COUNT_LIMIT 12

mutex_t count_lock; 
mutex_t mp;
cond_t count_nonzero, cond2;
unsigned count = 0 ; 
int tid1 = 0;
int tid2 = 0;

void decrement_count() 
{ 
    printf("***********Thread decrement in \n");

    mutex_lock(&count_lock);

    if (count == 0) {

        printf("%s: After sleep \n", __FUNCTION__);

        cond_wait(&count_nonzero, &count_lock);

        printf("[DBG_%s], After cond_wait \n", __FUNCTION__);

        cond_wait(&cond2, &count_lock);

        printf("[DBG_%s], After cond_wait II \n", __FUNCTION__);

    }

    count = count - 1; 

    mutex_unlock(&count_lock); 

    printf("Thread decrement out \n");
} 

void increment_count() 
{ 
	printf("***********Thread increment in \n");
    sleep(10);

	mutex_lock(&count_lock); 

     if (count == 0)
       cond_signal(&count_nonzero); 

     count = count + 1;

     mutex_unlock(&count_lock); 
     printf("Thread increment out \n");
}

void *thread1(void *input_args) 
{
    printf("***********Thread 1 in \n");
    decrement_count();
    printf("Decrement count = %d\n",count);
    printf("Thread 1 out \n");
    return (NULL);
}


void * thread2(void *input_args) 
{	
    printf("***********Thread 2 in \n");

    increment_count();
    printf("Increment count = %d\n",count);	
    printf("Thread 2 out \n");
    return (NULL);
}

int main() 
{
    int rm = 0, rc =0 ;
    int args1 = 1;
    int args2 = 2;

    rc = thr_init(10);

    rm = mutex_init(&count_lock);

    cond_init(&count_nonzero);
    cond_init(&cond2);

    tid1 = thr_create(thread1, (void *)(&(args1)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid1);

    tid2 = thr_create(thread2, (void *)(&(args2)));

    return 0;
}
