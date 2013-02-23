/** @file  utility.S
 *  @brief Implements assembly level utility functions that can be
 *  used by whole library.
 *
 *  @author Ankur Sharma (ankursha)
 *
 *  @bug No bugs
 */

.global util_get_esp
.global util_get_ebp
.global util_get_msb

/*
 * util_get_esp, returns the value of %esp register.
 */
util_get_esp:
    movl %esp, %eax
    ret

/*
 * util_get_ebp, returns the value of %ebp register.
 */
util_get_ebp:
    movl %ebp, %eax
    ret

/*
 * Provides MSB.
 */
util_get_msb:
    /* Reset eax */
    xorl %eax, %eax

    /* Get the argument from stack */
    movl 4(%esp), %ecx

    /* Use BSR to get the msb set */
    bsr  %ecx, %eax

    ret
    
