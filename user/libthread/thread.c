#include <stdio.h>
#include <simics.h>
#include <syscall.h>
#include <syscall_int.h>
#include <thr_internals.h>
#include "common.h"
#include "thread_common.h"

/*
 * Global Variables.
 */
static thread_glbl_t  thread_glbl;

int 
thr_init( unsigned int size )
{
    int rc = SUCCESS;
    char *main_stack_hi = NULL;
    char *main_stack_lo = NULL;
    char *resv_stack_hi = NULL;
    char *resv_stack_lo = NULL;

    /*
     * At this stage we expect that only main thread
     * would be running hence no need for mutex protection
     * while access thread_global.
     */
   
    main_stack_hi = thr_get_main_stackH();
    main_stack_lo = thr_get_main_stackL();

    /*
     * resv stack space comes immidiately after
     * main stack space ends (including the allowed extension).
     *
     */
    resv_stack_hi = (main_stack_lo - (MAIN_STACK_EXTRA_PAGES * PAGE_SIZE));
    resv_stack_lo = resv_stack_hi - (RESV_STACK_NUM_PAGES * PAGE_SIZE);

    thr_set_resv_stackH(resv_stack_hi);
    thr_set_resv_stackL(resv_stack_lo);

    return (rc);
}

int
thr_create(void * (*func)(void *), void *arg)
{
    int rc = SUCCESS;
    char *stack_lo = NULL;
    tcb_t *new_tcb = NULL;
    tid_t new_tid = TID_NUM_INVALID;

    /*
     * Do some basic verification.
     */
    if ((!func) || (!arg)) {
        /*
         * Invalid input arguments, log the event.
         */
        lprintf("[DBG_%s], Invalid input, func: %p, arg: %p\n", 
                                      __FUNCTION__, func, arg);
        return (rc);
    }

    /*
     * Allocate stack space for child.
     */
    stack_lo = thr_int_allocate_stack(CHILD_STACK_NUM_PAGES);

    if (!stack_lo) {

        /*
         * Stack allocation failed.
         * Log & return error.
         */
        lprintf("[DBG_%s], stack allocation failed \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    new_tcb = thr_int_create_tcb();

    if (!new_tcb) {
        /*
         * We are out of memory.
         * Free up the stack space.
         * Log & return error.
         */
        thr_int_deallocate_stack(stack_lo);
        lprintf("[DBG_%s], stack allocation failed \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    new_tid = THR_TCB_GET_TID(new_tcb);

    /*
     * After this stage there is no reason for thread create to fail.
     */
    rc = thr_int_insert_tcb(new_tcb);

    if (rc != THR_SUCCESS) {
        /*
         * TCB insertion failed for some reason.
         * Log the event & return;
         */
        lprintf("[DBG_%s], TCB insertion failed with rc: %d \n", 
                                              __FUNCTION__, rc);
        rc = ERROR;
        return (rc);
    }

    /*
     * Done with all tcb handling,
     * time to spawn a new thread.
     * Just pass tcb as argument as it contains all
     * the relevant info.
     */
    thr_int_fork_c_wrapper(new_tcb);

    return (new_tid);
}

/*
 * Set/Get APIs.
 */
void 
thr_set_main_stackH(char *input) 
{
    thread_glbl.main_stack_hi = input;
    return;
}

void 
thr_set_main_stackL(char *input) 
{
    thread_glbl.main_stack_lo = input;
    return;
}

void 
thr_set_resv_stackH(char *input) 
{
    thread_glbl.resv_stack_hi = input;
    return;
}

void
thr_set_resv_stackL(char *input) 
{
    thread_glbl.resv_stack_lo = input;
    return;
}

char *
thr_get_main_stackH() 
{
    return (thread_glbl.main_stack_hi);
}

char *
thr_get_main_stackL() 
{
    return (thread_glbl.main_stack_lo);
}

char *
thr_get_resv_stackH() 
{
    return (thread_glbl.resv_stack_hi);
}

char *
thr_get_resv_stackL() 
{
    return (thread_glbl.resv_stack_lo);
}

/*
 * ====================================
 * Thread specifc internal APIS.
 * ====================================
 */
char *
thr_int_allocate_stack(int stack_size)
{
    char *stack_ptr = NULL;

    /*
     * As of now return a hardcoded value.
     */
    stack_ptr = (thr_get_resv_stackL() - (WSIZE));

    /*
     * TODO: Add proper code.
     */
    return (stack_ptr);
}

void
thr_int_deallocate_stack(char *base)
{

    if (!base)
        return;

    /*
     * TODO: Add proper code.
     */

    return;
}

void
thr_int_fork_c_wrapper(tcb_t *new_tcb)
{
    /*
     * TODO: Add proper code.
     */
    return; 
}

tcb_t *
thr_int_create_tcb()
{
    /*
     * TODO: Add proper code.
     */
    return (NULL);
}

int
thr_int_insert_tcb(tcb_t *tcb)
{
    int rc = THR_SUCCESS;
    /*
     * TODO: Add proper code.
     */
    return (rc);
}
