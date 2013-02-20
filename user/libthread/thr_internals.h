/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */
#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <mutex_type.h>
#include <types.h>
#include "skip_list_common.h"
/*
 * =====================
 * Data Types.
 * =====================
 */
typedef int tid_t;

typedef struct thread_reuse_stack_s {

    char *stack_lo;
    struct thread_reuse_stack_s *next;

} thread_reuse_stack_t;

/*
 * Thread control block.
 */
typedef struct tcb_s {

    char *tcb_stack_hi;
    char *tcb_stack_lo;
    tid_t tid;
    tid_t kern_tid;
    void *func;
    void *args;
    void *ret_data;
    mutex_t self_mutex;
    struct tcb_s *waiting_tcb;
    void *ret_value;

} tcb_t;

/*
 * Thread globals.
 */
typedef struct thread_glbl_s {

    char *main_stack_hi;
    char *main_stack_lo;
    char *free_stack_hi;
    char *free_stack_lo;
    char *resv_stack_hi;
    char *resv_stack_lo;
    unsigned int tstack_size;
    thread_reuse_stack_t *reuse_stacks;
    mutex_t glb_mutex;
    skip_list_global_t skip_list;
    tcb_t main_tcb;

} thread_glbl_t;

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
#define TID_NUM_INVALID            0

/*
 * Thread specific error codes.
 */
#define THR_SUCCESS                 0
#define THR_ERROR                  -1
#define THR_ENOMEM                 -2
#define THR_TCB_INS_ERR            -3

/*
 * DATA Structure access GET/SET Macros.
 *
 * a. Accessing TCB
 */
#define THR_TCB_GET_STKH(_tcb_)      ((_tcb_) ? ((_tcb_)->tcb_stack_hi) : NULL)
#define THR_TCB_GET_STKL(_tcb_)      ((_tcb_) ? ((_tcb_)->tcb_stack_lo) : NULL)
#define THR_TCB_GET_TID(_tcb_)       ((_tcb_) ? ((_tcb_)->tid) : 0)
#define THR_TCB_GET_KTID(_tcb_)      ((_tcb_) ? ((_tcb_)->kern_tid) : 0)
#define THR_TCB_GET_FUNC(_tcb_)      ((_tcb_) ? ((_tcb_)->func) : NULL)
#define THR_TCB_GET_ARGS(_tcb_)      ((_tcb_) ? ((_tcb_)->args) : NULL)
#define THR_TCB_GET_RDATA(_tcb_)     ((_tcb_) ? ((_tcb_)->ret_data) : NULL)
#define THR_TCB_GET_MUTEX_PTR(_tcb_) ((&((_tcb_)->self_mutex)))
#define THR_TCB_GET_WAIT_TCB(_tcb_)  (((_tcb_)->waiting_tcb))
#define THR_TCB_GET_RET_VAL(_tcb_)   (((_tcb_)->ret_value))


#define THR_TCB_SET_STKH(_tcb_, _val_)   ((_tcb_) ? ((_tcb_)->tcb_stack_hi) \
                                         = _val_ : NULL)

#define THR_TCB_SET_STKL(_tcb_, _val_)   ((_tcb_) ? ((_tcb_)->tcb_stack_lo) \
                                         = _val_ : NULL)

#define THR_TCB_SET_TID(_tcb_, _val_)    ((_tcb_) ? ((_tcb_)->tid) = _val_ : 0)

#define THR_TCB_SET_KTID(_tcb_, _val_)   ((_tcb_) ? ((_tcb_)->kern_tid) =   \
                                         _val_ : 0)

#define THR_TCB_SET_FUN(_tcb_, _val_)    ((_tcb_) ? ((_tcb_)->func) = _val_ \
                                         : NULL)

#define THR_TCB_SET_ARG(_tcb_, _val_)    ((_tcb_) ? ((_tcb_)->args) = _val_ \
                                         : NULL)

#define THR_TCB_SET_RDATA(_tcb_, _val_)  ((_tcb_) ? ((_tcb_)->ret_data) =    \
                                         _val_ : NULL)

#define THR_TCB_SET_WAIT_TCB(_tcb_, _val_) (((_tcb_)->waiting_tcb) = _val_)
#define THR_TCB_SET_RET_VAL(_tcb_, _val_)  (((_tcb_)->ret_value) = _val_)

/*
 * b. Accessing Thread Global.
 */
#define THR_GLB_GET_FSTKH(_glb_)          ((_glb_)->free_stack_hi)
#define THR_GLB_GET_FSTKL(_glb_)          ((_glb_)->free_stack_lo)
#define THR_GLB_GET_RSTKH(_glb_)          ((_glb_)->resv_stack_hi)
#define THR_GLB_GET_RSTKL(_glb_)          ((_glb_)->resv_stack_lo)
#define THR_GLB_GET_TSSIZE(_glb_)         ((_glb_)->tstack_size)
#define THR_GLB_GET_RSTACK(_glb_)         ((_glb_)->reuse_stacks)
#define THR_GLB_GET_MUTEX_PTR(_glb_)      ((&((_glb_)->glb_mutex)))
#define THR_GLB_GET_SKPLST_PTR(_glb_)     ((&((_glb_)->skip_list)))
#define THR_GLB_GET_MAIN_TCB_PTR(_glb_)   (&((_glb_)->main_tcb))


#define THR_GLB_SET_FSTKH(_glb_, _val_)     (((_glb_)->free_stack_hi) = _val_)
#define THR_GLB_SET_FSTKL(_glb_, _val_)     (((_glb_)->free_stack_lo) = _val_)
#define THR_GLB_SET_RSTKH(_glb_, _val_)     (((_glb_)->resv_stack_hi) = _val_)
#define THR_GLB_SET_RSTKL(_glb_, _val_)     (((_glb_)->resv_stack_lo) = _val_)
#define THR_GLB_SET_TSSIZE(_glb_, _val_)    (((_glb_)->tstack_size) = _val_)
#define THR_GLB_SET_RSTACK(_glb_, _val_)    (((_glb_)->reuse_stacks) = _val_)


/*
 * =====================
 * Function Declarations
 * =====================
 */
void thr_int_fork_c_wrapper(tcb_t **new_tcb);
tid_t thr_int_fork_asm_wrapper(char *child_stack_hi);
void  thr_int_exit_asm_wrapper(mutex_t *mutex, char *new_stack_hi,
                                               char *old_stack_lo);
tcb_t * thr_int_create_tcb(char *stack_hi, char *stack_lo, 
                           void * (*func)(void *), void *arg);
int thr_int_insert_tcb(tcb_t *tcb);
char * thr_int_allocate_stack(int stack_size, void *data);
void thr_int_deallocate_stack(char *base);
tid_t thr_int_allocate_new_tid(char *stack_lo);
void thr_int_deallocate_tid(tid_t tid);
tcb_t* thr_int_search_tcb_by_stk(char *stack_lo);
tcb_t* thr_int_search_tcb_by_tid(tid_t tid);

#endif /* THR_INTERNALS_H */
