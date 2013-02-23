#include <simics.h>
#include "thread_common.h"

/** @file  autostack.c
 *
 *  @brief Defines install_autostack. 
 *
 *  @bugs  No Bugs.
 *
 *  @author Ankur Kumar Sharma(ankursha)
 */


/** @brief  
 *
 *  This function is called by process before calling main.
 *
 *  @param  stack_high: Stack high of main.
 *  @param  stack_low: Stack low of main.
 *
 *  @return NA
 */

void
install_autostack(void *stack_high, void *stack_low)
{
    /*
     * Install main thread's stack_high & stack low.
     */
    thr_set_main_stackH(stack_high);
    thr_set_main_stackL(stack_low);

    thr_int_add_main_tcb(stack_high, stack_low);

    return;
}
