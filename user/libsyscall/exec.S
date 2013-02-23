#include <syscall_int.h>

.global exec

exec:
    pushl   %esi			/* Save esi on stack */
    leal    8(%esp), %esi	/* Move address into esi */
    int		$EXEC_INT		/* interrupt */
	popl    %esi			/* Return from interrupt, restore save registers*/
    ret