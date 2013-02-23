#include <syscall_int.h>

.global print

print:
    pushl   %esi			/* Save esi on stack */
    leal	8(%esp), %esi 	/* load esp + 8  into esi */	
	int     $PRINT_INT		/* interrupt */
    popl	%esi			/* Return from interrupt, restore save registers*/
	ret
