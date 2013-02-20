/** @file sem.c
 *  @brief Implemetation of semaphore
 *
 *  
 *  Concept: 
 *  
 *  @author Ankur Sharma (ankursha)
 *  @author Himanshu Pandey (himanshp)
 *  @bug No know bugs
 */

#include <simics.h>
#include <syscall.h>
#include <syscall_int.h>
#include <thr_internals.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "thread_common.h"
#include "util.h"
#include <mutex_type.h>
#include "mutex_internals.h"
#include <thread.h> 
#include <cond.h> 


int sem_init( sem_t *sem, int count )
{
	/* If sem has been initialized before, return -1 */
	if ( sem -> initd == 1 )
		return -1;

	/* Initialize the mutex first */
	if( mutex_init( &sv -> mp ) )
		return -1;


	/* Mutex is initialized, time to initialize the sem */
	sem -> initd = 1;

	sem -> count = count;

	/* Queue of threads is empty from the start */
	sem -> head = NULL;

	return 0;
}

/**
 * @brief	to wait on 
 * @param sem [description]
 */
void sem_wait( sem_t *sem )
{
	struct node new_thread ;
	int reject = 0 ;
	/* @ToDo Function to get thread id - yet to be defined */
	new_thread.tid =  thr_getid();
	new_thread.next = NULL;
	
	/* Get lock of mp for decrementing the count */

	mutex_lock( &sem -> mp );
		sem -> count --;
		if( sem -> count >= 0){
			mutex_unlock( &sem -> mp );		
			return;
		}

		/* Put the thread in the queue */
		push ( &sem -> head, &new_thread);
		deschedule( &reject );
	mutex_unlock( &sem -> mp );
	
	
}
void sem_signal( sem_t *sem )
{
	int tid;
	if( sem -> head == NULL )
		return;

	/* Dequeue a thread */
	mutex_lock( &sem -> mp ) ;
		
		sem -> count++;
		if( sem -> count >= 0){
			mutex_unlock( &sem -> mp );		
			return;
		}
		/* Make a thread runnable from the queue */
		tid = pop( &sem -> head ) ;
		if( tid < 0 ) {
			/* This check is not necessary, due to implementation of pop, but still kept it, wats ur thought ?*/
			mutex_unlock( &sem -> mp );
			return;	
		}
		/* Make the popped thread runnable */
		while ( make_runnable( tid ) <  0 )
			yield(tid);

	mutex_unlock( &sem -> mp );


}


void sem_destroy( sem_t *sem )
{

	/* If sem has been destroyed/uninitialized before, return -1 */
	if ( sem -> initd == 0 )
		return;

	/* If still there are threads in queue for the cond var, return error */
	if ( sem -> head != NULL )
		return;

	/* Gracefully Destroy condvar now*/
	mutex_destroy( &sem -> mp );
	sem -> initd = 0;
}