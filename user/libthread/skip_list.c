#include <stdio.h>
#include <malloc.h>
#include <simics.h>
#include <skip_list_int.h>
#include "skip_list_common.h"
#include "util.h"

skip_list_global_t *
skip_list_init(skip_list_global_t *skip_list_global, 
              uint32_t max_num_buckets, 
              uint32_t max_num_node)
{
    if (!skip_list_global) {
        lprintf("[DBG_%s], Invalid input \n", __FUNCTION__);
    }

    SLIST_GLB_SET_M_NUM_BKT((skip_list_global), max_num_buckets);
    SLIST_GLB_SET_M_NUM_NODES((skip_list_global), max_num_node);

    /*
     * This skip list global will be the point of reference.
     * For any skip list reference by any applictation.
     */

    return (skip_list_global);
}

int
skip_list_insert(skip_list_global_t *skip_list_glb, 
                int32_t bucket_index, uint32_t node_index, 
                void *data)
{
    int rc = SUCCESS;
    skip_list_bucket_t *list_bucket = NULL;
    skip_list_node_t *list_node = NULL;

    if ((!data) || (!skip_list_glb)) {

        /*
         * Invalid input, return.
         */
        lprintf("[DBG_%s], Recvd invalid input, skip_list_glb: %p, "
               "data: %p \n", __FUNCTION__, skip_list_glb, data);

        rc = ERROR;
        return (rc);
    }

    list_bucket = skip_list_get_bucket(skip_list_glb, bucket_index);
    if (!list_bucket) {

        /*
         * Create a bucket & initialize it.
         */
        list_bucket = calloc(1, sizeof(skip_list_bucket_t));
        SLIST_BKT_SET_KEY(list_bucket, bucket_index);

        /*
         * Insert this new bucket.
         */
        rc = skip_list_insert_bucket(skip_list_glb, list_bucket);
    }

    list_node = calloc(1, sizeof(skip_list_node_t));
    SLIST_NODE_SET_KEY(list_node, node_index);

    /*
     * If this node index is a dup then insert node should catch it.
     */
    rc = skip_list_insert_node(list_bucket, list_node);

    if (rc != SUCCESS) {
        /*
         * Somethign went wrong could be a dup.
         * Free the allcoated memory & return.
         */
        if (!(SLIST_BKT_GET_NNODES(list_bucket))) {

            /*
             * Bucket is empty release, the memory.
             */
            skip_list_remove_bucket(skip_list_glb, &list_bucket);
        }

        free(list_node);
        list_node = NULL;
    }

    return (rc);
}

void
skip_list_remove(skip_list_global_t *skip_list_glb,
                int32_t bucket_index, uint32_t node_index)
{
    skip_list_bucket_t *list_bucket = NULL;

    if ((!skip_list_glb)) {

        /*
         * Invalid input, return.
         */
        lprintf("[DBG_%s], Recvd invalid input, skip_list_glb: %p \n",
                                         __FUNCTION__, skip_list_glb);

        return;
    }

    list_bucket = skip_list_get_bucket(skip_list_glb, bucket_index);
    if (!list_bucket) {

        /*
         * Bucket itself is not present.
         * Error, return.
         */
        lprintf("[DBG_%s], Bucket not present \n", __FUNCTION__);
        return;
    }
    
    skip_list_remove_node(list_bucket, node_index);

    if (!(SLIST_BKT_GET_NNODES(list_bucket))) {

        /*
         * Bucket is empty release, the memory.
         */
        skip_list_remove_bucket(skip_list_glb, &list_bucket);
    }

    return;
}

