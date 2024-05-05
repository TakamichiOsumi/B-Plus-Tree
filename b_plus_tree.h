#ifndef __B_Plus_Tree__
#define __B_Plus_Tree__

#include <stdbool.h>
#include <stdint.h>

#include "Linked-List/linked_list.h"

typedef struct bpt_key {
    uint16_t key_size;
    void *key;
} bpt_key;

/* Representation of root, internal or leaf nodes */
typedef struct bpt_node {

    bool is_root;
    bool is_leaf;

    /*
     * List of keys
     *
     * The maximum number of keys is 'm - 1'.
     */
    int n_keys;
    linked_list *keys;

    /*
     * List of children pointers.
     *
     * The maximum number of children is 'm'.
     *
     * If this node is an internal node or a root node,
     * 'children' point to the child nodes. Otherwise,
     * this points to the records.
     */
    linked_list *children;

    struct bpt_node *parent;
    struct bpt_node *next;
} bpt_node;


/* Specify how to access the key connected in the 'keys' */
typedef void *(*bpt_key_access_cb)(void *p);

/*
 * Return -1 when k1 < k2, 0 when k1 == k2 and 1 when k1 > k2.
 */
typedef int (*bpt_key_compare_cb)(void *k1, void *k2);

/* Free dynamic memory inside of the application data */
typedef void (*bpt_free_cb)(void *p);

typedef struct bpt_tree {
    bpt_node *root;

    bpt_node *left_most;

    bpt_key_access_cb key_access;

    bpt_key_compare_cb  key_compare;

    bpt_free_cb free;

     /* Equal to # of the maximum children */
    uint16_t m;
} bpt_tree;

bpt_key *bpt_gen_key(uint16_t key_size, void *key);
bpt_tree *bpt_init(bpt_key_access_cb key_access,
		   bpt_key_compare_cb key_compare,
		   bpt_free_cb free,
		   uint16_t m);
void bpt_insert(bpt_tree *bpt, bpt_key *key, void *data);
void bpt_search(bpt_tree *bpt, bpt_key *key);
void bpt_delete(bpt_tree *bpt, bpt_key *key);
void bpt_destroy(bpt_tree *bpt);

#endif
