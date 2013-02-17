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
#include <mutex.h>

/**
 * @brief	Initialize the mutex variable. It should be called before any mutex locks are asked.
 * @param  mp mutex variable 
 * @return    0 on succes, -1 on error
 */
int mutex_init( mutex_t *mp )
{
	if ( mp == NULL )
		return -1;
	mp -> initd = 1;
	mp -> lock = 1;
    return 0;
}


/**
 * @brief	destroy the mutex. Mutex lock should not be used after mutex is destroyed
 * @param mp  mutex variable
 */
void mutex_destroy( mutex_t *mp )
{
	if( mp == NULL || !mp -> initd)
		return;
    mp -> initd = 0;
    mp -> lock = 1;
}

/**
 * @brief	Try to get the lock on the mutex variable
 * @param mp Mutex variable
 */
void mutex_lock( mutex_t *mp )
{
	if( mp == NULL || !mp -> initd)
		return;
	/* If the current thread is holding the lock ?? what to do */
    /* if, its locked, yield the current thread, else continue */
    if( xchg( &mp -> lock, 0) == 0)
    	yield( -1 );
	return;
}


/**
 * @brief	Unlock the mutex variable.
 * @param mp mutex variable
 */
void mutex_unlock( mutex_t *mp )
{
	if( mp == NULL || !mp -> initd)
		return;
	mp -> lock = 1;
    return;
}

