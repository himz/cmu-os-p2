#include <syscall_int.h>

.global swexn

swexn:
    pushl 	%esi
	leal	8(%esp), %esi
	int		$SWEXN_INT
	popl 	%esi
	ret
