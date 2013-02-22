#include <simics.h>
#include "thread_common.h"

/* If you want to use assembly language instead of C,
 * delete this autostack.c and provide an autostack.S
 * instead.
 */

void
install_autostack(void *stack_high, void *stack_low)
{
    /*
     * Install main thread's stack_high & stack low.
     */
    lprintf("[DBG_%s], Parent stack, high: %p, lo: %p\n", __FUNCTION__, stack_high, stack_low);
    thr_set_main_stackH(stack_high);
    thr_set_main_stackL(stack_low);

    thr_int_add_main_tcb(stack_high, stack_low);

    return;
}
