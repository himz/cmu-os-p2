/** @file malloc.c
 *  @brief Wrapper over mem alloc/free functions.
 *
 *  This file contains the thread safe wrappers around mem
 *  allocation/free APIS. 
 *
 *  @author Ankur Kumar Sharma (ankursha)
 *  @bug No known bugs.
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <simics.h>
#include <thread_common.h>

/** @brief  Wrapper around _malloc, it makes sure that
 *          it reuests the calling thread to acquire a 
 *          mutex before movinf further. 
 *
 *  @param  __size: Size of memory to be allocated.
 *
 *  @return void
 */
void *malloc(size_t __size)
{
    void *ptr = NULL;

    thr_mutex_mem_lock();

    ptr = _malloc(__size);

    thr_mutex_mem_unlock();

    return (ptr);
}

/** @brief  Wrapper around _calloc, it makes sure that
 *          it reuests the calling thread to acquire a 
 *          mutex before movinf further. 
 *
 *  @param  __size: Size of each element.
 *  @param  __eltsize: Number of elements.
 *
 *  @return void
 */
void *calloc(size_t __nelt, size_t __eltsize)
{
    void *ptr = NULL;

    thr_mutex_mem_lock();

    ptr = _calloc(__nelt, __eltsize);

    thr_mutex_mem_unlock();

    return (ptr);
}

/** @brief  Wrapper around _recalloc, it makes sure that
 *          it requests the calling thread to acquire a 
 *          mutex before moving further. 
 *
 *  @param  _buf: Address of prev allocated memory.
 *  @param  __new_size: Amount of new memory to be allocated.
 *
 *  @return void
 */
void *realloc(void *__buf, size_t __new_size)
{
    void *ptr = NULL;
    
    thr_mutex_mem_lock();

    ptr = _realloc(__buf, __new_size);

    thr_mutex_mem_unlock();

    return (ptr);
}

/** @brief  Wrapper around _free, it makes sure that
 *          it requests the calling thread to acquire a 
 *          mutex before moving further. 
 *
 *  @param  _buf: Address of prev allocated memory.
 *
 *  @return void
 */
void free(void *__buf)
{
    thr_mutex_mem_lock();

    _free(__buf);

    thr_mutex_mem_unlock();

    return;
}
