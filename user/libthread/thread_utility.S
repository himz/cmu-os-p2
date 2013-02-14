#include <syscall_int.h>
.global thr_int_fork_asm_wrapper

/*
 * thr_int_fork_asm_wrapper
 */
thr_int_fork_asm_wrapper:

    push %ebp
    movl 8(%esp), %eax
    movl 4(%esp), %ecx
    movl %ecx, (%eax)
    movl %esp, %edx
    movl $0x0, %ebp
    movl %eax, %esp
    int  $THREAD_FORK_INT
    cmpl $0x0, %eax
    je  .CHILD
    movl %edx, %esp
    pop  %ebp
.CHILD :
    RET
