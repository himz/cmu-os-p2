#include <syscall_int.h>

.global readline

readline:
    pushl   %esi
    leal    8(%esp), %esi
    int     $READLINE_INT
    popl	%esi
	ret
