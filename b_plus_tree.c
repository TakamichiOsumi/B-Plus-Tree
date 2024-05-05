#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "b_plus_tree.h"

static void*
bpt_malloc(size_t size){
    void *p;

    if ((p = malloc(size)) == NULL){
	perror("malloc");
	exit(-1);
    }

    return p;
}

static bool
bpt_node_available(bpt_tree *tree, bpt_node *node){
    int max_key_num = tree->m - 1;

    if (node->n_keys < max_key_num)
	return true;
    else
	return false;
}

bpt_key*
bpt_gen_key(uint16_t key_size, void *key){
    bpt_key *new_key;

    new_key = (bpt_key *) bpt_malloc(sizeof(bpt_key));
    new_key->key_size = key_size;
    new_key->key = key;

    return new_key;
}

static bpt_node *
bpt_gen_node(uint16_t m){
    bpt_node *node;

    node = (bpt_node *) bpt_malloc(sizeof(bpt_node));
    node->is_root = node->is_leaf = NULL;
    node->n_keys = 0;
    node->keys = ll_init(NULL, NULL, NULL);
    node->children = ll_init(NULL, NULL, NULL);
    node->parent = node->next = NULL;

    return node;
}

bpt_tree *
bpt_init(bpt_key_compare_cb key_compare, bpt_free_cb free, uint16_t m){
    bpt_tree *tree;

    assert(m >= 3);

    tree = (bpt_tree *) bpt_malloc(sizeof(bpt_tree));

    /* Set up the initial empty node */
    tree->root = tree->left_most = bpt_gen_node(m);
    tree->root->is_root = tree->root->is_leaf = true;

    tree->key_compare = key_compare;
    tree->free = free;
    tree->m = m;

    return tree;
}

void
bpt_insert(bpt_tree *bpt, bpt_key *key, void *data){}

void
bpt_search(bpt_tree *bpt, bpt_key *key){}

void
bpt_delete(bpt_tree *bpt, bpt_key *key){}

void
bpt_destroy(bpt_tree *bpt){}