skip_list_bucket_t* 
skip_list_get_bucket(skip_list_global_t *skip_list_glb,
                                   uint32_t input_key)
{
    skip_list_bucket_t *bucket = NULL;
    skip_list_bucket_t *ret_bucket = NULL;
    uint32_t bucket_key = 0;

    if (!(skip_list_glb)) {

        /*
         * Invalid input.
         */
        lprintf("[DBG_%s], list_glb NULL \n", __FUNCTION__);
        return (NULL);
    }

    bucket = SLIST_GLB_GET_HEAD(skip_list_glb);
    if (!bucket) {

        /*
         * Head itself is NULL, list is empty.
         */
        return (NULL);
    }

    while (bucket) {

        bucket_key = SLIST_BKT_GET_KEY(bucket);

        if (bucket_key == input_key) {

            /*
             * Found the matching bucket, break.
             */
            ret_bucket = bucket;
            break;
        }

        if (bucket_key < input_key) {
            /*
             * we have moved fwd from where the bucket should have been,
             * break.
             */
            ret_bucket = NULL;
            break;
        }

        bucket = SLIST_BKT_GET_NEXT(bucket);
    }

    return (ret_bucket);
}

int 
skip_list_insert_bucket(skip_list_global_t *skip_list_glb, 
                          skip_list_bucket_t* list_bucket)
{
    int rc = SUCCESS;
    skip_list_bucket_t* head = NULL;
    skip_list_bucket_t* bucket_iter = NULL;
    skip_list_bucket_t* prev_bucket = NULL;
    uint32_t input_key = 0;

    if ((!skip_list_glb) || (!list_bucket)) {

        /*
         * Invalid inputs, log the event & return.
         */
        lprintf("[DBG_%s], Invalid inputs, skip_list_glb: %p,"
                " list_bucket: %p \n", __FUNCTION__, 
                skip_list_glb, list_bucket);

        rc = ERROR;

        return (rc);

    }

    input_key = SLIST_BKT_GET_KEY(list_bucket);

    head = SLIST_GLB_GET_HEAD(skip_list_glb);

    if (!head) {

        /*
         * This is the 1st node.
         */
        SLIST_GLB_SET_HEAD(skip_list_glb, list_bucket);
        SLIST_GLB_INC_C_NUM_BKT(skip_list_glb);

    } else {

        /*
         * Iterate across the bucket lists & inserted the bucket
         * in sorted manner indexed by bucket key.
         */
        bucket_iter = head;

        while (1) {

            if (!bucket_iter)
                break;

            if (SLIST_BKT_GET_KEY(bucket_iter) == input_key) {

                /*
                 * DUP Key, invalid scenario
                 * log the event & return.
                 */
                lprintf("[DBG_%s], duplicate key: %lu\n", 
                        __FUNCTION__, input_key);

                rc = ERROR;
                return (rc);

            } else if (SLIST_BKT_GET_KEY(bucket_iter) > input_key) {

                /*
                 * Move further down.
                 * Key is still less move further down.
                 */
                prev_bucket = bucket_iter;
                bucket_iter = bucket_iter->next;

            } else {

                /*
                 * 1st bucket whose key is greater than input.
                 */
                break;
            }

        }

        /*
         * Reaching here means no error were observed during search.
         * Insert the new bucket.
         */
        SLIST_GLB_INC_C_NUM_BKT(skip_list_glb);
        list_bucket->next = bucket_iter;
        list_bucket->prev = prev_bucket;

        if (prev_bucket) { 

            prev_bucket->next = list_bucket;

        } else {

            SLIST_GLB_SET_HEAD(skip_list_glb, list_bucket);
        }

        if (bucket_iter)
            bucket_iter->prev = list_bucket;

    }

    return (rc);
}

