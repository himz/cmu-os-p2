#include <syscall_int.h>

.global set_cursor_pos

set_cursor_pos:
	pushl	%esi
	leal 	8(%esp), %esi	/* put package address in %esi */
	int 	$SET_CURSOR_POS_INT
	popl	%esi
	ret
