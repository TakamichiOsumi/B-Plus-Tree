#ifndef __B_Plus_Tree__
#define __B_Plus_Tree__

#include <stdbool.h>
#include <stdint.h>

#define M 6

typedef struct bpt_key {
    uint16_t key_size;
    void *key;
} bpt_key;

/* Representation of root, internal or leaf nodes */
typedef struct bpt_node {

    bool is_root;
    bool is_leaf;

    /*
     * Array of keys.
     *
     * The maximum number of keys is 'm - 1'.
     */
    int n_keys;
    bpt_key *keys;

    /*
     * Array of children pointers.
     *
     * The maximum number of children is 'm'.
     *
     * If this node is an internal node or a root node,
     * 'children' point to the child nodes. Otherwise,
     * this points to the records.
     */
    void **children;

    struct bpt_node *parent;
    struct bpt_node *next;
    struct bpt_node *last;

} bpt_node;

/*
 * Return -1 when k1 > k2, 0 when k1 == k2 and 1 when k1 < k2.
 */
typedef int (*bpt_key_compare_cb)(bpt_key *k1, bpt_key *k2);
typedef void (*bpt_free_cb)(bpt_node *n);

typedef struct bpt_tree {
    bpt_node *root;

    bpt_node *left_most;

    bpt_key_compare_cb  key_compare;

    bpt_free_cb free;

    uint16_t m; /* Equal to # of children */
} bpt_tree;

bpt_key *bpt_gen_key(uint16_t key_size, void *key);
bpt_tree *bpt_init(bpt_key_compare_cb key_compare, bpt_free_cb free,
		   uint16_t m);
void bpt_insert(bpt_tree *bpt, bpt_key *key, void *data);
void bpt_search(bpt_tree *bpt, bpt_key *key);
void bpt_delete(bpt_tree *bpt, bpt_key *key);
void bpt_destroy(bpt_tree *bpt);

#endif
