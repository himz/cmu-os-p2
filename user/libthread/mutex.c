#include
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



int mutex_init( mutex_t *m )
{
	if (m == null)
		return -1;
	m ->  initd = 1;
	m -> lock = 1;
	m -> list_lock = 1;
	m -> owner = -1;
	m -> head = NULL;
    return 0;
}
}
void mutex_destroy( mutex_t *m )
{
	    m -> initd = 0;
        m -> owner = -1;
        m -> lock = 1;
        m -> list_lock = 1;
        m -> head = NULL;
}
void mutex_lock( mutex_t *m )
{

}
void mutex_unlock( mutex_t *m )
{

	if (m->head == NULL) {
        m->owner = -1;
        xchg(&m->lock, 1);
    }
    else {
        m->head->waiting = 0;
        m->owner = m->head->tid;
    }
    
    return;
}

