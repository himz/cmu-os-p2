#include <syscall_int.h>

.global gettid

gettid:
    int     $GETTID_INT		/* interrupt */
    ret
