#include <stdio.h>
#include <simics.h>
#include <syscall.h>
#include <syscall_int.h>
#include <thr_internals.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <mutex.h>
#include "common.h"
#include "thread_common.h"
#include "util.h"
#include "skip_list_common.h"

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
    char *resv_stack_hi = NULL;
    char *resv_stack_lo = NULL;

    unsigned int child_stack_size = 0;
    mutex_t *mutex = NULL;
    skip_list_global_t *skip_list = NULL;;

    //lprintf("[DBG_%s], Enter\n", __FUNCTION__);

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

    //lprintf("[DBG_%s], main_stack_hi: %p, main_stack_lo: %p \n", 
    //                __FUNCTION__, main_stack_hi, main_stack_lo);

    skip_list = THR_GLB_GET_SKPLST_PTR((&thread_glbl));

    /*
     * Calculate child stack size.
     * This size shoudl be scaled to integral number of
     * pages needed to accomodate requested input size.
     */
    child_stack_size = ((inp_size / PAGE_SIZE) * PAGE_SIZE) + 
                        ((inp_size % PAGE_SIZE) ? PAGE_SIZE : 0);

    THR_GLB_SET_TSSIZE((&thread_glbl), child_stack_size);

    //lprintf("[DBG_%s], child_stack_size: %u\n", 
    //            __FUNCTION__, child_stack_size);
    //
    resv_stack_hi = main_stack_lo - 
                    (MAIN_STACK_EXTRA_PAGES * PAGE_SIZE) 
                    - WSIZE;
    resv_stack_lo = (resv_stack_hi - child_stack_size + WSIZE);

    THR_GLB_SET_RSTKH((&thread_glbl), resv_stack_hi);
    THR_GLB_SET_RSTKL((&thread_glbl), resv_stack_lo);

    /*
     * Allocate this reserve stack.
     * Size of resv stack is 1 page as of now.
     */
    new_pages(resv_stack_lo, (PAGE_SIZE * RESV_STACK_NUM_PAGES));

#if 0
    free_stack_hi = main_stack_lo - 
                    (MAIN_STACK_EXTRA_PAGES * PAGE_SIZE) 
                    - WSIZE;
    free_stack_lo = (free_stack_hi - child_stack_size + WSIZE);
