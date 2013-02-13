#include <syscall_int.h>

.globl fork

/*
 * kernel_timer_handler, it is the handler where any timer interrupt
 * leads the control to. Thiis function saves the relevant registers
 * & calls the corresponding c handler.
 */
fork:
    INT  $FORK_INT
    RET
