#ifndef _SKIP_LIST_COMMON_H_
#define _SKIP_LIST_COMMON_H_

#include <stdint.h>

typedef struct skip_list_global_s {

    void *head; /* Cannot expose internal list types to the app. */
    uint32_t curr_num_buckets;
    uint32_t max_num_buckets;
    uint32_t max_num_nodes; /* Num nodes per bucket. */

} skip_list_global_t;

/*
 * =====================
 * Macros
 * =====================
 */

/*
 * Accessing GLOBAL.
 */
#define SLIST_GLB_GET_C_NUM_BKT(_glb_)   ((_glb_)->curr_num_buckets)
#define SLIST_GLB_GET_M_NUM_BKT(_glb_)   ((_glb_)->max_num_buckets)
#define SLIST_GLB_GET_M_NUM_NODES(_glb_) ((_glb_)->max_num_nodes)
#define SLIST_GLB_GET_HEAD(_glb_)        ((_glb_)->head)

#define SLIST_GLB_SET_C_NUM_BKT(_glb_, _val_)   (((_glb_)->curr_num_backets) \
                                                = _val_)

#define SLIST_GLB_SET_M_NUM_BKT(_glb_, _val_)   (((_glb_)->max_num_buckets) = \
                                                _val_)

#define SLIST_GLB_SET_M_NUM_NODES(_glb_, _val_) (((_glb_)->max_num_nodes) =   \
                                                _val_)
#define SLIST_GLB_SET_HEAD(_glb_, _val_)        (((_glb_)->head) = _val_)

#endif
