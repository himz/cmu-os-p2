#include <syscall_int.h>

.globl get_ticks

get_ticks:
    INT  $GET_TICKS_INT		/* interrupt */
    ret
