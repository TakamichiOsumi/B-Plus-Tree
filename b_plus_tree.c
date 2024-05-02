#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "b_plus_tree.h"

bpt_key*
bpt_gen_key(uint16_t key_size, void *key){
    bpt_key *new_key;

    if ((new_key = (bpt_key *) malloc(sizeof(bpt_key))) == NULL){
	perror("malloc");
	exit(-1);
    }

    new_key->key_size = key_size;
    new_key->key = key;

    return new_key;
}

static bpt_node *
bpt_gen_node(uint16_t m){
    bpt_node *node;

    if ((node = (bpt_node *) malloc(sizeof(bpt_node))) == NULL){
	perror("malloc");
	exit(-1);
    }
    node->is_root = node->is_leaf = NULL;
    node->n_elems = 0;
    if ((node->keys = (bpt_key *) malloc(sizeof(bpt_key) * m)) == NULL){
	perror("malloc");
	exit(-1);
    }
    if ((node->children = (void **) malloc(sizeof(void *))) == NULL){
	perror("malloc");
	exit(-1);
    }
    node->parent = node->next = node->last = NULL;

    return node;
}

bpt_tree *
bpt_init(bpt_key_compare_cb key_compare, bpt_free_cb free, uint16_t m){
    bpt_tree *tree;

    assert(m >= 3);

    if ((tree = (bpt_tree *) malloc(sizeof(bpt_tree))) == NULL){
	perror("malloc");
	exit(-1);
    }

    tree->root = NULL;
    tree->key_compare = key_compare;
    tree->free = free;
    tree->m = m;

    return tree;
}

void
bpt_insert(bpt_tree *bpt, bpt_key *key, void *data){
    assert(bpt != NULL);
    assert(key != NULL);
    assert(data != NULL);

    if (bpt->root == NULL){
	bpt->root  = bpt_gen_node(bpt->m);
    }
}

void
bpt_search(bpt_tree *bpt, bpt_key *key){
}

void
bpt_delete(bpt_tree *bpt, bpt_key *key){
}

void
bpt_destroy(bpt_tree *bpt){
}
