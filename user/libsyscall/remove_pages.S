#include <syscall_int.h>

.global remove_pages

remove_pages:
	pushl	%esi				/* Save esi on stack */
	movl 	8(%esp), %esi		/* Move address into esi */
	int 	$REMOVE_PAGES_INT	/* interrupt */
	popl	%esi			/* Return from interrupt, restore save registers*/
	ret
