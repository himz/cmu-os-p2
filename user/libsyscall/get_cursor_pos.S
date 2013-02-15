#include <syscall_int.h>

.global get_cursor_pos

get_cursor_pos:
    pushl   %esi
    leal    8(%esp), %esi
	int 	$GET_CURSOR_POS_INT
	popl	%esi
	ret
