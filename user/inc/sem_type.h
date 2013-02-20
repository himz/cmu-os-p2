/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H


typedef struct sem {
	mutex_t mp;	/* Mutex for semaphore implementation */
	int count;			/* Counting Sem */
	int initd;
	struct node* head;
} sem_t;

#endif /* _SEM_TYPE_H */
