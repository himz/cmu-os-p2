
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

#endif