#include <syscall_int.h>

.global readline

readline:
    pushl   %esi
    lea     8(%esp), %esi
    int     $READLINE_INT
    popl	%esi
	ret
