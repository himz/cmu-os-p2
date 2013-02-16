#include <stdio.h>
#include <simics.h>
#include <syscall.h>
#include <syscall_int.h>
#include <thr_internals.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "common.h"
#include "thread_common.h"
#include "util.h"

/*
 * Global Variables.
 */
static thread_glbl_t  thread_glbl;

int 
thr_init( unsigned int inp_size )
{
    int rc = SUCCESS;
    char *main_stack_hi = NULL;
    char *main_stack_lo = NULL;
    char *free_stack_hi = NULL;
    char *free_stack_lo = NULL;
    unsigned int child_stack_size = 0;

    lprintf("[DBG_%s], Enter\n", __FUNCTION__);

    /*
     * At this stage we expect that only main thread
     * would be running hence no need for mutex protection
     * while access thread_global.
     */
   
    main_stack_hi = thr_get_main_stackH();
    main_stack_lo = thr_get_main_stackL();

    if (!(main_stack_hi) || (!main_stack_lo)) {
        /*
         * Should not have happened, log it & return.
         */
        lprintf("[DBG_%s], PANIC: stack_hi: %p or stack_lo: %p "
                "invalid \n", __FUNCTION__, main_stack_hi, main_stack_lo);

        rc = ERROR;
        return (rc);
    }

    lprintf("[DBG_%s], main_stack_hi: %p, main_stack_lo: %p \n", 
                    __FUNCTION__, main_stack_hi, main_stack_lo);

    /*
     * Calculate child stack size.
     * This size shoudl be scaled to integral number of
     * pages needed to accomodate requested input size.
     */
    child_stack_size = ((inp_size / PAGE_SIZE) * PAGE_SIZE) + 
                        ((inp_size % PAGE_SIZE) ? PAGE_SIZE : 0);

    THR_GLB_SET_TSSIZE((&thread_glbl), child_stack_size);

    lprintf("[DBG_%s], child_stack_size: %u\n", 
                __FUNCTION__, child_stack_size);

    free_stack_hi = main_stack_lo - 
                    (MAIN_STACK_EXTRA_PAGES * PAGE_SIZE) 
                    - WSIZE;
    free_stack_lo = (free_stack_hi - child_stack_size + WSIZE);

    THR_GLB_SET_FSTKH((&thread_glbl), free_stack_hi);
    THR_GLB_SET_FSTKL((&thread_glbl), free_stack_lo);

    lprintf("[DBG_%s], free_stack_hi: %p, free_stack_lo: %p \n", 
                    __FUNCTION__, free_stack_hi, free_stack_lo);


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
    unsigned int child_stack_size = 0;

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

    child_stack_size = THR_GLB_GET_TSSIZE(&thread_glbl);

    /*
     * TODO: Take STACK Lock.
     */

    /*
     * Allocate stack space for child.
     */
    stack_lo = thr_int_allocate_stack(child_stack_size);

    /*
     * TODO: Release STACK Lock.
     */

    if (!stack_lo) {

        /*
         * Stack allocation failed.
         * Log & return error.
         */
        lprintf("[DBG_%s], stack allocation failed \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    stack_hi = (stack_lo + child_stack_size - WSIZE);

    lprintf("[DBG_%s], stack_hi: %p, stack_lo: %p, cstack_size: %u \n", __FUNCTION__, stack_hi, stack_lo, child_stack_size);

    /*
     * TODO: Take TCB Lock.
     */
    new_tcb = thr_int_create_tcb(stack_hi, stack_lo, func, arg);
    /*
     * TCB: Relase TCB Lock.
     */

    if (!new_tcb) {
        /*
         * We are out of memory.
         * Free up the stack space.
         * Log & return error.
         */

        /*
         * TODO: Take Stack Lock.
         */
        thr_int_deallocate_stack(stack_lo);
        /*
         * TODO: Release Stack Lock.
         */
        lprintf("[DBG_%s], stack allocation failed \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    new_tid = THR_TCB_GET_TID(new_tcb);

    /*
     * After following stage there is no reason for thread create to fail.
     */
    /*
     * TODO: Take TCB Lock.
     */
    rc = thr_int_insert_tcb(new_tcb);
    /*
     * TODO: Release TCB Lock.
     */

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
    thr_int_fork_c_wrapper(&new_tcb);

    /*
     *
     * TODO: Insert KERN TID in data structure.
     */

    return (new_tid);
}

void
thr_exit(void *status)
{
    char *stack_lo = NULL;
    char *curr_stack_ptr = NULL;
    unsigned int stack_lo_mask = 0;
    tcb_t *tcb = NULL;
    tid_t tid = TID_NUM_INVALID;
    boolean_t is_joinable = FALSE;
    
    curr_stack_ptr = util_get_esp();
    stack_lo_mask = (~(PAGE_SIZE - 1));
    stack_lo = (char *)(((unsigned int)curr_stack_ptr) & stack_lo_mask);
    
    lprintf("[DBG_%s], stack_lo: %p \n", __FUNCTION__, stack_lo); 

    /*
     * TODO: Acquire TCB lock.
     */
    /*
     * Search TCB.
     */
    tcb = thr_int_search_tcb_by_stk(stack_lo);

    if (!tcb) {
        /*
         * Search failed.
         * Data structure corruption.
         * Log the event but the thread should be killed anyways.
         */
        lprintf("[DBG_%s], search based on stk: %p failed \n", 
                                      __FUNCTION__, stack_lo);
        /*
         * TODO: Release TCB lock.
         */

    } else {

        tid = THR_TCB_GET_TID(tcb);
        is_joinable = THR_TCB_GET_JOIN(tcb);

        /*
         * TODO: Release TCB lock.
         */
    }

    if (is_joinable == TRUE) {
        /*
         * Thread is joinable, let the joining thread do the
         * cleanup.
         */
        vanish();
    }

    /*
     * Clean up the tid.
     */
    /*
     * TODO: Take TID Lock.
     */
    /*
     * TODO: Take Stack Lock.
     */
    thr_int_deallocate_tid(tid);
    thr_int_deallocate_stack(stack_lo);
    /*
     * TODO: Release TID Lock.
     */
    /*
     * TODO: Release STACK Lock.
     */

    /*
     * No other resources should be released.
     */
    vanish();
}


/*
 * Set/Get APIs which will be called by other files.
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
    thread_reuse_stack_t *reuse_stack = NULL;
    char *new_free_stack_lo = NULL;
    char *new_free_stack_hi = NULL;
    unsigned int child_stack_size = THR_GLB_GET_TSSIZE(&thread_glbl);

    reuse_stack = THR_GLB_GET_RSTACK(&thread_glbl);

    if (!reuse_stack) {

        stack_ptr = THR_GLB_GET_FSTKL(&thread_glbl);

        /*
         * OK we are assigning from free pool (not 
         * from reusable pool), update free_stack_hi
         * & free_stack_lo to the new values.
         */
        new_free_stack_hi = stack_ptr - WSIZE;
        new_free_stack_lo = (new_free_stack_hi - child_stack_size + WSIZE);

        THR_GLB_SET_FSTKH(&thread_glbl, new_free_stack_hi);
        THR_GLB_SET_FSTKL(&thread_glbl, new_free_stack_lo);

    } else {
#if 0

        /*
         * Pick one from reusable stack space.
         */
        stack_ptr = thr_int_get_reuse_node(&reuse_stack);

        if (!stack_ptr) {
            /*
             * Reuse node linked list is corrupted.
             * Log the event & return NULL.
             */
            lprintf("[DBG_%s], stack_ptr is NULL in"
                    " reuse linked list\n", __FUNCTION__);

            return (NULL);
        }
#endif
    }

    /*
     * Allocate the page.
     */
    lprintf("[DBG_%s], Allocate page at %p\n", __FUNCTION__, stack_ptr);
    rc = new_pages(stack_ptr, stack_size);

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
thr_int_fork_c_wrapper(tcb_t **input_tcb)
{
    tid_t child_tid = 0;
    char *child_stack_hi = NULL;
    tcb_t *new_tcb = *input_tcb;
#if 0
    char *temp_sp = NULL;
    char *temp_bp = NULL;
#endif

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

    child_tid = thr_int_fork_asm_wrapper(child_stack_hi - WSIZE);

    if (child_tid) {
        /*
         * Inside Parent.
         */
#if 0
        temp_sp = util_get_esp();
        temp_bp = util_get_ebp();
        lprintf("[DBG_%s], in Parent conext, sp: %p, bp: %p\n", __FUNCTION__, temp_sp, temp_bp);
#endif

    } else {

        /*
         * Inside child context.
         * Lets run the registered function.
         */
#if 0
        temp_sp = util_get_esp();
        temp_bp = util_get_ebp();
#endif
        func = THR_TCB_GET_FUNC(new_tcb);
        args = THR_TCB_GET_ARGS(new_tcb);
#if 0
        lprintf("[DBG_%s], in child conext, sp: %p, bp: %p\n", __FUNCTION__, temp_sp, temp_bp);
#endif

        /*
         * Time to call the child function.
         */
        child_rc = (*func) (args);

        /*
         * Reaching here means that thread is done w/o calling thread_exit.
         * thread exit should never return.
         */
        thr_exit(NULL);

        lprintf("[DBG_%s], What am i doing here !!!\n", __FUNCTION__);
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

#if 0
char *
thr_int_get_reuse_node(thread_reuse_stack_t **input)
{
    char *ret_ptr = NULL;
    thread_reuse_stack_t *reuse_node_head = *input;

    if (!reuse_node_head) {

        /*
         * Why am i being called with NULL header.
         * Should not have happened, log it & return NULL.
         */
        lprintf("[DBG_%s], ERR: function called"
               " with NULL header\n", __FUNCTION__);

        return (NULL);
    }

    ret_ptr = reuse_head_node->stack_lo;

    /*
     * Free up the node & move head pointer fwd.
     */
    THR_GLB_SET_RSTACK(&thread_glbl, reuse_head_node->head);
    free(*input);

    return (ret_ptr);
}
#endif
tcb_t *
thr_int_search_tcb_by_stk(char *stack_lo)
{
    /*
     * TODO: Add code here.
     */
    return (NULL);
}
