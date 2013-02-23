#include <syscall_int.h>

.global make_runnable

make_runnable:
    pushl   %esi				/* Save esi on stack */
    movl    8(%esp), %esi		/* Move address into esi */
    int     $MAKE_RUNNABLE_INT	/* interrupt */
    popl    %esi			/* Return from interrupt, restore save registers*/
    ret
