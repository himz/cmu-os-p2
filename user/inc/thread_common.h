/** @file   thr_internals.h
 *
 *  @brief  This file is used to declare the data types, functions
 *          Macros etc which are made available by thread module to
 *          other modules in library.
 *
 *  @Author Ankur Kumar Sharma (ankursha)
 */
#ifndef _THREAD_COMMON_H_
#define _THREAD_COMMON_H_

/*
 * Exposed THREAD APIs.
 */
inline void thr_set_main_stackH(char *input);
inline void thr_set_main_stackL(char *input);
inline char * thr_get_main_stackH();
inline char * thr_get_main_stackL();
void thr_mutex_mem_lock();
void thr_mutex_mem_unlock();
void thr_int_add_main_tcb(char *stack_hi, char *stack_lo);

#endif
