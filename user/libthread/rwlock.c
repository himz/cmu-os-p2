/** @file rwlock.c
 *  @brief Implemetation of Reader's Lock
 *
 *  
 *  Concept: 
 *  
 *  @author Himanshu Pandey (himanshp)
 *  @bug No know bugs
 */

#include <simics.h>
#include <syscall.h>
#include <syscall_int.h>
#include <thr_internals.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "thread_common.h"
#include "util.h"
#include <mutex_type.h>
#include "mutex_internals.h"
#include <thread.h> 
#include <cond.h> 
#include <sem.h>
#include <rwlock.h>

/**
 * @brief   
 * @param  rwlock [description]
 * @return        [description]
 */
int rwlock_init( rwlock_t *rwlock )
{
    if( rwlock -> initd != 0)
        return -1;

    rwlock -> initd = 1;
    rwlock -> count_readers = 0 ;
    rwlock -> count_writers = 0 ;
    rwlock -> count_read_queue = 0 ;
    rwlock -> count_write_queue = 0 ;
    mutex_init( &rwlock -> mp);
    cond_init( &rwlock -> read);
    cond_init( &rwlock-> write); 
    rwlock -> mode = -1 ;

    return 0;
}

void rwlock_destroy( rwlock_t *rwlock )
{
    if( rwlock -> initd != 1)
        return ;
    rwlock -> initd = 0;
    mutex_destroy( &rwlock -> mp );
    cond_destroy( &rwlock -> read );
    cond_destroy( &rwlock -> write );
}

void rwlock_lock( rwlock_t *rwlock, int type )
{
    mutex_lock( &rwlock -> mp );

    if ( type == RWLOCK_WRITE ) {
   
        if ((rwlock->count_readers) + (rwlock->count_writers) > 0) {

            rwlock -> count_write_queue++;

            cond_wait( &rwlock -> write, &rwlock -> mp );
            rwlock -> count_write_queue--;
        }

        rwlock -> count_writers = 1;
        rwlock -> mode = 1;

    } else if ( type == RWLOCK_READ ) {

        if ((rwlock -> count_writers) + (rwlock -> count_write_queue) > 0) {

            rwlock -> count_read_queue++;

            cond_wait( &rwlock -> read, &rwlock -> mp );
            rwlock -> count_read_queue--;
        }

        rwlock -> count_readers++;
        rwlock -> mode = 0;
    }

    mutex_unlock( &rwlock -> mp );    

}

void rwlock_unlock( rwlock_t *rwlock )
{
    /* Writer mode */
    mutex_lock( &rwlock -> mp);

    if (rwlock -> mode == 1) {
            rwlock -> count_writers = 0;

            if(rwlock -> count_write_queue > 0 ) {

                cond_signal( &rwlock -> write) ;

            } else if( rwlock -> count_read_queue > 0 ) {

                cond_broadcast( &rwlock -> read ) ;
            }

            rwlock -> mode = -1 ;

    } else if ( rwlock -> mode == 0 ) {

        /* Readers Mode*/
        rwlock -> count_readers--;

        /* If reader count is now zero, signal write queue. */

        if (rwlock -> count_readers == 0 &&  
            rwlock -> count_write_queue > 0 ) {

            cond_signal( &rwlock -> write ) ;

        } else if (rwlock -> count_read_queue > 0) {

            cond_broadcast( &rwlock -> read ) ;
        }
    }  
     
    mutex_unlock( &rwlock -> mp );

    return;
}

void rwlock_downgrade( rwlock_t *rwlock)
{
    /* Writers Mode */
    mutex_lock( &rwlock -> mp );

    if (rwlock -> mode == 1) {

        rwlock -> mode = 0;
        rwlock -> count_readers++ ;
        rwlock -> count_writers = 0 ;
        /* @ToDo -- check this solution for degrade for any race issues */
    }

    cond_broadcast( &rwlock -> read ) ;

    mutex_unlock( &rwlock -> mp );
    return;
}

