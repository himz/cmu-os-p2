#include <syscall_int.h>

.global exec

exec:
    pushl   %esi
    leal    8(%esp), %esi
    int		$EXEC_INT
	popl    %esi
    ret