#endif

    free_stack_hi = resv_stack_lo - WSIZE;
    free_stack_lo = (free_stack_hi - child_stack_size + WSIZE);

    THR_GLB_SET_FSTKH((&thread_glbl), free_stack_hi);
    THR_GLB_SET_FSTKL((&thread_glbl), free_stack_lo);

    mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);
    mutex_init(mutex);

    /*
     * TODO: Change it.
     */
    skip_list_init(skip_list, 0, 0);
    
    lprintf("[DBG_%s], resv_stack_hi: %p, resv_stack_lo: %p \n", 
                    __FUNCTION__, resv_stack_hi, resv_stack_lo);

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
    mutex_t *mutex = NULL;

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

    mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);
    mutex_lock(mutex);

    new_tcb = thr_int_create_tcb(stack_hi, stack_lo, func, arg);
    if (!new_tcb) {
        /*
         * We are out of memory.
         * Log & return error.
         */
        mutex_unlock(mutex);
        lprintf("[DBG_%s], stack allocation failed \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    /*
     * Allocate stack space for child.
     */
    stack_lo = thr_int_allocate_stack(child_stack_size, new_tcb);
    if (!stack_lo) {

        /*
         * Stack allocation failed.
         * Log & return error.
         */
        mutex_unlock(mutex);
        lprintf("[DBG_%s], stack allocation failed \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    stack_hi = (stack_lo + child_stack_size - WSIZE);

    THR_TCB_SET_STKH(new_tcb, stack_hi);
    THR_TCB_SET_STKL(new_tcb, stack_lo);

    lprintf("[DBG_%s], stack_hi: %p, stack_lo: %p, cstack_size: %u \n", __FUNCTION__, stack_hi, stack_lo, child_stack_size);

    new_tid = THR_TCB_GET_TID(new_tcb);

    /*
     * After following stage there is no reason for thread create to fail.
     */
    rc = thr_int_insert_tcb(new_tcb);

    if (rc != THR_SUCCESS) {

        /*
         * TCB insertion failed for some reason.
         * Log the event & return;
         */
        mutex_unlock(mutex);
        lprintf("[DBG_%s], TCB insertion failed with rc: %d \n", 
                                              __FUNCTION__, rc);
        rc = ERROR;
        return (rc);
    }

    mutex_unlock(mutex);

    /*
     * Done with all tcb handling,
     * time to spawn a new thread.
     * Just pass tcb as argument as it contains all
     * the relevant info.
     */
    thr_int_fork_c_wrapper(&new_tcb);

    /*
     * TODO: Insert KERN TID in data structure.
     */

    return (new_tid);
}

void
thr_exit(void *status)
{
    char *stack_lo = NULL;
    char *curr_stack_ptr = NULL;
    char *resv_stack_hi = NULL;
    unsigned int stack_lo_mask = 0;
    tcb_t *tcb = NULL;
    tid_t tid = TID_NUM_INVALID;
    boolean_t is_joinable = FALSE;
    mutex_t *mutex = NULL;
    
    curr_stack_ptr = util_get_esp();
    stack_lo_mask = (~(PAGE_SIZE - 1));
    stack_lo = (char *)(((unsigned int)curr_stack_ptr) & stack_lo_mask);
    
    lprintf("[DBG_%s], stack_lo: %p \n", __FUNCTION__, stack_lo);
    
    mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);
    resv_stack_hi = THR_GLB_GET_RSTKH(&thread_glbl);
    mutex_lock(mutex); 

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


    } else {

        tid = THR_TCB_GET_TID(tcb);
        is_joinable = THR_TCB_GET_JOIN(tcb);

    }

    if (is_joinable == TRUE) {

        /*
         * Thread is joinable, let the joining thread do the
         * cleanup.
         */
        vanish();
    }

    thr_int_deallocate_tid(tid);
    thr_int_deallocate_stack(stack_lo);


    /*
     * TODO: do it in assembly.
     */
#if 0
    /*
     * Clean up the tid.
     */

        /*
     * Mutex unlock.
     */
    mutex_unlock();

    /*
     * No other resources should be released.
     */
    vanish();
#endif
    thr_int_exit_asm_wrapper(mutex, resv_stack_hi, stack_lo);

    return;
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
thr_int_allocate_stack(int stack_size, void *list_data)
{
    char *stack_ptr = NULL;
    int rc = 0;
    thread_reuse_stack_t *reuse_stack = NULL;
    char *new_free_stack_lo = NULL;
    char *new_free_stack_hi = NULL;
    unsigned int child_stack_size = THR_GLB_GET_TSSIZE(&thread_glbl);
    skip_list_global_t *skip_list = NULL;

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

        /*
         * TODO: Add code here.
         */

    }

    /*
     * Allocate the page.
     */
    rc = new_pages(stack_ptr, stack_size);

    if (rc < 0) {
        /*
         * Page allocation failed.
         */
        lprintf("[DBG_%s], Page allocation failed with rc: %d \n",
                                                 __FUNCTION__, rc);
        stack_ptr = NULL;
    }

    skip_list = THR_GLB_GET_SKPLST_PTR((&thread_glbl));

    /*
     * TODO: change the bucket index.
     */
    skip_list_insert(skip_list, 0, ((uint32_t)(stack_ptr)), list_data);

    return (stack_ptr);
}

void
thr_int_deallocate_stack(char *base)
{
    skip_list_global_t *skip_list = NULL;

    lprintf("[DBG_%s], Inside deallocate stack \n", __FUNCTION__);

    if (!base)
        return;

    skip_list = THR_GLB_GET_SKPLST_PTR((&thread_glbl));

    /*
     * Remove the node from skip list..
     */

    /*
     * TODO: change the bucket index.
     */
    skip_list_remove(skip_list, 0, ((uint32_t)base));

    /*
     * Deallocate does not remove the page, as it could be called
     * in the same thread's context.
     */

#if 0
    /*
     * Free up this page.
     */
    remove_pages(base);
#endif

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
        //lprintf("[DBG_%s], in child conext, sp: %p\n", __FUNCTION__, temp_sp);
        //lprintf("[DBG_%s], I am ok till here !!!\n", __FUNCTION__);

        /*
         * Time to call the child function.
         */
        child_rc = (*func) (args);


        /*
         * Reaching here means that thread is done w/o calling thread_exit.
         * thread exit should never return.
         */
        thr_exit(NULL);
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
    tid_t new_tid = THR_GLB_GET_NEXT_TID(&(thread_glbl));

    THR_GLB_INC_NEXT_TID(&(thread_glbl));

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
    tcb_t *tcb = NULL;
    skip_list_global_t *skip_list = NULL;

    skip_list = THR_GLB_GET_SKPLST_PTR(&(thread_glbl));

    if (!skip_list)
        return NULL;

    tcb = skip_list_find(skip_list, 0, ((uint32_t)(stack_lo)));

    return (tcb);
}
