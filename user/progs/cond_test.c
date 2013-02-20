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
cond_t count_nonzero; 
unsigned count = 0 ; 
int tid1 = 0;
int tid2 = 0;

void decrement_count() 
{ 
	mutex_lock(&count_lock); 
	 while (count == 0) 
	    cond_wait(&count_nonzero, &count_lock); 
	 count = count - 1; 
	mutex_unlock(&count_lock); 
} 

void increment_count() 
{ mutex_lock(&count_lock); 
     if (count == 0)
       cond_signal(&count_nonzero); 
     count = count + 1;
     mutex_unlock(&count_lock); 
}

void *thread1(void *input_args) 
{
	mutex_lock(&count_lock); 
		decrement_count();
		printf("Decrement count = %d\n",count);
	mutex_unlock(&count_lock);
	return (NULL);
}


void * thread2(void *input_args) 
{
	mutex_lock(&count_lock); 
		increment_count();
		printf("Increment count = %d\n",count);	
	mutex_unlock(&count_lock);

	return (NULL);
}
int main() {

	int rm = 0;
	int args1 = 1;
    int args2 = 2;
    rm = mutex_init(&count_lock);

    tid1 = thr_create(thread1, (void *)(&(args1)));

    lprintf("[APP_%s],  after thr_create  tid = %d\n", __FUNCTION__, tid1);

    tid2 = thr_create(thread2, (void *)(&(args2)));


    return 0;
}


