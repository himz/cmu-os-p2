#include <stdio.h>
#include <simics.h>
#include <syscall.h>
#include <syscall_int.h>
#include <thr_internals.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "thread_common.h"
#include "util.h"

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

    lprintf("[DBG_%s], Enter\n", __FUNCTION__);

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
    char *stack_hi = NULL;
    tcb_t *new_tcb = NULL;
    tid_t new_tid = TID_NUM_INVALID;

    lprintf("[DBG_%s], Enter\n", __FUNCTION__);

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

    stack_hi = (stack_lo + PAGE_SIZE - WSIZE);
    new_tcb = thr_int_create_tcb(stack_hi, stack_lo, func, arg);

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
    int rc = 0;

    /*
     * As of now return a hardcoded value.
     */
    stack_ptr = (thr_get_resv_stackL());

    /*
     * TODO: Add proper code.
     */
    
    /*
     * Allocate the page.
     */
    lprintf("[DBG_%s], Allocate page at %p\n", __FUNCTION__, stack_ptr);
    rc = new_pages(stack_ptr, (CHILD_STACK_NUM_PAGES * PAGE_SIZE));

    if (rc < 0) {
        /*
         * Page allocation failed.
         */
        lprintf("[DBG_%s], Page allocation failed with rc: %d \n",
                                                 __FUNCTION__, rc);

        stack_ptr = NULL;
    }

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
    tid_t child_tid = 0;
    char *child_stack_hi = NULL;
    char *temp_sp = NULL;
    char *temp_bp = NULL;

    /*
     * Child thread's callback.
     */
    void * (*func)(void *) = NULL;
    void *args = NULL;
    void *child_rc = NULL;

    if (!new_tcb) {
        /*
         * Should not have happened.
         */
        return;
    }

    child_stack_hi = THR_TCB_GET_STKH(new_tcb);
    lprintf("[DBG_%s], Child stack_hi: %p \n", __FUNCTION__, child_stack_hi);

    child_tid = thr_int_fork_asm_wrapper(child_stack_hi);

    if (child_tid) {
        /*
         * Inside Parent.
         */
        temp_sp = util_get_esp();
        temp_bp = util_get_ebp();
        lprintf("[DBG_%s], in Parent conext, sp: %p, bp: %p\n", __FUNCTION__, temp_sp, temp_bp);

    } else {

        /*
         * Inside child context.
         * Lets run the registered function.
         */
        temp_sp = util_get_esp();
        temp_bp = util_get_ebp();
        func = THR_TCB_GET_FUNC(new_tcb);
        args = THR_TCB_GET_ARGS(new_tcb);
        lprintf("[DBG_%s], in child conext, func: %p, arg: %p\n", __FUNCTION__, func, args);

        /*
         * Time to call the child function.
         */

        child_rc = (*func) (args);
    }

    return; 
}

tcb_t *
thr_int_create_tcb(char *stack_hi, char *stack_lo, 
                   void * (*func)(void *), void *arg)
{
    tcb_t *new_tcb = NULL;
    tid_t new_tid = TID_NUM_INVALID;

    new_tid = thr_int_allocate_new_tid();
    new_tcb = malloc(sizeof(tcb_t));
    memset(new_tcb, 0, sizeof(tcb_t));

    /*
     * Set TCB parameters.
     */
    THR_TCB_SET_TID(new_tcb, new_tid);
    THR_TCB_SET_STKH(new_tcb, stack_hi);
    THR_TCB_SET_STKL(new_tcb, stack_lo);
    THR_TCB_SET_FUN(new_tcb, func);
    THR_TCB_SET_ARG(new_tcb, arg);

    return (new_tcb);
}

int
thr_int_insert_tcb(tcb_t *tcb)
{
    int rc = THR_SUCCESS;
    return (rc);
}

tid_t 
thr_int_allocate_new_tid()
{
    tid_t new_tid = 1;

    /*
     * TODO: Add proper code.
     */
    return (new_tid);
}

void 
thr_int_deallocate_tid(tid_t tid)
{
    /*
     * TODO: Add proper code.
     */
    return;
}
