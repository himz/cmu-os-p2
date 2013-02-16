/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H


typedef struct mutex {
    int initd;			/* Whether this mutex initialized before*/
    int list_lock;		/* List of blocked threads */
    int lock;			/* Locked, unlocked*/
    int owner;			/* tid of Thread holding lock */
    struct node *head;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
