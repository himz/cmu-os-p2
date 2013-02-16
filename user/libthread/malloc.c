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

void *malloc(size_t __size)
{
    void *ptr = NULL;

    ptr = _malloc(__size);

    return (ptr);
}

void *calloc(size_t __nelt, size_t __eltsize)
{
    void *ptr = NULL;

    ptr = _calloc(__nelt, __eltsize);

    return (ptr);
}

void *realloc(void *__buf, size_t __new_size)
{
    void *ptr = NULL;

    ptr = _realloc(__buf, __new_size);

    return (ptr);
}

void free(void *__buf)
{
    _free(__buf);
    return;
}
