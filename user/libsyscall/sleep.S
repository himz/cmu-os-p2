#include <syscall_int.h>

.globl sleep

sleep:
    push %esi			/* Save esi on stack */
    movl 8(%esp), %esi 	/* Move address into esi */
    int  $SLEEP_INT		/* interrupt */
    pop  %esi		/* Return from interrupt, restore save registers*/
    ret
