
#ifndef _MUTEX_INTERNALS_H
#define _MUTEX_INTERNALS_H

#include <mutex_type.h>

struct node {

    int tid; /* Thread id. */

    /*
     * Wait flag, 1 = thread is waiting, 0 = not waiting.
     */
    int waiting;
    struct node *next;
    mutex_t reject_mutex; /* Mutex for accessing reject. */
};

int xchg(int *lock, int value);


/* Some helper functions */
/* Push the new thread on the queue of condvar*/
void push ( struct node ** headref, struct node* new_thread );

/* Pop a thread from the queue of condvar*/
struct node * pop ( struct node **headref );

#endif
