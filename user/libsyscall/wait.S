#include <syscall_int.h>

.global wait

wait:
    pushl   %esi			/* Save esi on stack */
    movl    8(%esp), %esi	/* Move address into esi */
    int     $WAIT_INT		/* interrupt */
    popl    %esi			/* Return from interrupt, restore save registers*/
    ret
