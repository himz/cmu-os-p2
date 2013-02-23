#include <syscall_int.h>

.global misbehave

misbehave:
	pushl	%esi			/* Save esi on stack */
	movl	8(%esp), %esi	/* Move address into esi */
	int 	$MISBEHAVE_INT	/* interrupt */
	popl	%esi			/* Return from interrupt, restore save registers*/
	ret