void
skip_list_remove_bucket(skip_list_global_t *skip_list_glb, 
                       skip_list_bucket_t **input_bucket)
{
    skip_list_bucket_t *bucket_iter = NULL;
    skip_list_bucket_t *prev_bucket = NULL;
    skip_list_bucket_t *next_bucket = NULL;
    skip_list_bucket_t *search_bucket = NULL;
    uint32_t input_key = 0;

    if ((!(*input_bucket)) || (!(skip_list_glb))) {

        /*
         * Invalid input param
         * Log it & return.
         */
        lprintf("[DBG_%s], Invalid input param, input_bucket: %p,"
                " skip_list_glb: %p\n",__FUNCTION__, *input_bucket, 
                                                    skip_list_glb);
        return;
    }

    input_key = SLIST_BKT_GET_KEY((*input_bucket));
    bucket_iter = SLIST_BKT_GET_HEAD(skip_list_glb);

    if (!bucket_iter) {

        /*
         * There are no buckets.
         * Log the event & return.
         */
        lprintf("[DBG_%s], no buckets present \n", __FUNCTION__);
        return;
    }

    search_bucket = (*(input_bucket));

    SLIST_GLB_DEC_C_NUM_BKT(skip_list_glb);

    prev_bucket = SLIST_BKT_GET_PREV(search_bucket);
    next_bucket = SLIST_BKT_GET_NEXT(search_bucket);

    if (prev_bucket) {

        prev_bucket->next = search_bucket->next;

    } else {

        SLIST_GLB_SET_HEAD(skip_list_glb, next_bucket);
    }

    if (next_bucket) {
        next_bucket->prev = prev_bucket;
    }

    SLIST_BKT_SET_PREV(search_bucket, NULL);
    SLIST_BKT_SET_NEXT(search_bucket, NULL);

    /*
     * Done with list cleanup, free the bucket.
     */
    free(search_bucket);
    *input_bucket = NULL;

    return;
}

int 
skip_list_insert_node(skip_list_bucket_t *input_bucket, 
                          skip_list_node_t* input_node)
{
    int rc = SUCCESS;
    skip_list_node_t* head = NULL;
    skip_list_node_t* node_iter = NULL;
    skip_list_node_t* prev_node = NULL;
    uint32_t input_key = 0;

    if ((!input_bucket) || (!input_node)) {

        /*
         * Invalid inputs, log the event & return.
         */
        lprintf("[DBG_%s], Invalid inputs, skip_list_glb: %p,"
                " list_bucket: %p \n", __FUNCTION__, 
                input_bucket, input_node);

        rc = ERROR;

        return (rc);
    }

    input_key = SLIST_NODE_GET_KEY(input_node);
    head = SLIST_BKT_GET_HEAD(input_bucket);

    if (!head) {

        /*
         * This is the 1st node.
         */
        SLIST_BKT_SET_HEAD(input_bucket, input_node);
        SLIST_BKT_INC_NNODES(input_bucket);


    } else {

        /*
         * Iterate across the node lists & inserted the bucket
         * in sorted manner indexed by node key.
         */
        node_iter = head;

        while (1) {

            if (!node_iter)
                break;

            if (SLIST_NODE_GET_KEY(node_iter) == input_key) {

                /*
                 * DUP Key, invalid scenario
                 * log the event & return.
                 */
                lprintf("[DBG_%s], duplicate key: %lu\n", 
                        __FUNCTION__, input_key);

                rc = ERROR;
                return (rc);

            } else if (SLIST_NODE_GET_KEY(node_iter) > input_key) {

                /*
                 * Move further down.
                 * Key is still less move further down.
                 */
                prev_node = node_iter;
                node_iter = node_iter->next;

            } else {

                /*
                 * 1st bucket whose key is greater than input.
                 */
                break;
            }

        }

        /*
         * Reaching here means no error were observed during search.
         * Insert the new node.
         */
        SLIST_BKT_INC_NNODES(input_bucket);
        input_node->next = node_iter;
        input_node->prev = prev_node;

        if (prev_node) {

            prev_node->next = input_node;

        } else {

            /*
             * Change the head of the linked list.
             */
            SLIST_BKT_SET_HEAD(input_bucket, input_node);
        }

        if (node_iter)
            node_iter->prev = input_node;
    }

    return (rc);
}

