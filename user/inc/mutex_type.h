/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H


typedef struct mutex {
    int initd;			/* Whether this mutex initialized before*/
    int lock;			/* Locked = 0, unlocked = 1 */
    //int tid;			/* tid of Thread holding lock */
} mutex_t;

#endif /* _MUTEX_TYPE_H */
