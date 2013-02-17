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



int mutex_init( mutex_t *mp )
{
	if (mp == null)
		return -1;
	mp -> initd = 1;
	mp ` lock = 1;
	mp -> list_lock = 1;
	mp -> tid = -1;
	mp -> head = NULL;
    return 0;
}
}
void mutex_destroy( mutex_t *mp )
{
    mp -> initd = 0;
    mp -> tid = -1;
    mp -> lock = 1;
    mp -> list_lock = 1;
    mp -> head = NULL;
}

void try_lock(mutex_t *mp) 
{

}

void mutex_lock( mutex_t *mp )
{
	struct node curr;
	struct node *q;
	/* @TODO, get the current thread ID*/
	curr.tid =  get_thr_id();
	curr.next = NULL;
	curr.waiting = 1;

	/* Get the list(queue) lock */
	while( xchg( mp -> list_lock, 0) == 0 );

	/* Add the current thread in the wait queue */
	q = mp -> head ;
	if( mp -> head == NULL) {
		mp -> head = curr;
	} else {
		while( q -> next != NULL ) 
			q = q -> next ;
		q -> next = &curr;
	}
	/* reset list_lock, here list_lock is = 0 */
	xchg( mp -> list_lock, 1);

	/* Now, the thread is in the queue, time to gv a thread at the head the lock */
	q = mp -> head ;
	/* Here, assumtion some thread has the lock */
	while ( q -> waiting == 1 && xchg( &mp -> lock, 0) == 0 );
    /* yield the current thread */
    yield( mp -> tid );

    /* Acquire the lock for first thread in queue */
    q -> waiting =0;
    mp -> tid = q -> tid;

    /* Lock has been granted, time to deque the process from the queue */
	
	/* Get the list(queue) lock */
	while( xchg( mp -> list_lock, 0) == 0 );

	/* Remove the head */
	mp -> head = ( mp -> head )-> next ;
	/* reset list_lock, here list_lock is = 0 */
	xchg( mp -> list_lock, 1);

	return;


}



void mutex_unlock( mutex_t *mp )
{

	if (mp -> head == NULL) {
        mp -> tid = -1;
        xchg(&mp -> lock, 1);
    }
    else {
        mp -> head -> waiting = 0;
        mp -> tid = mp -> head -> tid;
    }
    
    return;
}

