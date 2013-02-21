/*
 * these functions should be thread safe.
 * It is up to you to rewrite them
 * to make them thread safe.
 *
 */
#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <simics.h>
#include <thread_common.h>

void *malloc(size_t __size)
{
    void *ptr = NULL;

    thr_mutex_mem_lock();

    ptr = _malloc(__size);

    thr_mutex_mem_unlock();

    return (ptr);
}

void *calloc(size_t __nelt, size_t __eltsize)
{
    void *ptr = NULL;

    thr_mutex_mem_lock();

    ptr = _calloc(__nelt, __eltsize);

    thr_mutex_mem_unlock();

    return (ptr);
}

void *realloc(void *__buf, size_t __new_size)
{
    void *ptr = NULL;
    
    thr_mutex_mem_lock();

    ptr = _realloc(__buf, __new_size);

    thr_mutex_mem_unlock();

    return (ptr);
}

void free(void *__buf)
{
    thr_mutex_mem_lock();

    _free(__buf);

    thr_mutex_mem_unlock();

    return;
}
