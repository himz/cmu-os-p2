#include <syscall_int.h>

.global set_cursor_pos

set_cursor_pos:	
	pushl	%esi				/* Save esi on stack */
	leal 	8(%esp), %esi	 	/* load esp + 8 address into esi */
	int 	$SET_CURSOR_POS_INT	/* interrupt */
	popl	%esi				/* Return from interrupt, restore save registers*/
	ret
