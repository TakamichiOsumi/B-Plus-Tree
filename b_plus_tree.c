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

/*
 * True if 'new_key' has lower value than any keys existing in the 'curr_node'
 */
static bool
bpt_found_insert_pos(bpt_tree *tree, bpt_key *new_key, bpt_node *curr_node){
    int i;
    bpt_key *curr_key;

    for (i = 0; i < curr_node->n_keys; i++){
	curr_key = &curr_node->keys[i];
	if (tree->key_compare(new_key, curr_key) > 0){
	    printf("found a existing key smaller than the new inserted key\n");
	    return true;
	}
    }

    return false;
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
    node->keys = (bpt_key *) bpt_malloc(sizeof(bpt_key) * m);
    node->children = (void **) bpt_malloc(sizeof(void *) * m);
    node->parent = node->next = node->last = NULL;

    return node;
}

bpt_tree *
bpt_init(bpt_key_compare_cb key_compare, bpt_free_cb free, uint16_t m){
    bpt_tree *tree;

    assert(m >= 3);

    tree = (bpt_tree *) bpt_malloc(sizeof(bpt_tree));

    /* set up the initial empty node */
    tree->root = tree->left_most = bpt_gen_node(m);
    tree->root->is_root = tree->root->is_leaf = true;

    tree->key_compare = key_compare;
    tree->free = free;
    tree->m = m;

    return tree;
}

void
bpt_insert(bpt_tree *bpt, bpt_key *key, void *data){
    bpt_node *prev, *curr;
    bool has_space, has_smaller_key;

    assert(bpt != NULL);
    assert(key != NULL);
    assert(data != NULL);

    prev = curr = bpt->left_most;


    do {
	has_space = has_smaller_key = false;

	/* find a place of insertion */
	has_space = bpt_node_available(bpt, curr);
	has_smaller_key = bpt_found_insert_pos(bpt, key, curr);

	if (has_space || has_smaller_key){

	    printf("found a node to insert new data\n");
	    if (has_space && has_smaller_key){
		/* insert before the key and keep the ascending order */
		;
	    }else if (has_space && !has_smaller_key){
		/* insert at the end */
		;
	    }else if (!has_space && has_smaller_key){
		;
	    }
	}

	prev = curr;

    } while((curr->last != NULL) && ((curr = curr->next) != NULL));
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
