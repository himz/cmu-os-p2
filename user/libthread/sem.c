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

/**
 * @brief Initialization routine for the semaphore. 
 * @param  sem   Semaphore variable
 * @param  count Number of resources. 
 * @return       0 on success and -1 on failure
 */
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
 * @brief Semaphore wait implementation. All the threads who want to get access
 *        to a resources, call for wait on it. If the count of people sharing, 
 *        the resource is available, it is allocated, else the thread is put
 *        in a queue and descheduled. 
 * @param sem Semaphore variable
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

/**
 * @brief Signal the thread or free the lock on the resource. 
 * @param sem Semaphore variable
 */
void sem_signal( sem_t *sem )
{
    int tid;
    struct node *node;
    int rc = SUCCESS;


/*  if( sem -> head == NULL ){
        lprintf("[DBG_%s], Head Null\n", __FUNCTION__);
    }
*/
    /* Dequeue a thread */
    mutex_lock( &sem -> mp ) ;

    sem -> count++;
    if( sem -> count > 0){
        mutex_unlock( &sem -> mp );     
        return;
    }

    /* Make a thread runnable from the queue */
    node = pop( &sem -> head );
    if (!node) {
        mutex_unlock( &sem -> mp );
        return;
    }

    tid = node->tid;
    /*
     * Set the reject variable to 1 a non zero value
     * so that deschedule fails now.
     */
    *(node->reject) = 1;

    free(node);    if( tid < 0 ) {
        /* This check is not necessary, due to implementation of pop */
        mutex_unlock( &sem -> mp );
        return; 
    }

    /* Make the popped thread runnable */
    lprintf("[DBG_%s], lin 133 \n", __FUNCTION__);

    rc = make_runnable(tid);
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