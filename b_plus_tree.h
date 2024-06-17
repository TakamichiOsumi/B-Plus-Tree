#ifndef __B_Plus_Tree__
#define __B_Plus_Tree__

#include <stdbool.h>
#include <stdint.h>

#include "Linked-List/linked_list.h"

/*
typedef struct bpt_key {
    uint16_t key_size;
    void *key;
} bpt_key;
*/

typedef struct bpt_tree bpt_tree;

/*
 * B+ Tree Node
 *
 * Representation of root, internal or leaf nodes
 */
typedef struct bpt_node {

    bool is_root;
    bool is_leaf;

    /*
     * Keys
     *
     * The max number of keys is 'max_children - 1'.
     */
    linked_list *keys;

    /*
     * Children
     *
     * The max number of children is 'max_children'. Both
     * internal nodes and the root node point to other lower nodes.
     * Otherwise, this member points to inserted records.
     */
    linked_list *children;

    struct bpt_node *parent;

    /*
     * Build doubly linked list between leaf nodes.
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
typedef int (*bpt_key_compare_cb)(void *k1, void *k2);

/*
 * Free dynamic memory inside of the application data.
 */
typedef void (*bpt_free_cb)(void *p);

/*
 * B+ Tree
 */
typedef struct bpt_tree {

    bpt_node *root;

    /* The number of maximum children */
    uint16_t max_keys;

} bpt_tree;

bpt_node *bpt_gen_node(void);
bpt_node *bpt_gen_root_callbacks_node(bpt_tree *bpt);
bpt_tree *bpt_init(bpt_key_access_cb keys_key_access,
		   bpt_key_compare_cb keys_key_compare,
		   bpt_free_cb keys_key_free,
		   bpt_key_access_cb children_key_access,
		   bpt_key_compare_cb children_key_compare,
		   bpt_free_cb children_key_free,
		   uint16_t max_keys);
bool bpt_insert(bpt_tree *bpt, void *key, void *data);
bool bpt_search(bpt_tree *bpt, void *key, bpt_node **node);
bool bpt_delete(bpt_tree *bpt, void *key);
void bpt_destroy(bpt_tree *bpt);

#endif
