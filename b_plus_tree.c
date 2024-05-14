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

static void
bpt_dump_list(linked_list *list){
    void *p;

    ll_begin_iter(list);
    while((p = ll_get_iter_node(list)) != NULL){
	printf("\t dump : %lu\n", (uintptr_t) p);
    }
    ll_end_iter(list);
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

/*
 * Create a new empty node that copies
 * tree's root all callbacks for keys and children.
 *
 * Yet, both lists are returned as empty.
 */
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

/*
 * Return the right half node by ll_split().
 *
 * Wrapper function of ll_split() for node split. ll_split
 * returns the *left* half, so swap the return value internally.
 *
 * This is required to connect the split nodes with other
 * existing nodes before and after the split nodes.
 */
static bpt_node *
bpt_node_split(bpt_tree *t, bpt_node *curr_node){
    bpt_node *half;
    linked_list *left_keys, *left_children;
    int node_num = (t->m % 2 == 0) ? (t->m / 2) : (t->m / 2) + 1;

    /* Create an empty node whose 'keys' and 'children' are null */
    half = bpt_gen_node();

    /* Move the internal data of node */
    half->keys = ll_split(curr_node->keys, node_num);
    half->children = ll_split(curr_node->children, node_num + 1);

    /* Set the first right half child to NULL */
    ll_index_insert(curr_node->children, NULL, 0);

    /* Update the list states */
    half->n_keys = ll_get_length(half->keys);
    curr_node->n_keys = ll_get_length(curr_node->keys);

    /* Copy other attributes to share */
    half->is_root = curr_node->is_root;
    half->is_leaf = curr_node->is_leaf;
    half->parent = curr_node->parent;

    /*
     * Swap the two node to return the right half.
     * At this moment, the 'half' node has left part of keys and
     * children.
     */
    left_keys = half->keys;
    left_children = half->children;

    half->keys = curr_node->keys;
    half->children = curr_node->children;

    curr_node->keys = left_keys;
    curr_node->children = left_children;

    return half;
}

static void *
bpt_get_copied_up_key(linked_list *keys){
    void *first_key;

    ll_begin_iter(keys);
    first_key = ll_get_iter_node(keys);
    ll_end_iter(keys);

    return first_key;
}

/*
 * Insert a new pair of key and data, propagating keys towards
 * the top of tree recursively, when required.
 *
 * The first 'new_key' is a key user indicated. Meanwhile, recursive
 * call sets it to 'copied_up_key', since it needs to propagate one
 * of keys, from the leaf to upper nodes.
 *
 * The 'new_child' is the pointer of newly genereated child by split,
 * which points to the node which has the 'copied_up_key'. Similarly,
 * 'new_child_index' is the index where 'new_child' should be inserted.
 * Those two parameters are set by recursive call only.
 *
 * Note that on the leaf nodes, the new key-value pair is inserted
 * at aligned index position.
 */
void
bpt_insert_internal(bpt_tree *t, bpt_node *curr_node, void *new_key,
		    void *new_value, int new_child_index, void *new_child){
    assert(t != NULL);
    assert(curr_node != NULL);
    assert(new_key != NULL);

    /* Does this node have room to store a new key ? */
    if (ll_get_length(curr_node->keys) < t->m){
	curr_node->n_keys++;
	if (curr_node->is_leaf){
	    printf("debug : Add key = %lu to node (%p)\n",
		   (uintptr_t) new_key, curr_node);
	    ll_asc_insert(curr_node->keys, new_key);
	    ll_tail_insert(curr_node->children, new_child);
	}else{
	    /* Get a copied up key from lower node */
	    printf("debug : (%p) curr_node->children[%d] = child\n",
		   curr_node,new_child_index);
	    ll_asc_insert(curr_node->keys, new_key);
	    ll_index_insert(curr_node->children, new_child, new_child_index);
	}
    }else{
	bpt_node *right_half;
	void *copied_up_key = NULL;

	printf("*split scenario for %lu*\n", (uintptr_t) new_key);

	/* Add the new key */
	ll_asc_insert(curr_node->keys, new_key);
	if (curr_node->is_leaf == true)
	    ll_asc_insert(curr_node->children, new_value);
	else
	    ll_index_insert(curr_node->children, new_child, new_child_index);

	/* Split keys and children */
	right_half = bpt_node_split(t, curr_node);

	printf("debug : left (%d len : %p) => \n",
	       ll_get_length(curr_node->keys), curr_node);
	bpt_dump_list(curr_node->keys);
	printf("debug : right (%d len : %p) => \n",
	       ll_get_length(right_half->keys), right_half);
	bpt_dump_list(right_half->keys);

	/* Get the key that will go up and/or will be deleted */
	copied_up_key = bpt_get_copied_up_key(right_half->keys);

	if (curr_node->is_leaf == true){
	    /* Connect split leaf nodes */
	    right_half->next = curr_node->next;
	    curr_node->next = right_half;
	}else{
	    /*
	     * Delete the copied up key and the corresponding child
	     * if this node is one of upper nodes.
	     */
	    (void) ll_get_first_node(right_half->keys);
	    (void) ll_get_first_node(right_half->children);
	    printf("Removed internal node's key = %lu. Left key num = %d\n",
		   (uintptr_t) copied_up_key, ll_get_length(right_half->keys));
	}

	if (!curr_node->parent){
	    /* Create the new root */
	    bpt_node *new_top;

	    assert(curr_node->is_root == true);
	    assert(right_half->is_root == true);
	    assert(right_half->parent == NULL);

	    new_top = bpt_gen_root_callbacks_node(t);
	    new_top->is_root = true;
	    new_top->is_leaf = curr_node->is_root = right_half->is_root = false;
	    curr_node->parent = right_half->parent = t->root = new_top;

	    ll_asc_insert(new_top->keys, copied_up_key);

	    /* Arrage the order of children */
	    printf("Created a new root with key = %lu\n",
		   (uintptr_t) copied_up_key);
	    ll_tail_insert(new_top->children, curr_node);
	    ll_tail_insert(new_top->children, right_half);
	}else{
	    /*
	     * Propagate the key insertion to the upper node. Notify the
	     * upper node of the index to insert a new split right child.
	     *
	     * To determine the new child index, we will iterate all children
	     * that the current node's parent stores. The new child should be
	     * placed right after the current node.
	     */
	    int index;

	    for (index = 0; index < ll_get_length(curr_node->parent->children); index++)
		if (curr_node == ll_get_index_node(curr_node->parent->children, index))
		    break;

	    printf("Recursive call of bpt_insert() with key = %lu\n",
		   (uintptr_t) copied_up_key);

	    bpt_insert_internal(t, curr_node->parent,
				copied_up_key, NULL, index + 1, right_half);
	}
    }
}

bool
bpt_insert(bpt_tree *t, void *new_key, void *new_data){
    bpt_node *last_node;
    bool found_same_key = false;

    found_same_key = bpt_search(t->root, new_key, &last_node);

    assert(last_node->is_leaf == true);

    /* Prohibit duplicate keys */
    if (found_same_key)
	return false;
    else{
	bpt_insert_internal(t, last_node, new_key, new_data, 0, NULL);

	return true;
    }
}

bool
bpt_search(bpt_node *curr_node, void *new_key, bpt_node **last_explored_node){
    linked_list *curr_keys;
    void *existing_key;
    int iter, diff, children_index;

    printf("debug : bpt_search() for key = %lu in node '%p'\n",
	   (uintptr_t) new_key, curr_node);

    /*
     * If recursive call of bpt_search() receives NULL 'curr_node',
     * it means node's children contained NULL value and we tried
     * to search it. Return false.
     */
    if (curr_node == NULL)
	return false;

    /* Set the last searched node. This search can be last */
    *last_explored_node = curr_node;

    /*
     * Iterate each bpt's key and compare it with the new key.
     * Search for an exact match or find the first smaller value than
     * the new key user indicated.
     */
    children_index = 0;
    curr_keys = curr_node->keys;

    for (iter = 0; iter < ll_get_length(curr_keys); iter++){

	/* We might hit NULL during key iteration. Skip NULL. */
	if ((existing_key = ll_get_index_node(curr_keys, iter)) == NULL)
	    continue;

	diff = curr_keys->key_compare_cb(curr_keys->key_access_cb(existing_key),
					 curr_keys->key_access_cb(new_key));
	/*
	 * Two keys are equal or the existing key is larger than new key.
	 * The latter means we can insert the 'new_key' before the larger
	 * existing key. Break now.
	 */
	if (diff == 0 || diff == 1)
	    break;

	children_index++;
    }

    if (diff == 0){
	/* Exact key match at the leaf node level */
	if (curr_node->is_leaf){
	    return true;
	}else{
	    /* Search for the leaf node */
	    return bpt_search((bpt_node *)
			      ll_get_index_node(curr_node->children, children_index + 1),
			      new_key, last_explored_node);
	}
    }else if (diff == 1){
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
    printf("did not found smaller or equal key value than %lu\n",
	   (uintptr_t) new_key);

    /*
     * If this node has no children, then search failure.
     * Otherwise, run a recursive call for the rightmost node.
     */
    if (curr_node->is_leaf)
	return false;
    else{
	printf("debug : bpt_search() will access to the %lu index child\n",
	       (uintptr_t) ll_get_length(curr_node->children) - 1);

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
