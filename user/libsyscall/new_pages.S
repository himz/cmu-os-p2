#include <syscall_int.h>

.globl new_pages

new_pages:
    push %esi			/* Save esi on stack */
    movl %esp, %esi		/* Move value in stack pointer into esi */
    addl $0x8, %esi		/* Add 8 to get new address, to be passed as parameter*/
    int  $NEW_PAGES_INT	/* interrupt */
    pop  %esi			/* Return from interrupt, restore save registers*/
    ret
