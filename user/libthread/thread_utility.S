/** @file  thread_utility.S
 *  @brief Implements various utility functions used 
 *   by thread module when assmebly level control is needed
 *
 *  @author Ankur Sharma (ankursha)
 *
 *  @bug No bugs
 */

#include <syscall_int.h>

.global thr_int_fork_asm_wrapper
.global thr_int_exit_asm_wrapper

/*
 * thr_int_fork_asm_wrapper
 */
thr_int_fork_asm_wrapper:

    /* Push current ebp on stack */
    push %ebp 

    /* Get the stack address for new thread */ 
    movl 0x8(%esp), %eax

    /* Get the return address. */
    movl 0x4(%esp), %ecx

    /* Store the return address on new stack */
    movl %ecx, (%eax)

    /* Save the easp to edx. */ 
    movl %esp, %edx

    /* Reset ebp so that new thread gets ebp as zero. */
    movl $0x0, %ebp

    /* Copy new stack address to esp */
    movl %eax, %esp

    /* Call thread fork with the current set of registers */
    int  $THREAD_FORK_INT

    /* Check the return value. */
    cmpl $0x0, %eax

    /* Return value of zero means child context. */
    je  .CHILD

    /* I am in main thread context here.
     * restore esp from edx & ebp from stack.
     */
    movl %edx, %esp
    pop  %ebp
.CHILD :
    /* Both child & parent will return to c_wrapper */
    RET

/*
 * thr_int_exit_asm_wrapper
 */
thr_int_exit_asm_wrapper:

    /* Copy the mutex */
    movl 0x4(%esp), %ecx

    /* Copy the new stk pointer */
    movl 0x8(%esp), %eax

    /* Copy the old stk pointer */
    movl 0xc(%esp), %edx

    /* Just copy new address to esp.*/
    movl %eax, %esp
    movl %eax, %ebp
    
    /*
     * We are in new stack now.
     */

    /* Expand stack for calling new functions.*/
    sub  $0x4, %esp

    /* Save caller saved registers. */
    push %ecx
    push %edx

    /* Deallocate the page */
    call remove_pages

    /* Unlock the global mutex */
    pop %edx
    call mutex_unlock

    /*
     * Now we cann not use the reserve stack,
     * Hence just jump to vanish.
     */
    jmp vanish  
