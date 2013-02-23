#include <syscall_int.h>

.global set_status

set_status:
    pushl   %esi			/* Save esi on stack */
    movl    8(%esp), %esi	/* Move address into esi */
    int     $SET_STATUS_INT	/* interrupt */
    popl    %esi			/* Return from interrupt, restore save registers*/
    ret
