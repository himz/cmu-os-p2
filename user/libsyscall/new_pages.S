#include <syscall_int.h>

.globl new_pages

new_pages:
    push %esi
    movl %esp, %esi
    addl $0x8, %esi
    int  $NEW_PAGES_INT
    pop  %esi
    ret
