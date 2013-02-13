.global util_get_esp

/*
 * util_get_esp, returns the value of %esp register.
 */
util_get_esp:
    movl %esp, %eax
    ret
