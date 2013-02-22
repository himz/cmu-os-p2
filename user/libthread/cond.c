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
 * @brief	Every condition variable(condvar) will have a 
 * queue of threads currently blocked on the condvar.
 */
void 
push (struct node ** headref, struct node* new_thread)
{
    struct node *q =  *headref ;

    /* If list is empty */
    if ( q == NULL ) {

        *headref = new_thread;
        q = *headref;
        lprintf("%d----", q->tid);

    } else {

        while (q -> next != NULL)
            q = q -> next ;

        q -> next = new_thread ;	
    }
        lprintf("getting out of push");
    return;
}

/* Pop a thread from the queue of condvar*/
struct node *
pop (struct node **headref)
{
	struct node *q = *headref ;

    if (q) {

        /* Pop an element from the front of the queue */
        *headref = q -> next ;

    } else {
        
        *headref = NULL;
    }

    return (q);
}

int 
cond_init( cond_t *cv )
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

void 
cond_destroy( cond_t *cv )
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

void 
cond_wait (cond_t *cv, mutex_t *mp)
{
	struct node * new_thread =	NULL;
    int reject = 0 ;

    if (!cv) {

        /*
         * Incorrect input from application.
         */
        return;
    }

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

	/* Put the thread in the queue */
    mutex_lock(&(cv -> mp));

    push (&cv -> head, new_thread);

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
void 
cond_signal(cond_t *cv )
{
	int tid;
    int rc = SUCCESS;
    struct node *node = NULL;

    if (!cv) {

        /*
         * Invalid input.
         */
        return;
    }

    if (cv -> head == NULL ) {

        /*
         * Nothign waiting on it.
         */
        return;
    }

	/* Dequeue a thread */
	mutex_lock( &(cv->mp));

    node = pop( &(cv -> head ));

    if (!node) {
        /*
         * Queue is empty.
         */
        mutex_unlock(&(cv->mp));
        return;
    }

    tid = node->tid;

    /*
     * Set the reject variable to 1 a non zero value
     * so that deschedule fails now.
     */
    *(node->reject) = 1;

    free(node);

    if (tid < 0) {
        /* 
         * This check is not necessary, due to implementation of pop, 
         * but still kept it, wats ur thought ?
         */
        mutex_unlock( &cv -> mp );
        return;	
    }

    /* Make the popped thread runnable */
    rc = make_runnable(tid);

    if (rc != SUCCESS) {

        /*
         * Should not have happened, but we'll log the event.
         */
        lprintf("[DBG_%s], make_runnable failed for tid: %d \n",
                                             __FUNCTION__, tid);
    }

    mutex_unlock(&cv -> mp );

    return;
}

/**
 * @brief	Wake up all the threads in the waiting queue
 * @param cv [description]
 */
void cond_broadcast( cond_t *cv )
{
    int tid = -1;
    struct node *node = NULL;
    int rc = SUCCESS;

    if ( cv -> head == NULL )
        return;

    mutex_lock( &cv -> mp ) ;

    while (1) {

        node = pop(&(cv -> head ));

        if (!node) {
            /*
             * Queue is empty.
             */
            break;
        }

        /* Dequeue a thread */
        tid = node->tid;
        
        /*
         * Set the reject variable to 1 a non zero value
         * so that deschedule fails now.
         */
        *(node->reject) = 1;

        free(node);

        if( tid < 0 ) {
            /* 
             * This check is not necessary, due to implementation of pop, 
             * but still kept it, wats ur thought ?
             */
            continue;
        }

        /* Make the popped thread runnable */
        rc = make_runnable( tid );
        if (rc != SUCCESS) {

            /*
             * Should not have happened, but we'll log the event.
             */
            lprintf("[DBG_%s], make_runnable failed for tid: %d \n",
                                                 __FUNCTION__, tid);
        }
    }

    mutex_unlock( &cv -> mp );	
}
