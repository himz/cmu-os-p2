/** @file  thread.c
 *
 *  @brief Main implementation of thread APIs.
 *
 *  This file imeplements all the APIS (internal/external) of thread module.
 *
 *  @bugs refer to docx file for design related issues.
 *
 *  @author Ankur Kumar Sharma(ankursha)
 */

#include <stdio.h>
#include <simics.h>
#include <syscall.h>
#include <syscall_int.h>
#include <thr_internals.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <rand.h>
#include <mutex.h>
#include <ureg.h>
#include <syscall.h>
#include "common.h"
#include "thread_common.h"
#include "util.h"
#include "skip_list_common.h"

/*
 * Global Variables.
 */
static thread_glbl_t  thread_glbl;

/*
 * ======================================
 * Application exposed functions.
 * ======================================
 */

/** @brief  Initializes the thread library.
 *
 *  This function does following
 *  a. Calculates the stack space needed by each child thread.
 *  b. Genrated bucket key mas based on stack size for tcb 
 *     distribution in skip list.
 *  c. Allocate reserve stack.
 *  d. Initialize global mutexes.
 *  e. Initialize skip list
 *
 *  @param  inp_size: Stack space needed by each thread.
 *
 *  @return ERROR if anythign wrong, SUCCESS otherwise.
 */
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
    int msb_pos = 0;
    uint32_t bucket_key_mask = ~(0x0);
    int temp = 0;
    unsigned int child_stack_size = 0;
    mutex_t *mutex = NULL;
    mutex_t *mem_mutex = NULL;
    skip_list_global_t *skip_list = NULL;

    skip_list = THR_GLB_GET_SKPLST_PTR((&thread_glbl));

    main_stack_hi = thr_get_main_stackH();
    main_stack_lo = thr_get_main_stackL();

    /*
     * Calculate child stack size.
     * This size shoudl be scaled to integral number of
     * pages needed to accomodate requested input size.
     */
    child_stack_size = ((inp_size / PAGE_SIZE) * PAGE_SIZE) + 
                        ((inp_size % PAGE_SIZE) ? PAGE_SIZE : 0);

    THR_GLB_SET_TSSIZE((&thread_glbl), child_stack_size);

    msb_pos = util_get_msb((void *)(child_stack_size));
    temp = util_get_msb((void *)(~0x0));

    /*
     * Mask generation algo.
     * A single page consumes 12 bits, now say a thread uses 2 pages
     * then additional 1 bit will be used, as a result 32 - (12 + 1)
     * these many bits will be availabe for any thread allocated stack
     * to begin/end. We'll divide these bits in half & use 1 half as 
     * bucket key. I.e say if we are left with 20 bits then 10 bits will 
     * indicate bucket key, we'll have 1024 buckets, where each bucket can 
     * hold 1024 tcbs.
     */
    temp = (((temp - msb_pos)/2) + msb_pos);
    bucket_key_mask = ((~0x0) << (temp));
    THR_GLB_SET_BKT_KEY_MASK((&(thread_glbl)), bucket_key_mask);

    resv_stack_hi = main_stack_lo - 
                    (MAIN_STACK_EXTRA_PAGES * PAGE_SIZE) 
                    - WSIZE;
    resv_stack_lo = (resv_stack_hi - PAGE_SIZE + WSIZE);

    THR_GLB_SET_RSTKH((&thread_glbl), resv_stack_hi);
    THR_GLB_SET_RSTKL((&thread_glbl), resv_stack_lo);

    /*
     * Allocate this reserve stack.
     * Size of resv stack is 1 page as of now.
     */
    new_pages(resv_stack_lo, (PAGE_SIZE * RESV_STACK_NUM_PAGES));

    free_stack_hi = resv_stack_lo - WSIZE;
    free_stack_lo = (free_stack_hi - child_stack_size + WSIZE);

    THR_GLB_SET_FSTKH((&thread_glbl), free_stack_hi);
    THR_GLB_SET_FSTKL((&thread_glbl), free_stack_lo);

    /*
     * Initialize global mutexes.
     */
    mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);
    mutex_init(mutex);

    /*
     * Initialize mytex related to thread safe malloc.
     */
    mem_mutex = THR_GLB_GET_MEM_MUTEX_PTR((&thread_glbl));
    mutex_init(mem_mutex);

    THR_GLB_SET_IS_MULTI_THR((&thread_glbl), TRUE);

    /*
     * we need not woryr about per node count for our currrent 
     * implementation, hence initialize it as zero.
     */
    skip_list_init(skip_list, 0, 0);
    return (rc);
}