void
skip_list_remove_node(skip_list_bucket_t *input_bucket, uint32_t input_key)
{
    skip_list_node_t *search_node = NULL;
    skip_list_node_t *node_iter = NULL;
    skip_list_node_t *prev_node = NULL;
    skip_list_node_t *next_node = NULL;

    if (!input_bucket) {

        /*
         * Input list is empty.
         * Log it & return.
         */
        lprintf("[DBG_%s], Input bucket is NULL \n",__FUNCTION__);
        return;
    }

    node_iter = SLIST_BKT_GET_HEAD(input_bucket);

    if (!node_iter) {

        /*
         * Bucket has no nodes, should not have happened,
         * log the event & return.
         */
        lprintf("[DBG_%s], Input bucket is NULL \n", __FUNCTION__);
        return;
    }

    /*
     * Search through the list & find the node to be rmeoved.
     */
    while (1) {

        if (SLIST_NODE_GET_KEY(node_iter) == input_key) {

            search_node = node_iter;
            break;
        }

        if (SLIST_NODE_GET_KEY(node_iter) < input_key) {

            /*
             * we have crossed the place where node should have been.
             */
            break;
        }

        node_iter = node_iter->next;
    } 

    if (search_node) {

        /*
         * We could find the node.
         */
        SLIST_BKT_DEC_NNODES(input_bucket);

        prev_node = SLIST_NODE_GET_PREV(search_node);
        next_node = SLIST_NODE_GET_NEXT(search_node);

        if (prev_node) {

            prev_node->next = search_node->next;

        } else {

            SLIST_BKT_SET_HEAD(input_bucket, next_node);
        }

        if (next_node) {
            next_node->prev = prev_node;
        }

        SLIST_NODE_SET_PREV(search_node, NULL);
        SLIST_NODE_SET_NEXT(search_node, NULL);

        /*
         * Done with list cleanup, free the node.
         */
        free(search_node);
    }

    return;
}

/*
 * DBG APIS.
 */
void 
skip_list_dbg_dump_node(skip_list_node_t *input_node)
{
    if (!input_node)
        return;

    lprintf("\t\tSelf: %p\n", input_node);
    lprintf("\t\tNode_Key: %lu\n", SLIST_NODE_GET_KEY(input_node));
    lprintf("\t\tNext: %p\n", SLIST_NODE_GET_NEXT(input_node));
    lprintf("\t\tPrev: %p\n", SLIST_NODE_GET_PREV(input_node));

    return;
}

void 
skip_list_dbg_dump_bucket(skip_list_bucket_t *input_bucket)
{
    skip_list_node_t *node_iter = NULL;

    if (!input_bucket)
        return;

    lprintf("\tSelf: %p\n", input_bucket);
    lprintf("\tHead: %p\n", SLIST_BKT_GET_HEAD(input_bucket));
    lprintf("\tNext: %p\n", SLIST_BKT_GET_NEXT(input_bucket));
    lprintf("\tPrev: %p\n", SLIST_BKT_GET_PREV(input_bucket));
    lprintf("\tBucket_key: %lu\n", SLIST_BKT_GET_KEY(input_bucket));
    lprintf("\tNum_nodes: %lu\n", SLIST_BKT_GET_NNODES(input_bucket));

    node_iter =  SLIST_BKT_GET_HEAD(input_bucket);

    while (node_iter) {

        skip_list_dbg_dump_node(node_iter);

        node_iter = node_iter->next;
    }

    return;
}

void 
skip_list_dbg_dump_all(skip_list_global_t *input_global)
{
    skip_list_bucket_t *bucket_iter = NULL;

    if (!input_global)
        return;

    lprintf("Head: %p\n", SLIST_GLB_GET_HEAD(input_global));
    lprintf("Curr_num_buckets: %lu\n", SLIST_GLB_GET_C_NUM_BKT(input_global));
    lprintf("Max_num_buckets: %lu\n", SLIST_GLB_GET_M_NUM_BKT(input_global));
    lprintf("Max_num_nodes: %lu\n", SLIST_GLB_GET_M_NUM_NODES(input_global));

    bucket_iter = SLIST_GLB_GET_HEAD(input_global);

    while (bucket_iter) {

        skip_list_dbg_dump_bucket(bucket_iter);
        bucket_iter = bucket_iter->next;
    }

    return;
}
