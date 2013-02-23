#include <syscall_int.h>

.globl fork

fork:
    INT  $FORK_INT	/* interrupt */
    ret