/** @brief  Spwans a new thread.
 *
 *  This function does following
 *  a. Creates TCB.
 *  b. Allocates stack space.
 *  c. Allocates TID.
 *  d. Calls the c_wrapper for spawning new thread.
 *
 *  @param  func: Thread callback.
 *  @param  arg: Thread callback argument.
 *
 *  @return ERROR if anythign wrong, SUCCESS otherwise.
 */
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

    /*
     * Do some basic verification.
     */
    if ((!func)) {
        /*
         * Invalid input arguments, log the event.
         */
        lprintf("[DBG_%s], Invalid input, func: %p\n", 
                                  __FUNCTION__, func);
        rc = ERROR;
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
    new_tid = thr_int_allocate_new_tid(stack_lo);

    /*
     * Setup the stack parameters.
     */
    THR_TCB_SET_TID(new_tcb, new_tid);
    THR_TCB_SET_STKH(new_tcb, stack_hi);
    THR_TCB_SET_STKL(new_tcb, stack_lo);

    /*
     * Done with all tcb handling,
     * time to spawn a new thread.
     * Just pass tcb as argument as it contains all
     * the relevant info.
     */
    thr_int_fork_c_wrapper(&new_tcb);

    mutex_unlock(mutex);

    return (new_tid);
}

/** @brief  Exits the thread.
 *
 *  This function does following
 *  a. Deallocates TCb
 *  b. Deallocates stack space.
 *  c. Calls the c_wrapper for vanishing the current thread
 *     by moving to reserve stack space.
 *
 *  @param  status: User input to be passed to joining thread.
 *
 *  @return NA.
 */
void
thr_exit(void *status)
{
    char *stack_lo = NULL;
    char *curr_stack_ptr = NULL;
    char *resv_stack_hi = NULL;
    unsigned int stack_lo_mask = 0;
    tcb_t *my_tcb = NULL;
    tcb_t *wait_tcb = NULL;
    mutex_t *glb_mutex = NULL;
    mutex_t *wait_mutex = NULL;
   
    curr_stack_ptr = util_get_esp();
    stack_lo_mask = (~(PAGE_SIZE - 1));
    stack_lo = (char *)(((unsigned int)curr_stack_ptr) & stack_lo_mask);

    glb_mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);
    resv_stack_hi = THR_GLB_GET_RSTKH(&thread_glbl);

    mutex_lock(glb_mutex);
     
    /*
     * Search TCB.
     */
    my_tcb = thr_int_search_tcb_by_stk(stack_lo);

    if (!my_tcb) {
        /*
         * Search failed.
         * Data structure corruption.
         * Log the event but the thread should be killed anyways.
         */
        lprintf("[DBG_%s], search based on stk: %p failed \n", 
                                      __FUNCTION__, stack_lo);
        mutex_unlock(glb_mutex);
        vanish();
    }

    stack_lo = THR_TCB_GET_STKL(my_tcb);

    /*
     * Get the tcb of the thread waiting on me.
     */
    wait_tcb = THR_TCB_GET_WAIT_TCB(my_tcb);

    if (wait_tcb) {

        /*
         * Wake up the waiting thread.
         */
        wait_mutex = THR_TCB_GET_MUTEX_PTR(wait_tcb);

        /*
         * Provide the return value.
         */
        THR_TCB_SET_RET_VAL(wait_tcb, status);
        mutex_unlock(wait_mutex);

    } else {

        /*
         * No one is waiting for me, put myself to zombie list.
         */
        tcb_int_push_zombie_thread(my_tcb, status);
    } 

    /*
     * Remove the stack address from the s/w image.
     */
    thr_int_deallocate_stack(stack_lo);

    /*
     * Free up the TCB memory.
     */
    free(my_tcb->tcb_excp_stack_lo);
    free(my_tcb);
    my_tcb= NULL;

    /*
     * Do the stack cleanup & mutex unlock in assembly.
     */
    thr_int_exit_asm_wrapper(glb_mutex, resv_stack_hi, stack_lo);

    return;
}

/** @brief  Makes the calling thread join the input tid.
 *
 *  This function does following
 *  a. Validate if operations is valid.
 *  b. Waits on target tcb (if its active).
 *  c. Returns the exit status in statusp.
 *
 *  @param  tid: Thread on which caller wants to join.
 *  @param  statusp: Pointer to memory lcoation where 
 *                   we need to put thr_exit status.
 *
 *  @return ERROR if validation fails, SUCCESS otherwise.
 */
