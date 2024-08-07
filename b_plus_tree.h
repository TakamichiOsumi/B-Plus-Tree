#ifndef __B_Plus_Tree__
#define __B_Plus_Tree__

#include <stdbool.h>
#include <stdint.h>

#include "bpt_key_handler.h"
#include "Linked-List/linked_list.h"

typedef struct bpt_tree bpt_tree;

/*
 * B+ Tree Node
 *
 * Representation of root, internal or leaf nodes.
 */
typedef struct bpt_node {

    bool is_root;
    bool is_leaf;

    /*
     * Keys
     *
     * The max number of keys is 'max_keys'.
     */
    linked_list *keys;

    /*
     * Children
     *
     * The max number of children is equal to 'max_keys' if this node
     * is a leaf node. The number is 'max_keys + 1' if this is an
     * internal node or root node.
     */
    linked_list *children;

    struct bpt_node *parent;

    /*
     * Build doubly linked list between nodes on same level.
     */
    struct bpt_node *prev;
    struct bpt_node *next;

} bpt_node;

/*
 * Specify how to access the key connected in the 'keys'.
 */
typedef void *(*bpt_key_access_cb)(void *p);

/*
 * Return -1 when k1 < k2, 0 when k1 == k2 and 1 when k1 > k2.
 */
typedef int (*bpt_key_compare_cb)(void *k1, void *k2,
				  void *key_metadata);

/*
 * Free dynamic memory inside of the application data.
 */
typedef void (*bpt_free_cb)(void *p);

/*
 * B+ Tree
 */
typedef struct bpt_tree {

    bpt_node *root;

    /*
     * Number of maximum children
     */
    uint16_t max_keys;

    /*
     * Manage the metadata of composite key.
     *
     * Copied to the linked list's keys_compare_metadata.
     */
    composite_key_store keys_compare_metadata;

} bpt_tree;

void bpt_dump_whole_tree(bpt_tree *bpt);
void bpt_node_validity(bpt_node *node);
bpt_node *bpt_gen_node(void);
bpt_node *bpt_gen_root_callbacks_node(bpt_tree *bpt);
bpt_tree *bpt_init(bpt_key_compare_cb keys_key_compare, bpt_free_cb keys_key_free,
		   bpt_free_cb records_record_free, uint16_t max_keys,
		   composite_key_store *keys_compare_metadata);
bool bpt_insert(bpt_tree *bpt, void *key, void *data);
bool bpt_search(bpt_tree *bpt, void *key, bpt_node **node,
		void **record);
bool bpt_delete(bpt_tree *bpt, void *key, void **record);
void bpt_destroy(bpt_tree *bpt);

#endif
