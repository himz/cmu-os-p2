#include <syscall_int.h>

.globl vanish

vanish:
    INT  $VANISH_INT
    RET
