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
    node->is_root = node->is_leaf = false;
    node->n_keys = 0;
    node->keys = node->children = NULL;
    node->parent = node->next = NULL;

    return node;
}

bpt_node *
bpt_gen_root_callbacks_node(bpt_tree *t){
    bpt_node *n;

    n = bpt_gen_node();
    n->keys = ll_init(t->root->keys->key_access_cb,
		      t->root->keys->key_compare_cb,
		      t->root->keys->free_cb);
    n->children = ll_init(t->root->children->key_access_cb,
			  t->root->children->key_compare_cb,
			  t->root->children->free_cb);

    return n;
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

bool
bpt_search(bpt_node *curr_node, void *new_key, bpt_node **last_explored_node){
    linked_list *curr_keys;
    void *existing_key;
    int cmp, children_index;

    printf("debug : search node : %p\n", curr_node);

    /* Set the return value first. This node can be the last */
    *last_explored_node = curr_node;

    /*
     * Iterate each bpt's key and compare it with the new key.
     * Search for an exact match or find the first smaller value.
     */
    curr_keys = curr_node->keys;
    children_index = 0;

    ll_begin_iter(curr_keys);
    while((existing_key = ll_get_iter_node(curr_keys)) != NULL){
	cmp = curr_keys->key_compare_cb(curr_keys->key_access_cb(existing_key),
					curr_keys->key_access_cb(new_key));
	/*
	 * Two keys are equal or the existing key is larger than new key.
	 * The latter means we can insert the 'new_key' before the larger
	 * existing key. Break now.
	 */
	if (cmp == 0 || cmp == 1){
	    break;
	}

	children_index++;
    }
    ll_end_iter(curr_keys);

    if (cmp == 0){
	/* Exact key match at the leaf node level */
	if (curr_node->is_leaf){
	    return true;
	}else{
	    /* Search for the leaf node */
	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children, children_index + 1),
			      new_key, last_explored_node);
	}
    }else if (cmp == 1){
	/* Found larger key value than the new_key */
	if (curr_node->is_leaf)
	    return false;
	else{
	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children, children_index),
			      new_key, last_explored_node);
	}
    }

    /* All keys in this node are smaller than the new key */
    printf("did not found smaller or equal key value than %p\n", new_key);

    /* If this node has no children, then search failure.
     * Otherwise, run a recursive call for the rightmost node.
     */
    if (curr_node->is_leaf){
	return false;
    }else{
	return bpt_search((bpt_node *)
			  ll_get_index_node(curr_node->children,
					    ll_get_length(curr_node->children) - 1),
			  new_key, last_explored_node);
    }
}

void
bpt_delete(bpt_tree *bpt, void *key){}

void
bpt_destroy(bpt_tree *bpt){}
