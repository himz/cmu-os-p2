#include <syscall_int.h>

.global task_vanish

task_vanish:	
	pushl	%esi				/* Save esi on stack */
	movl	8(%esp), %esi		/* Move address into esi */
	int 	$TASK_VANISH_INT	/* interrupt */
	popl	%esi			/* Return from interrupt, restore save registers*/
	ret
