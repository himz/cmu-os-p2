/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

/*
 * =====================
 * Data Types.
 * =====================
 */
typedef unsigned long tid_t;

typedef struct thread_reuse_stack_s {

    char *stack_hi;
    struct thread_reuse_stack_s *next;

} thread_reuse_stack_t;

/*
 * Thread globals.
 */
typedef struct thread_glbl_s {

    char *main_stack_hi;
    char *main_stack_lo;
    char *resv_stack_hi;
    char *resv_stack_lo;
    char *avail_stack_hi;
    char *avail_stack_lo;
    unsigned int thread_stack_size;
    thread_reuse_stack_t *reuse_stacks;

} thread_glbl_t;

/*
 * Thread control block.
 */
typedef struct tcb_s {

    char *tcb_stack_hi;
    char *tcb_stack_lo;
    tid_t tid;
    void *func;
    void *args;

} tcb_t;

/*
 * ====================
 * Macros.
 * ====================
 */
/*
 * Numberof extra pages main thread is allowed to extend to.
 */
#define MAIN_STACK_EXTRA_PAGES     2
#define RESV_STACK_NUM_PAGES       1
#define CHILD_STACK_NUM_PAGES      2
#define TID_NUM_INVALID            0

/*
 * Thread specific error codes.
 */
#define THR_SUCCESS                0
#define THR_ERROR                  -1
#define THR_ENOMEM                 -2
#define THR_TCB_INS_ERR            -3

/*
 * DATA Structure access GET/SET Macros.
 */
#define THR_TCB_GET_STKH(_tcb_)   ((_tcb_) ? ((_tcb_)->tcb_stack_hi) : NULL)
#define THR_TCB_GET_STKL(_tcb_)   ((_tcb_) ? ((_tcb_)->tcb_stack_lo) : NULL)
#define THR_TCB_GET_TID(_tcb_)    ((_tcb_) ? ((_tcb_)->tid) : 0)
#define THR_TCB_GET_FUNC(_tcb_)   ((_tcb_) ? ((_tcb_)->func) : NULL)
#define THR_TCB_GET_ARGS(_tcb_)   ((_tcb_) ? ((_tcb_)->args) : NULL)

#define THR_TCB_SET_STKH(_tcb_, _val_)   ((_tcb_) ? ((_tcb_)->tcb_stack_hi) \
                                         = _val_ : NULL)

#define THR_TCB_SET_STKL(_tcb_, _val_)   ((_tcb_) ? ((_tcb_)->tcb_stack_lo) \
                                         = _val_ : NULL)

#define THR_TCB_SET_TID(_tcb_, _val_)    ((_tcb_) ? ((_tcb_)->tid) = _val_ : 0)

#define THR_TCB_SET_FUN(_tcb_, _val_)    ((_tcb_) ? ((_tcb_)->func) = _val_ \
                                         : NULL)

#define THR_TCB_SET_ARG(_tcb_, _val_)    ((_tcb_) ? ((_tcb_)->args) = _val_ \
                                         : NULL)

/*
 * =====================
 * Function Declarations
 * =====================
 */
void thr_int_fork_c_wrapper(tcb_t *new_tcb);
tid_t thr_int_fork_asm_wrapper();
tcb_t * thr_int_create_tcb(char *stack_hi, char *stack_lo, 
                           void * (*func)(void *), void *arg);
int thr_int_insert_tcb(tcb_t *tcb);
char * thr_int_allocate_stack(int stack_size);
void thr_int_deallocate_stack(char *base);
tid_t thr_int_allocate_new_tid();
void thr_int_deallocate_tid(tid_t tid);
tid_t thr_int_fork_asm_wrapper(char *child_stack_hi);
#endif /* THR_INTERNALS_H */
