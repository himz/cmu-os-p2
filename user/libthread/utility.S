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
    xorl %eax, %eax
    movl 4(%esp), %ecx
    bsr  %ecx, %eax
    ret
    
