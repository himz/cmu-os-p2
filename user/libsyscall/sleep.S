#include <syscall_int.h>

.globl sleep

sleep:
    push %esi
    movl 8(%esp), %esi 
    int  $SLEEP_INT
    pop  %esi
    ret
