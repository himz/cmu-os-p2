/** @file cond.c
 *  @brief Implemetation of Condition Variable
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


/**
 * @brief	Every condition variable(condvar) will have a queue of threads currently blocked on the condvar.
 */


void push ( struct node ** headref, struct node* new_thread )
{
	struct node *q =  *headref ;
	/* If list is empty */
	if ( q == NULL ){
		lprintf("here");
		*headref = new_thread;
		q = *headref;
		lprintf("%d----", q->tid);
	} else {
		while( q -> next != NULL )
			q = q -> next ;
		q -> next = new_thread ;	
	}
	
	
}

/* Pop a thread from the queue of condvar*/
int pop ( struct node **headref )
{
	struct node *q = *headref ;
	if( q == NULL )
		return -1; // queue is empty
	/* Pop an element from the front of the queue */
	*headref = q -> next ;
	/* Return the thread id of the top thread of the queue */
	return q -> tid;
}



int cond_init( cond_t *cv )
{
	/* If condvar has been initialized before, return -1 */
	if ( cv -> initd == 1 )
		return -1;

	/* Initialize the mutex */
	if( mutex_init( &cv -> mp ))
		return -1;

	/* Mutex is initialized, time to initialize the condvar */
	cv -> initd = 1;

	/* Queue of threads is empty fro the start */
	cv -> head = NULL;

	return 0;

}

void cond_destroy( cond_t *cv )
{
	/* If condvar has been destroyed/uninitialized before, return -1 */
	if ( cv -> initd == 0 )
		return;

	/* If still there are threads in queue for the cond var, return error */
	if ( cv -> head != NULL )
		return;

	/* Gracefully Destroy condvar now*/
	mutex_destroy( &cv -> mp );
	cv -> initd = 0;
}

void cond_wait( cond_t *cv, mutex_t *mp )
{

	struct node * new_thread = malloc(sizeof(struct node));
	int reject = 0 ;
	/* @ToDo Function to get thread id - yet to be defined */
	new_thread -> tid =  thr_getid();
	new_thread -> next = NULL;
	lprintf("threda nu: %d", new_thread -> tid );
	/* Put the thread in the queue */
	mutex_lock( &cv -> mp );
		push ( &cv -> head, new_thread);
		if( cv -> head != NULL)
			lprintf("threda nu: %d", cv -> head -> tid);
		else
			lprintf("wait in .. ....... head null");
		mutex_unlock(mp);
	mutex_unlock( &cv -> mp );

	/* deschedule the current thread */
	deschedule( &reject );
	mutex_lock(mp);
}

/**
 * @brief	Wake up a thread from queue, blocked on condvar - if it exists
 * @param cv [description]
 */
void cond_signal( cond_t *cv )
{
	lprintf("------cp1\n");
	
	unsigned int tid;
	if( cv -> head == NULL ){
		lprintf("------ head null" )	;
		return;
	}
		

	/* Dequeue a thread */
	mutex_lock( &cv -> mp ) ;
	lprintf("------cp2\n");
		tid = pop( &cv -> head ) ;
		if( tid < 0 ) {
			/* This check is not necessary, due to implementation of pop, but still kept it, wats ur thought ?*/
			mutex_unlock( &cv -> mp );
			return;	
		}
		lprintf("------cp3\n");
		/* Make the popped thread runnable */
		while ( make_runnable( tid ) <  0 )
			thr_yield(tid);
		lprintf("signal done\n");
	mutex_unlock( &cv -> mp );
}

/**
 * @brief	Wake up all the threads in the waiting queue
 * @param cv [description]
 */
void cond_broadcast( cond_t *cv )
{
	int tid;
	if( cv -> head == NULL )
		return;
	mutex_lock( &cv -> mp ) ;
		while( cv -> head != NULL ) {

			/* Dequeue a thread */
			tid = pop( &cv -> head ) ;
			
			if( tid < 0 ) {
				/* This check is not necessary, due to implementation of pop, but still kept it, wats ur thought ?*/
				mutex_unlock( &cv -> mp );
				return;	
			}
			/* Make the popped thread runnable */
			while ( make_runnable( tid ) <  0 )
				yield(tid);
		}
	mutex_unlock( &cv -> mp );	
}