#include <syscall_int.h>

.global readfile

readfile:
    pushl   %esi
    leal    8(%esp), %esi
    int     $READFILE_INT
    popl	%esi
	ret
