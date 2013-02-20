
#ifndef _MUTEX_INTERNALS_H
#define _MUTEX_INTERNALS_H
struct node {
    /**
     * Thread ID
     */
    int tid;

    /*
     * Wait flag, 1 = thread is waiting, 0 = not waiting.
     */
    int waiting;
    struct node *next;
};

int xchg(int *lock, int value);


/* Some helper functions */
/* Push the new thread on the queue of condvar*/
void push ( struct node ** headref, struct node* new_thread );

/* Pop a thread from the queue of condvar*/
int pop ( struct node **headref );

#endif