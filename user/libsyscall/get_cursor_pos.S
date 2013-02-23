#include <syscall_int.h>

.global get_cursor_pos

get_cursor_pos:
    pushl   %esi			/* Save esi on stack */
    leal    8(%esp), %esi /* load esp + 8 address into esi */
	int 	$GET_CURSOR_POS_INT		/* interrupt */
	popl	%esi			/* Return from interrupt, restore save registers*/
	ret
