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
 * Return empty and nullified node
 *
 * Exported for API tests.
 */
bpt_node *
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
bpt_init(bpt_key_access_cb keys_key_access,
	 bpt_key_compare_cb keys_key_compare,
	 bpt_free_cb keys_key_free,
	 bpt_key_access_cb children_key_access,
	 bpt_key_compare_cb children_key_compare,
	 bpt_free_cb children_key_free,
	 uint16_t m){
    bpt_tree *tree;

    if(m < 3){
	printf("b+ tree's 'm' is too small\n");
	return NULL;
    }

    tree = (bpt_tree *) bpt_malloc(sizeof(bpt_tree));
    tree->m = m;

    /* Set up the initial empty node with empty lists */
    tree->root = bpt_gen_node();
    tree->root->is_root = tree->root->is_leaf = true;
    tree->root->keys = ll_init(keys_key_access,
			       keys_key_compare,
			       keys_key_free);
    tree->root->children = ll_init(children_key_access,
				   children_key_compare,
				   children_key_free);

    return tree;
}

void
bpt_insert(bpt_tree *bpt, void *new_key, void *new_data){}

bpt_node *
bpt_search(bpt_node *curr_node, void *new_key){
    linked_list *curr_keys;
    void *existing_key;
    int compared, children_index;
    bool found = false;

    /*
     * Iterate each bpt's key and compare it with the new key.
     * Search for an exact match or find the first smaller value.
     */
    curr_keys = curr_node->keys;
    children_index = 0;

    ll_begin_iter(curr_keys);
    while((existing_key = ll_get_iter_node(curr_keys)) != NULL){
	compared = curr_node->keys->key_compare_cb(curr_node->keys->key_access_cb(existing_key),
						   curr_node->keys->key_access_cb(new_key));
	if (compared == 0 /* equal */ || compared == 1 /* existing key > new_key */){
	    printf("found = true\n");
	    found = true;
	    break;
	}
	children_index++;
    }
    ll_end_iter(curr_keys);

    if (found){
	if (curr_node->is_root && curr_node->is_leaf){
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
	/*
	 * Didn't find any value larger than new key value.
	 * All keys in this node are smaller than the new key.
	 */
	printf("did not found larger or equal key value\n");

	if (curr_node->is_root && curr_node->is_leaf){
	    /*
	     * Root without any children.
	     */
	    assert(curr_node->next == NULL);

	    return curr_node;
	}else if (curr_node->is_root && !curr_node->is_leaf){
	    /*
	     * Root with children.
	     */
	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children,
						ll_get_length(curr_node->keys)) /* rightmost child */,
						new_key);
	}else if (!curr_node->is_root && curr_node->is_leaf){
	    /*
	     * Leaf node that doesn't have any children.
	     */
	    return curr_node;
	}else if (!curr_node->is_root && !curr_node->is_leaf){
	    /*
	     * Internal node.
	     */
	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children, ll_get_length(curr_node->keys)),
			      new_key);
	}
    }

    assert(0);

    return NULL;
}

void
bpt_delete(bpt_tree *bpt, void *key){}

void
bpt_destroy(bpt_tree *bpt){}
