#ifndef _SKIP_LIST_INT_H_
#define _SKIP_LIST_INT_H_

#include <skip_list_common.h>
#include <stdint.h>

typedef struct skip_list_node_s {

    struct skip_list_node_s *next;
    struct skip_list_node_s *prev;
    uint32_t node_key;
    void *data;

} skip_list_node_t;

typedef struct skip_list_bucket_s {

    struct skip_list_node_s *head;
    struct skip_list_bucket_s *next;
    struct skip_list_bucket_s *prev;
    /*
     * This implementation is not as generic as it should be,
     * it expects the key is always an unsigned int.
     */
    uint32_t bucket_key;
    uint32_t num_nodes; /* Number of nodes present in this bucket. */

} skip_list_bucket_t;

/*
 * =====================
 * Macros
 * =====================
 */

/*
 * GET Macros.
 */
#define SLIST_BKT_GET_NEXT(_bkt_)   ((_bkt_)->next)
#define SLIST_BKT_GET_PREV(_bkt_)   ((_bkt_)->prev)
#define SLIST_BKT_GET_HEAD(_bkt_)   ((_bkt_)->head)
#define SLIST_BKT_GET_KEY(_bkt_)    ((_bkt_)->bucket_key)
#define SLIST_BKT_GET_NNODES(_bkt_) ((_bkt_)->num_nodes)

#define SLIST_NODE_GET_NEXT(_node_)   ((_node_)->next)
#define SLIST_NODE_GET_PREV(_node_)   ((_node_)->prev)
#define SLIST_NODE_GET_KEY(_node_)    ((_node_)->node_key)
#define SLIST_NODE_GET_DATA(_node_)   ((_node_)->data)

/*
 * SET Macros.
 */
#define SLIST_BKT_SET_NEXT(_bkt_, _val_)        (((_bkt_)->next = _val_))
#define SLIST_BKT_SET_PREV(_bkt_, _val_)        (((_bkt_)->prev = _val_))
#define SLIST_BKT_SET_KEY(_bkt_, _val_)         (((_bkt_)->bucket_key = _val_))
#define SLIST_BKT_SET_HEAD(_bkt_, _val_)        (((_bkt_)->head = _val_))
#define SLIST_BKT_SET_NNODES(_bkt_, _val_)      (((_bkt_)->num_nodes = _val_))

#define SLIST_NODE_SET_NEXT(_node_, _val_)        (((_node_)->next = _val_))
#define SLIST_NODE_SET_PREV(_node_, _val_)        (((_node_)->prev = _val_))
#define SLIST_NODE_SET_KEY(_node_, _val_)         (((_node_)->node_key = _val_))
#define SLIST_NODE_SET_DATA(_node_, _val_)        (((_glb_)->data = _val_))

/*
 * Declarations.
 */

skip_list_bucket_t* skip_list_get_bucket(skip_list_global_t *skip_list_glb, 
                                                      uint32_t bucket_key);

int skip_list_insert_bucket(skip_list_global_t *skip_list_glb, 
                            skip_list_bucket_t* list_bucket);
int
skip_list_insert_node(skip_list_bucket_t *insert_bucket,
                          skip_list_node_t* input_node);

void skip_list_remove_node(skip_list_bucket_t *input_bucket, 
                                        uint32_t input_key);

void skip_list_remove_bucket(skip_list_global_t *skip_list_glb,
                              skip_list_bucket_t** list_bucket);



#endif

