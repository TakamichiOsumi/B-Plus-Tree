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
static bool
bpt_node_available(bpt_tree *tree, bpt_node *node){
    int max_key_num = tree->m - 1;

    if (node->n_keys < max_key_num)
	return true;
    else
	return false;
}
*/

bpt_key*
bpt_gen_key(uint16_t key_size, void *key){
    bpt_key *new_key;

    new_key = (bpt_key *) bpt_malloc(sizeof(bpt_key));
    new_key->key_size = key_size;
    new_key->key = key;

    return new_key;
}

/* Return empty and nullified node */
static bpt_node *
bpt_gen_node(void){
    bpt_node *node;

    node = (bpt_node *) bpt_malloc(sizeof(bpt_node));
    node->is_root = node->is_leaf = NULL;
    node->n_keys = 0;
    node->keys = NULL;
    node->children = NULL;
    node->parent = node->next = NULL;

    return node;
}

bpt_tree *
bpt_init(bpt_key_access_cb key_access,
	 bpt_key_compare_cb key_compare,
	 bpt_free_cb free, uint16_t m){
    bpt_tree *tree;

    assert(m >= 3);

    tree = (bpt_tree *) bpt_malloc(sizeof(bpt_tree));

    /* Set up the initial empty node with empty lists */
    tree->root = bpt_gen_node();
    tree->root->is_root = tree->root->is_leaf = true;
    tree->root->keys = ll_init(key_access,
			       key_compare,
			       NULL);
    tree->root->children = ll_init(key_access,
				   key_compare,
				   NULL);

    /* Set up the callback functions */
    tree->key_access = key_access;
    tree->key_compare = key_compare;
    tree->free = free;

    tree->m = m;

    return tree;
}

void
bpt_insert(bpt_tree *bpt, bpt_key *new_key, void *new_data){}

bpt_node *
bpt_search(bpt_node *curr_node, bpt_key *new_key){
    linked_list *curr_keys;
    bpt_key *existing_key;
    int compared, children_index;
    bool found = false;

    /*
     * Iterate each bpt's key and compare it with the new key.
     * Search for an exact match or find the first smaller value.
     */
    curr_keys = curr_node->keys;
    children_index = 0;
    ll_begin_iter(curr_keys);
    while((existing_key = (bpt_key *) ll_get_iter_node(curr_keys)) != NULL){
	compared = curr_node->keys->key_compare_cb(existing_key, new_key);
	if (compared == 0 || compared == 1){
	    found = true;
	    break;
	}
	children_index++;
    }
    ll_end_iter(curr_keys);

    if (found){
	if (curr_node->is_root){
	    return curr_node;
	}else{
	    /*
	     * Recursive call of child bpt_node.
	     */
	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children, children_index),
			      new_key);
	}
    }else{
	if (curr_node->is_root && curr_node->is_leaf){
	    /*
	     * This is a root without any children.
	     *
	     * Any smaller key couldn't be found
	     * and there is no 'next' node.
	     */
	    assert(curr_node->next == NULL);

	    return curr_node;
	}else if (curr_node->is_root && !curr_node->is_leaf){
	    /*
	     * This is a root that has children. Search for
	     * larger key value by recursive call from the
	     * rightmost children node.
	     */
	    int rightmost_index = ll_get_length(curr_node->keys);

	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children, rightmost_index),
			      new_key);
	}else if (!curr_node->is_root && curr_node->is_leaf){
	    /* This is leaf node that has upper node */
	    return NULL;
	}else if (!curr_node->is_root && !curr_node->is_leaf){
	    /* This is internal node */
	    int rightmost_index = ll_get_length(curr_node->keys);

	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children, rightmost_index),
			      new_key);
	}
    }

    assert(0);

    return NULL;
}

void
bpt_delete(bpt_tree *bpt, bpt_key *key){}

void
bpt_destroy(bpt_tree *bpt){}
