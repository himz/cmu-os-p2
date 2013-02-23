#include <syscall_int.h>

.global swexn

swexn:
    pushl 	%esi			/* Save esi on stack */
	leal	8(%esp), %esi	/* load esp + 8 address into esi */
	int		$SWEXN_INT		/* interrupt */
	popl 	%esi			/* Return from interrupt, restore save registers*/	
	ret
