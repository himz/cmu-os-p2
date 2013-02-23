#include <syscall_int.h>

.global set_term_color

set_term_color:
	pushl	%esi				/* Save esi on stack */
	movl 	8(%esp), %esi		/* Move address into esi */
	int 	$SET_TERM_COLOR_INT	/* interrupt */
	popl	%esi			/* Return from interrupt, restore save registers*/
	ret
