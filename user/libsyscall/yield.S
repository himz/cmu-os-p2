#include <syscall_int.h>

.globl yield

yield:
    INT  $YIELD_INT
    RET
