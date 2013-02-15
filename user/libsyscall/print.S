#include <syscall_int.h>

.global print

print:
    pushl   %esi
    leal	8(%esp), %esi 		/* %esi stores the address of arguments package */
	int     $PRINT_INT
    popl	%esi
	ret
