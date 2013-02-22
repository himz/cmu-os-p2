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
 #include <sem.h>


int sem_init( sem_t *sem, int count )
{
	/* If sem has been initialized before, return -1 */
	if ( sem -> initd == 1 )
		return -1;

	/* Initialize the mutex first */
	if( mutex_init( &sem -> mp ) )
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
    struct node *new_thread;
    int reject = 0 ;
    /* @ToDo Function to get thread id - yet to be defined */
    new_thread = malloc(sizeof(struct node));
    if (!new_thread) {
        /*
         * We are out of memory.
         * log it & return.
         */
        lprintf("[DBG_%s], malloc failed \n", __FUNCTION__);
        return;
    }

    memset(new_thread, 0, sizeof(struct node));

    new_thread -> tid =  gettid();
    new_thread -> next = NULL;
    new_thread -> reject = &reject;


    /* Get lock of mp for decrementing the count */
    lprintf("[DBG_%s], tid = %d \n", __FUNCTION__, new_thread -> tid );
    mutex_lock( &sem -> mp );
    sem -> count --;
    if( sem -> count >= 0){
        mutex_unlock( &sem -> mp );		
        return;
    }

    /* Put the thread in the queue */
    push ( &sem -> head, new_thread);
    mutex_unlock( &sem -> mp ); 
    deschedule( &reject );
    lprintf("[DBG_%s], deschedule done \n", __FUNCTION__);
    
}

void sem_signal( sem_t *sem )
{
	int tid;
    struct node *node;
    int rc = SUCCESS;
    lprintf("[DBG_%s], enter sem signal \n", __FUNCTION__);
	if( sem -> head == NULL ){
        lprintf("[DBG_%s], Head Null\n", __FUNCTION__);
    }
		
    //mutex_unlock( &sem -> mp );
	/* Dequeue a thread */
    mutex_lock( &sem -> mp ) ;

    sem -> count++;
    if( sem -> count > 0){
        lprintf("[DBG_%s],here   kkkkkk\n", __FUNCTION__);
        mutex_unlock( &sem -> mp );		
        return;
    }

    /* Make a thread runnable from the queue */
    node = pop( &sem -> head );
    lprintf("[DBG_%s], lin 112 \n", __FUNCTION__);
    if (!node) {
        mutex_unlock( &sem -> mp );
        return;
    }
        
    lprintf("[DBG_%s], lin 118 \n", __FUNCTION__);
    tid = node->tid;
    /*
     * Set the reject variable to 1 a non zero value
     * so that deschedule fails now.
     */
    *(node->reject) = 1;

    free(node);    if( tid < 0 ) {
        /* This check is not necessary, due to implementation of pop, but still kept it, wats ur thought ?*/
        mutex_unlock( &sem -> mp );
        return;	
    }

    /* Make the popped thread runnable */
    lprintf("[DBG_%s], lin 133 \n", __FUNCTION__);
    while( ( rc = make_runnable(tid) )< 0)
        yield(tid);

    if (rc != SUCCESS) {

        /*
         * Should not have happened, but we'll log the event.
         */
        lprintf("[DBG_%s], make_runnable failed for tid: %d \n",
                                             __FUNCTION__, tid);
    }


    mutex_unlock( &sem -> mp );

    return;
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