int
thr_join(int tid, void **statusp)
{
    tcb_t *thr_tcb = NULL;
    tcb_t *my_tcb = NULL;
    mutex_t *glb_mutex = NULL;
    mutex_t *self_mutex = NULL;
    tid_t thr_tid = 0;
    char *stack_lo = NULL;
    char *curr_stack_lo = NULL;
    unsigned int stack_lo_mask = 0;
    tcb_zombie_t *zombie_tcb = NULL;
    int rc = SUCCESS;

    stack_lo_mask = (~(PAGE_SIZE - 1));
    curr_stack_lo = util_get_ebp();

    /*
     * Mask off the LSB 12 bits.
     */
    curr_stack_lo = (char *)((unsigned int)curr_stack_lo & stack_lo_mask);

    glb_mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);

    if (!glb_mutex) {
        /*
         * Should not have happened.
         */
        lprintf("[DBG_%s], Glb mutex is NULL \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    mutex_lock(glb_mutex);

    /*
     * Lets get the TCB 1st.
     */
    thr_tcb = thr_int_search_tcb_by_tid(tid);

    if (!thr_tcb) {
        /*
         * Either thread has already been removed or this tid never existed.
         */
        zombie_tcb = tcb_int_rem_zombie_thread(tid);

        if (!zombie_tcb) {

            /*
             * This tid never existed, return error.
             */

            rc = ERROR;

        } else {

            /*
             * Get the status from zombie thread & free up the memory.
             */

            if (statusp) {
                *statusp = zombie_tcb->tcb_data;
            }

            free(zombie_tcb);
            zombie_tcb = NULL;
        }

        mutex_unlock(glb_mutex);
        return (rc);
    }

    stack_lo = THR_TCB_GET_STKL(thr_tcb);
    thr_tid = THR_TCB_GET_TID(thr_tcb);

    if (thr_tid != tid) {

        /*
         * Not sure if this can happen, still we'll handle it the
         * same way we handled the non existing tcb scenario.
         */
        zombie_tcb = tcb_int_rem_zombie_thread(tid);
        if (!zombie_tcb) {

            /*
             * This tid never existed, return error.
             */

            rc = ERROR;

        } else {

            /*
             * Get the status from zombie thread & free up the memory.
             */

            if (statusp) {
                *statusp = zombie_tcb->tcb_data;
            }

            free(zombie_tcb);
            zombie_tcb = NULL;
        }

        mutex_unlock(glb_mutex);
        return (rc);
    }

    my_tcb = thr_int_search_tcb_by_stk(curr_stack_lo);

    if (!my_tcb) {

        /*
         * Somethign is really bad.
         */
        lprintf("[DBG_%s], My tcb NULL for stack: %p\n", 
                __FUNCTION__, curr_stack_lo);
        mutex_unlock(glb_mutex);
        rc = ERROR;
        return (rc);
    }

    self_mutex = THR_TCB_GET_MUTEX_PTR(my_tcb);

    if (!self_mutex) {
        /*
         * Should not have happened.
         */
        lprintf("[DBG_%s], Self mutex is NULL \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }

    /*
     * Time to link the child & parent.
     * a. Acquire the self lock.
     * b. Notify child tcb about waiting parent tcb.
     */
    mutex_lock(self_mutex);

    THR_TCB_SET_WAIT_TCB(thr_tcb, my_tcb);

    mutex_unlock(glb_mutex);

    /*
     * Now i'll wait on self lock again.
     * If Child has already executed by now it would 
     * have release the lock, else parent would wait 
     * on the lock.
     */
    mutex_lock(self_mutex);

    if (statusp) {
        /*
         * Update the return value.
         */
        *statusp = THR_TCB_GET_RET_VAL(my_tcb); 
    }

    mutex_unlock(self_mutex);

    return (rc);
}

/** @brief  Returns tid of current thread.
 *
 *  @param  tid: Thread on which caller wants to join.
 *  @param  statusp: Pointer to memory lcoation where 
 *                   we need to put thr_exit status.
 *
 *  @return ERROR if validation fails, SUCCESS otherwise.
 */
int
thr_getid()
{
    char *curr_stack_ptr = NULL;
    char *stack_lo = NULL;
    unsigned int stack_lo_mask = 0;
    mutex_t *glb_mutex = NULL;
    tcb_t *my_tcb = NULL;
    tid_t ret_tid = 0;

    glb_mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);

    if (!glb_mutex) {
        /*
         * Should not have happened.
         */
        lprintf("[DBG_%s], Glb mutex is NULL \n", __FUNCTION__);
        return (ret_tid);
    }

    curr_stack_ptr = util_get_esp();
    stack_lo_mask = (~(PAGE_SIZE - 1));
    stack_lo = (char *)(((unsigned int)curr_stack_ptr) & stack_lo_mask);

    mutex_lock(glb_mutex);

    my_tcb = thr_int_search_tcb_by_stk(stack_lo);

    if (!my_tcb) {

        /*
         * Weird my tcb does not exist in the database.
         */
        lprintf("[DBG_%s], my_tcb NULL for stack: %p \n", 
                                __FUNCTION__, stack_lo);

        mutex_unlock(glb_mutex);
        return (ret_tid);
    }

    ret_tid = THR_TCB_GET_TID(my_tcb);

    mutex_unlock(glb_mutex);

    return (ret_tid);
}

/** @brief  Yields the control to another thread.
 *
 *  @param  tid: Thread to which caller wntes to yield to.
 *
 *  @return ERROR if validation fails, SUCCESS otherwise.
 */
int
thr_yield(int tid)
{
    mutex_t *glb_mutex = NULL;
    tcb_t *other_tcb = NULL;
    int rc = SUCCESS;

    glb_mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);

    if (!glb_mutex) {
        /*
         * Should not have happened.
         */
        lprintf("[DBG_%s], Glb mutex is NULL \n", __FUNCTION__);
        rc = ERROR;
        return (rc);
    }


    if (tid != -1) {

        mutex_lock(glb_mutex);

        /*
         * No need to search for any TCB, we'll pass
         * on the argument as it is.
         */

        other_tcb = thr_int_search_tcb_by_tid(tid);

        if (!other_tcb) {
            /*
             * Searching the tcb failed.
             */
            lprintf("[DBG_%s], Search for tid: %d failed \n", __FUNCTION__, tid);
            mutex_unlock(glb_mutex);
            rc = ERROR;
            return (rc);
        }

        /*
         * Get kernel TID.
         */
        tid = THR_TCB_GET_KTID(other_tcb);
        mutex_unlock(glb_mutex);
    }

    rc = yield(tid);

    return (rc);
}

/*
 * ====================================
 * APIS exposed to other library modules, but not to
 * user. 
 * ====================================
 */

/** @brief  Sets the stack hi for main thread.
 *
 *  @param  input: stack hi value
 *
 *  @return NA
 */

void 
thr_set_main_stackH(char *input) 
{
    thread_glbl.main_stack_hi = input;
    return;
}

/** @brief  Sets the stack lo for main thread..
 *
 *  @param  input: stack lo value
 *
 *  @return NA
 */
void 
thr_set_main_stackL(char *input) 
{
    thread_glbl.main_stack_lo = input;
    return;
}

/** @brief  Gets the stack hi from main thread..
 *
 *  @param  NA
 *
 *  @return stack hi address
 */
char *
thr_get_main_stackH() 
{
    return (thread_glbl.main_stack_hi);
}

/** @brief  Gets the stack lo from main thread..
 *
 *  @param  NA
 *
 *  @return stack lo address
 */
char *
thr_get_main_stackL() 
{
    return (thread_glbl.main_stack_lo);
}

/** @brief  Takes the mem mutex lock.
 *
 *  @param  NA
 *
 *  @return NA
 */
void
thr_mutex_mem_lock()
{
    mutex_t *mem_mutex = NULL;

    if (THR_GLB_GET_IS_MULTI_THR((&thread_glbl)) == FALSE) {

        /*
         * No need to take any lock since application
         * is single threaded.
         */
        return;
    }

    mem_mutex = THR_GLB_GET_MEM_MUTEX_PTR((&(thread_glbl)));

    if (!mem_mutex) {
        /*
         * Invalid input.
         */
        return;
    }

    mutex_lock(mem_mutex);
    return;
}

/** @brief  Releases the mem mutex lock.
 *
 *  @param  NA
 *
 *  @return NA
 */
void
thr_mutex_mem_unlock()
{
    mutex_t *mem_mutex = NULL;
    mem_mutex = THR_GLB_GET_MEM_MUTEX_PTR((&thread_glbl));

    if (THR_GLB_GET_IS_MULTI_THR((&thread_glbl)) == FALSE) {
        /*
         * No need to do anything, just return.
         */
        return;
    }

    if (!mem_mutex) {
        /*
         * Invalid input.
         */
        return;
    }

    mutex_unlock(mem_mutex);
    return;
}

/*
 * ====================================
 * Thread specifc internal APIS.
 * ====================================
 */

/** @brief  Allocates a new stack space.
 *  
 *  This function allocates stack space.
 *  It first check the free pool, & if its empty & then takes the next
 *  possible free page from stack address space. It also updates the skip
 *  list with allocated stack space.
 *
 *  @param  stack_size: Size of the stack we want to allcoate.
 *  @param  list_data: The data which we want to insert in skip list node data.
 *
 *  @return stack_lo of allocated address.
 */
char *
thr_int_allocate_stack(int stack_size, void *list_data)
{
    char *stack_ptr_lo = NULL;
    char *stack_ptr_hi = NULL;
    int rc = 0;
    char *reuse_stack = NULL;
    char *new_free_stack_lo = NULL;
    char *new_free_stack_hi = NULL;
    unsigned int child_stack_size = THR_GLB_GET_TSSIZE(&thread_glbl);
    skip_list_global_t *skip_list = NULL;
    uint32_t bucket_key_index = 0;

    reuse_stack = thr_int_pop_reuse_stack();

    if (!reuse_stack) {

        stack_ptr_lo = THR_GLB_GET_FSTKL(&thread_glbl);

        /*
         * OK we are assigning from free pool (not 
         * from reusable pool), update free_stack_hi
         * & free_stack_lo to the new values.
         */
        new_free_stack_hi = stack_ptr_lo - WSIZE;
        new_free_stack_lo = (new_free_stack_hi - child_stack_size + WSIZE);

        THR_GLB_SET_FSTKH(&thread_glbl, new_free_stack_hi);
        THR_GLB_SET_FSTKL(&thread_glbl, new_free_stack_lo);

    } else {

        stack_ptr_lo = reuse_stack;
    }

    /*
     * Allocate the page.
     */
    rc = new_pages(stack_ptr_lo, stack_size);

    if (rc < 0) {
        /*
         * Page allocation failed.
         */
        lprintf("[DBG_%s], Page allocation failed with rc: %d \n",
                                                 __FUNCTION__, rc);
        stack_ptr_lo = NULL;

        return (stack_ptr_lo);
    }

    stack_ptr_hi = (stack_ptr_lo + stack_size - WSIZE);

    skip_list = THR_GLB_GET_SKPLST_PTR((&thread_glbl));

    bucket_key_index = ((uint32_t)(stack_ptr_lo) & 
                       (THR_GLB_GET_BKT_KEY_MASK(&(thread_glbl))));
    /*
     * Stack ptr should already have 12 lsbs as zero since it is page
     * alligned.
     */
    skip_list_insert(skip_list, bucket_key_index, 
                    ((uint32_t)(stack_ptr_lo)), ((uint32_t)(stack_ptr_hi)), 
                    list_data);

    return (stack_ptr_lo);
}

/** @brief  Deallcoates stack space.
 *  
 *  This function deallocates stack space.
 *  It adds the node to reuse list & removes corrsponding node
 *  from skip_list. It does nto free the page as caller may still be 
 *  in the stack's context.
 *
 *  @param  base: Address of stack spce we want to free.
 *
 *  @return NA.
 */
void
thr_int_deallocate_stack(char *base)
{
    skip_list_global_t *skip_list = NULL;
    uint32_t bucket_key_index = 0;
    int rc = SUCCESS;

    if (!base)
        return;

    skip_list = THR_GLB_GET_SKPLST_PTR((&thread_glbl));

    /*
     * Remove the node from skip list..
     */

    bucket_key_index = ((uint32_t)(base) & 
                       (THR_GLB_GET_BKT_KEY_MASK(&(thread_glbl))));

    skip_list_remove(skip_list, bucket_key_index, ((uint32_t)base));

    /*
     * Insert this node in the reuse stack list.
     */
    rc = thr_int_push_reuse_stack(base);

    /*
     * Deallocate does not remove the page, as it could be called
     * in the same thread's context.
     */

    return;
}

/** @brief  Spwans a new thread.
 *  
 *  This function spanws a new thread by doing following.
 *  a. Calls asm_wrapper, which updates the esp (& restores it for main thread)
 *     & spawns a new thread, returning twice.
 *  b. Sets the kernel returned tid.
 *  c. Calls the thread's registered callback.
 *  d. Handles the case where thread returns without calling thr_exit. 
 *
 *  @param  base: Address of stack spce we want to free.
 *
 *  @return NA.
 */
void
thr_int_fork_c_wrapper(tcb_t **input_tcb)
{
    tid_t child_tid = 0;
    char *child_stack_hi = NULL;
    tcb_t *new_tcb = *input_tcb;

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

    child_tid = thr_int_fork_asm_wrapper(child_stack_hi - WSIZE);

    if (child_tid) {

        /*
         * Inside Parent.
         */
        THR_TCB_SET_KTID(new_tcb, child_tid);

    } else {

        /*
         * Inside child context.
         * Lets run the registered function.
         */

        func = THR_TCB_GET_FUNC(new_tcb);
        args = THR_TCB_GET_ARGS(new_tcb);

        /*
         * Lets install the exception handler.
         */

        /*
         * Time to call the child function.
         */
        child_rc = (*func) (args);


        /*
         * Reaching here means that thread is done w/o calling thread_exit.
         * thread exit should never return.
         */
        thr_exit(child_rc);
    }

    return; 
}

/** @brief  Creates a new tcb
 *  
 *  @param  stack_hi: stack_hi of its allcoated stack.
 *  @param  stack_lo: stack_lo of its allcoated stack.
 *  @param  func: Thread callback
 *  @param  arg: Callback's argument
 *
 *  @return NA.
 */
tcb_t *
thr_int_create_tcb(char *stack_hi, char *stack_lo, 
                   void * (*func)(void *), void *arg)
{
    tcb_t *new_tcb = NULL;
    mutex_t *self_mutex = NULL;
    char *tcb_excp_stack_lo = NULL;
    char *tcb_excp_stack_hi = NULL;

    new_tcb = malloc(sizeof(tcb_t));
    memset(new_tcb, 0, sizeof(tcb_t));

    /*
     * Create Exception stack on heap.
     */
    tcb_excp_stack_lo = calloc(1, EXCP_STACK_SIZE);
    tcb_excp_stack_hi = tcb_excp_stack_lo + EXCP_STACK_SIZE;

    /*
     * Set TCB parameters.
     */
    THR_TCB_SET_STKH(new_tcb, stack_hi);
    THR_TCB_SET_STKL(new_tcb, stack_lo);
    THR_TCB_SET_FUN(new_tcb, func);
    THR_TCB_SET_ARG(new_tcb, arg);
    THR_TCB_SET_EXC_STKH(new_tcb, tcb_excp_stack_hi);
    THR_TCB_SET_EXC_STKL(new_tcb, tcb_excp_stack_lo);

    /*
     * Initialize the per TCB mutex.
     * This mutex is used for implementing thread join.
     */
    self_mutex = THR_TCB_GET_MUTEX_PTR(new_tcb);

    /*
     * The thread will take lock on its own thread.
     */
    mutex_init(self_mutex);

    return (new_tcb);
}

/** @brief  Allocates a new tid from stack address.
 *
 *  Fetches tid from stack address.
 *  
 *  @param  stack_lo: stack_lo of its allcoated stack.
 *
 *  @return tid: Generated tid.
 */
tid_t 
thr_int_allocate_new_tid(char *stack_lo)
{
    unsigned int stack_lo_mask = 0;
    unsigned long rand_num = 0;
    tid_t new_tid = TID_NUM_INVALID;

    stack_lo_mask = (~((PAGE_SIZE/2) - 1));

    /*
     * Mask out the 12 lsb bits which will consumed within the page.
     */
    new_tid = ((unsigned int)stack_lo) & (stack_lo_mask);

    /*
     * Right shift this new tid by 1 so that we can have a 
     * +ve return value & reset the MSB.
     */
    new_tid = (new_tid >> 1);

    /*
     * Generate a 11 bit random number & append it to new_tid.
     */
    rand_num = (genrand() % (PAGE_SIZE/2));

    /*
     * The tid that we are generating is linked with stack address.
     */
    new_tid = ((new_tid | rand_num) & (0x7fffffff));

    return (new_tid);
}

/** @brief  Searches for a tcb in skip list using stack_lo as key.
 *
 *  @param  stack_lo: stack_lo of its allcoated stack.
 *
 *  @return tcb_t: Searched TCB.
 */
tcb_t *
thr_int_search_tcb_by_stk(char *stack_lo)
{
    tcb_t *tcb = NULL;
    skip_list_global_t *skip_list = NULL;
    uint32_t bucket_key_index = 0;

    skip_list = THR_GLB_GET_SKPLST_PTR(&(thread_glbl));

    if (!skip_list)
        return NULL;

    bucket_key_index = ((uint32_t)(stack_lo) & 
                       (THR_GLB_GET_BKT_KEY_MASK(&(thread_glbl))));

    tcb = skip_list_find(skip_list, bucket_key_index, ((uint32_t)(stack_lo)));

    if (!tcb) {

        /*
         * Check if address represents the main thread.
         */
        if (((uint32_t)stack_lo >= (uint32_t)thr_get_main_stackL()) && 
                ((uint32_t)stack_lo <= (uint32_t)thr_get_main_stackH())) {

            tcb = THR_GLB_GET_MAIN_TCB_PTR(&thread_glbl);

        } else {

            /*
             * Enable it for debugging.
             */
#if 0
            skip_list_dbg_dump_all(skip_list);
#endif
        }
    }

    return (tcb);
}

/** @brief  Searches for a tcb by tid.
 *
 *  @param  tid: tid of the tcb that we want to search.
 *
 *  @return tcb_t: Searched TCB.
 */
tcb_t* 
thr_int_search_tcb_by_tid(tid_t tid)
{
    unsigned int stack_lo_mask = 0;
    char *stack_lo = NULL;
    tcb_t *ret_tcb = NULL;

    stack_lo_mask = (~((PAGE_SIZE/2) - 1));

    /*
     * We'll fetch the stack address from tid, mask out
     * the lsbs.
     */
    stack_lo = (char *)((tid & stack_lo_mask) << 1);

    ret_tcb = thr_int_search_tcb_by_stk(stack_lo);

    return (ret_tcb);
}

/** @brief  Adds a node to zombie list.
 *
 *  @param  tcb: tcb that we want to insert
 *  @param  tcb_data: tcb related data that we want in each zombie node.
 *                    for ex: status in case of thr_exit.
 *
 *  @return NA.
 */
void
tcb_int_push_zombie_thread(tcb_t *tcb, void *tcb_data)
{
    tid_t input_tid = 0;
    tcb_zombie_t *tcb_zombie_node = NULL;
    tcb_zombie_t *tcb_zombie_head = NULL;

    if (!tcb) {
        /*
         * Input is invalid.
         */
        lprintf("[DBG_%s], Invalid input \n", __FUNCTION__);
        return;
    }

    /*
     * Insert this node at the head.
     */
    input_tid = THR_TCB_GET_TID(tcb);
    tcb_zombie_node = calloc(1, sizeof(tcb_zombie_t));

    tcb_zombie_node->tid = input_tid;
    tcb_zombie_node->tcb_data = tcb_data;
    tcb_zombie_node->next = NULL;

    tcb_zombie_head = thread_glbl.zombie_threads;
    thread_glbl.zombie_threads = tcb_zombie_node;


    if (tcb_zombie_head) {
        /*
         * Non empty zombie queue
         */
        tcb_zombie_node->next = tcb_zombie_head;
    } 

    return;
}

/** @brief  remove a node from zombie list.
 *
 *  @param  tid: tid of the tcb for which i want to remove the zombie node.
 *
 *  @return zombie node.
 */
tcb_zombie_t *
tcb_int_rem_zombie_thread(tid_t input_tid)
{

    tcb_zombie_t *tcb_ret_node = NULL;
    tcb_zombie_t *tcb_zombie_head = NULL;
    tcb_zombie_t *tcb_prev_node = NULL;

    tcb_zombie_head = thread_glbl.zombie_threads;
    
    while (tcb_zombie_head) {

        if (tcb_zombie_head->tid == input_tid) {

            /*
             * Found a matching node.
             */
            tcb_ret_node = tcb_zombie_head;
            break;
        }

        tcb_prev_node = tcb_zombie_head;
        tcb_zombie_head = tcb_zombie_head->next;
    }

    if (!tcb_ret_node) {
        /*
         * No Match found.
         */
        lprintf("[DBG_%s], Match failed for %d \n", __FUNCTION__, input_tid);
        return (NULL);
    }

    /*
     * Match found, remove this node.
     */
    if (tcb_prev_node) {

        tcb_prev_node->next = tcb_ret_node->next;

    } else {

        /*
         * Change head.
         */
        thread_glbl.zombie_threads = tcb_ret_node->next;
    }

    /*
     * Delink the node from linked list.
     */
    tcb_ret_node->next = NULL;

    return (tcb_ret_node);
}

/** @brief  push a stack address to free pool.
 *
 *  @param  stack_lo: address that we want to add.
 *
 *  @return ERROR if insertion fails, SUCCESS otherwise
 */
int
thr_int_push_reuse_stack (char *stack_lo)
{
    thread_reuse_stack_t *head = NULL;
    thread_reuse_stack_t *new_node = NULL;
    int rc = SUCCESS;

    new_node = calloc(1, sizeof(thread_reuse_stack_t));
    new_node->stack_lo = stack_lo;

    head = THR_GLB_GET_RSTACK(&(thread_glbl));

    if (head) {

        /*
         * Not the 1st node.
         */
        new_node->next = head->next;
    }

    THR_GLB_SET_RSTACK((&(thread_glbl)), new_node);

    return (rc);
}

/** @brief  pops a stack address from free pool.
 *
 *  @param  NA
 *
 *  @return free stack address
 */
char *
thr_int_pop_reuse_stack (void)
{
    thread_reuse_stack_t *head = NULL;
    thread_reuse_stack_t *next_node = NULL;
    char *ret_ptr = NULL;

    head = THR_GLB_GET_RSTACK(&(thread_glbl));

    if (head) {

        ret_ptr = head->stack_lo;
        next_node = head->next;
        free(head);
        head = NULL;
    }

    THR_GLB_SET_RSTACK((&(thread_glbl)), next_node);

    return (ret_ptr);
}

/** @brief  Searches if a particular address is in reuse pool.
 *
 *  @param  base: address to be searched.
 *
 *  @return TRUE: if present, FALSE: if not.
 */
boolean_t
thr_int_search_reuse_stack(char *base)
{
    thread_reuse_stack_t *head = NULL;
    boolean_t rc = FALSE;
    head = THR_GLB_GET_RSTACK(&(thread_glbl));

    /*
     * Search if this address is already present.
     */
    while (head) {

        if (head->stack_lo == base) {
            rc = TRUE;
            break;
        }

        head = head->next;
    }

    return (rc);
}

/** @brief  Creates the tcb for main thread.
 *
 *  @param  stack_hi: Stack hi for main thread.
 *  @param  stack_lo: Stack lo for main thread.
 *
 *  @return NA
 */
void 
thr_int_add_main_tcb(char *stack_hi, char *stack_lo)
{
    tcb_t *main_tcb = NULL;
    mutex_t *self_mutex = NULL;
    char *tcb_excp_stack_lo = NULL;
    char *tcb_excp_stack_hi = NULL;

    /*
     * At this stage we expect that only main thread
     * would be running hence no need for mutex protection
     * while access thread_global.
     */

    /*
     * We have stack values now, lets initialize the main tcb.
     * i.e TCB used to represent the main thread.
     */
    main_tcb = THR_GLB_GET_MAIN_TCB_PTR(&(thread_glbl));
    THR_TCB_SET_STKH(main_tcb, stack_hi);
    THR_TCB_SET_STKL(main_tcb, stack_lo);

    self_mutex = THR_TCB_GET_MUTEX_PTR(main_tcb);
    mutex_init(self_mutex);

    tcb_excp_stack_lo = calloc(1, EXCP_STACK_SIZE);
    tcb_excp_stack_hi = tcb_excp_stack_lo + EXCP_STACK_SIZE;
    
    THR_TCB_SET_EXC_STKH(main_tcb, tcb_excp_stack_hi);
    THR_TCB_SET_EXC_STKL(main_tcb, tcb_excp_stack_lo);

    thr_int_install_excp_handler(tcb_excp_stack_hi, 
                                 thr_int_swexn_handler,
                                 NULL, NULL);

    return;
}

/** @brief  Installs an exception handler in invoking thread.
 *
 *  @param  esp3: Exception stack.
 *  @param  eip:  Callback for exception handling.
 *  @param  arg:  Argument thread wants to pass on to exc handler.
 *  @param  newureg: set of register values
 
 *  @return NA
 */
int 
thr_int_install_excp_handler(void *esp3, swexn_handler_t eip,
                                  void *arg, ureg_t *newureg)
{
    int rc = SUCCESS;
    rc = swexn(esp3, eip, arg, newureg);
    return (rc);
}

/** @brief  Registered callback fo s/w exception handling.
 *
 *  @param  arg:  Argument thread wants to pass on to exc handler.
 *  @param  ureg: set of register values
 
 *  @return NA
 */
void 
thr_int_swexn_handler(void *arg, ureg_t *ureg)
{
    boolean_t is_multi_thr = FALSE;
    int rc = 0;
    tcb_t *main_tcb = NULL;
    int extd_stack_size = PAGE_SIZE;
    char *tcb_excp_stack_hi = NULL;
    char *stack_lo = NULL;
    char *curr_stack_ptr = NULL;
    unsigned int stack_lo_mask = 0;
    tcb_t *my_tcb = NULL;
    mutex_t *glb_mutex = NULL;
    char *main_stack_lo = NULL;
    char *resv_stack_hi = NULL;

    rc = ureg->cause;

    is_multi_thr = THR_GLB_GET_IS_MULTI_THR((&(thread_glbl)));
    main_tcb = THR_GLB_GET_MAIN_TCB_PTR(&thread_glbl);

    main_stack_lo = THR_TCB_GET_STKL(main_tcb);
    resv_stack_hi = THR_GLB_GET_RSTKH(&(thread_glbl));
    
    curr_stack_ptr = util_get_esp();
    stack_lo_mask = (~(PAGE_SIZE - 1));
    stack_lo = (char *)(((unsigned int)curr_stack_ptr) & stack_lo_mask);

    glb_mutex = THR_GLB_GET_MUTEX_PTR(&thread_glbl);

    /*
     * No need to check anything.
     */
    if (rc != SWEXN_CAUSE_PAGEFAULT) {

        /*
         * As of now we handle only page fault.
         */
        vanish();
    }

    if (is_multi_thr == FALSE) {

        rc = thr_int_expand_stack(main_tcb, extd_stack_size);
        tcb_excp_stack_hi = THR_TCB_GET_EXC_STKH(main_tcb);

        if (rc != SUCCESS) {
            /*
             * Stack expansion has failed, vanish.
             */
            vanish();
        }

        /*
         * Install the handler again.
         */
        thr_int_install_excp_handler(tcb_excp_stack_hi,
                                     thr_int_swexn_handler,
                                     NULL, ureg);
    } else {

        /*
         * If i am main thread then we'll allow main thread to extend for
         * MAIN_STACK_EXTRA_PAGES, if i receive exception for child thread
         * then i'll just call thr_exit to clean up the acquired resources.
         */

        mutex_lock(glb_mutex);

        /*
         * Search TCB.
         */
        my_tcb = thr_int_search_tcb_by_stk(stack_lo);

        if (!my_tcb) {
            /*
             * Search failed.
             * Data structure corruption.
             * Log the event but the thread should be killed anyways.
             */
            lprintf("[DBG_%s], search based on stk: %p failed \n",
                                           __FUNCTION__, stack_lo);
            mutex_unlock(glb_mutex);
            vanish();
        }

        if ((THR_TCB_GET_TID(my_tcb)) == (THR_TCB_GET_TID(main_tcb))) {

            /*
             * I am the main thread, see if i can expand.
             */
            if ((main_stack_lo - resv_stack_hi) == WSIZE) {
                /*
                 * Main stack cannot expand, vanish
                 */
                vanish();
            }

            /*
             * Expand the main stack by one page.
             */
            rc = thr_int_expand_stack(main_tcb, extd_stack_size);
            tcb_excp_stack_hi = THR_TCB_GET_EXC_STKH(main_tcb);

            if (rc != SUCCESS) {
                /*
                 * Stack expansion has failed, vanish.
                 */
                vanish();
            }

            /*
             * Install the handler again.
             */
            thr_int_install_excp_handler(tcb_excp_stack_hi,
                                     thr_int_swexn_handler,
                                                NULL, ureg);

        } else {
            /*
             * I am a child thread, exit.
             */
            thr_exit(NULL);
        }

    }

    return;
}

/** @brief  Expands stack for calling thread.
 *
 *  @param  tcb:  tcb corresponding to calling thread.
 *  @param  size: size by which we want to increment.
 
 *  @return ERROR if expansion fails, SUCCESS otherwise
 */
int 
thr_int_expand_stack(tcb_t *tcb, int size)
{
    int rc = SUCCESS;
    char *curr_lo = NULL;
    char *new_lo = NULL;

    curr_lo = THR_TCB_GET_STKL(tcb);

    /*
     * Increment the stack space by size.
     */
    new_lo = curr_lo - size;

    rc = new_pages(new_lo, size);

    if (rc < 0) {
        /*
         * Page allocation failed.
         */
        lprintf("[DBG_%s], Page allocation failed with rc: %d \n",
                __FUNCTION__, rc);
        /*
         * Cannot expand the stack inform the caller.
         */
        return (rc);
    }

    /*
     * Update it in s/w.
     */
    THR_TCB_SET_STKL(tcb, new_lo);

    return (rc);
}
