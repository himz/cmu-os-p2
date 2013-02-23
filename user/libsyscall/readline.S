#include <syscall_int.h>

.global readline

readline:
    pushl   %esi			/* Save esi on stack */
    leal    8(%esp), %esi	/* load esp + 8 address into esi */
    int     $READLINE_INT	/* interrupt */
    popl	%esi			/* Return from interrupt, restore save registers*/
	ret
