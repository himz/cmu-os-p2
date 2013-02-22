#include <syscall_int.h>
.global thr_int_fork_asm_wrapper
.global thr_int_exit_asm_wrapper

/*
 * thr_int_fork_asm_wrapper
 */
thr_int_fork_asm_wrapper:

    push %ebp 
    movl 0x8(%esp), %eax
    movl 0x4(%esp), %ecx
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

/*
 * thr_int_exit_asm_wrapper
 */
thr_int_exit_asm_wrapper:

/*
 * Copy the mutex.
 */
    movl 0x4(%esp), %ecx

/*
 * Copy the new stk pointer.
 */
    movl 0x8(%esp), %eax

/*
 * Copy the old stk pointer.
 */
    movl 0xc(%esp), %edx


/*
 * Just copy new address to esp.
 */
    movl %eax, %esp
    movl %eax, %ebp
    
/*
 * We are in new stack now.
 */

/*
 * Expand stack for calling new functions.
 */
    sub  $0x4, %esp

/*
 * Save caller saved registers.
 */
    push %ecx
    push %edx
    call remove_pages

    pop %edx
    call mutex_unlock

/*
 * Now we cann not use the reserve stack,
 * Hence just jump to vanish.
 */
    jmp vanish   
