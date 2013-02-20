/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H


typedef struct rwlock {
  /* fill this in */
	int initd ;
	/* Mode : -1 = unlocked, 0 = reader locked, 1 = writer locked */
	int mode ;
	mutex_t mp;

    cond_t read, write;
    unsigned int count_readers, count_writers, count_read_queue, count_write_queue;
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
