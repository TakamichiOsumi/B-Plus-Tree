#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "b_plus_tree.h"

static bpt_node *
bpt_gen_node(){
    bpt_node *node;

    if ((node = (bpt_node *) malloc(sizeof(bpt_node))) == NULL){
	perror("malloc");
	exit(-1);
    }

    node->is_root = node->is_leaf = NULL;
    node->n_elems = 0;
    node->parent = node->next = node->last = NULL;

    return node;
}

bpt_tree *
bpt_init(bpt_key_compare_cb key_compare, bpt_free_cb free){
    bpt_tree *tree;

    if ((tree = (bpt_tree *) malloc(sizeof(bpt_tree))) == NULL){
	perror("malloc");
	exit(-1);
    }

    tree->root = NULL;
    tree->key_compare = key_compare;
    tree->free = free;

    return tree;
}

