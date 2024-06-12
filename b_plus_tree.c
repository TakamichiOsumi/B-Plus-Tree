#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Linked-List/linked_list.h"

#include "b_plus_tree.h"

#define HAVE_SAME_PARENT(n1, n2) \
    (n1 != NULL && n2 != NULL && n1->parent == n2->parent)

static void*
bpt_malloc(size_t size){
    void *p;

    if ((p = malloc(size)) == NULL){
	perror("malloc");
	exit(-1);
    }

    return p;
}

/* static */ void
bpt_dump_list(linked_list *list){
    void *p;

    ll_begin_iter(list);
    while((p = ll_get_iter_data(list)) != NULL){
	printf("\t dump : %lu\n", (uintptr_t) p);
    }
    ll_end_iter(list);
}

static void
bpt_dump_children_keys(bpt_node *curr_node){
    bpt_node *child;
    void *key;

    if (curr_node->is_leaf)
	return;

    ll_begin_iter(curr_node->children);
    while((child = (bpt_node *) ll_get_iter_data(curr_node->children)) != NULL){
	printf("\t\t [ ");
	ll_begin_iter(child->keys);
	while((key = ll_get_iter_data(child->keys)) != NULL){
	    printf("%lu, ", (uintptr_t) key);
	}
	ll_end_iter(child->keys);
	printf("]\n");
    }
    ll_end_iter(curr_node->children);
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
    node->parent = node->prev = node->next = NULL;

    return node;
}

/*
 * Create a new empty node that copies
 * tree's root all callbacks for keys and children.
 *
 * Yet, both lists are returned as empty.
 */
bpt_node *
bpt_gen_root_callbacks_node(bpt_tree *bpt){
    bpt_node *n;

    n = bpt_gen_node();
    n->keys = ll_init(bpt->root->keys->key_access_cb,
		      bpt->root->keys->key_compare_cb,
		      bpt->root->keys->free_cb);
    n->children = ll_init(bpt->root->children->key_access_cb,
			  bpt->root->children->key_compare_cb,
			  bpt->root->children->free_cb);

    return n;
}

bpt_tree *
bpt_init(bpt_key_access_cb keys_key_access,
	 bpt_key_compare_cb keys_key_compare,
	 bpt_free_cb keys_key_free,
	 bpt_key_access_cb children_key_access,
	 bpt_key_compare_cb children_key_compare,
	 bpt_free_cb children_key_free, uint16_t max_children){
    bpt_tree *tree;

    if(max_children < 3){
	fprintf(stderr,
		"the number of max children should be larger than three\n");
	return NULL;
    }

    if (keys_key_access == NULL || keys_key_compare == NULL){
	fprintf(stderr,
		"NULL 'keys_key_access' or 'keys_key_compare' callback is invalid\n");
	return NULL;
    }

    tree = (bpt_tree *) bpt_malloc(sizeof(bpt_tree));
    tree->max_children = max_children;

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
 * ll_split() returns the *left* half, so swap the return value within
 * this function.
 *
 * This is required to connect the split nodes with other existing nodes
 * before and after the two split nodes.
 */
static bpt_node *
bpt_node_split(bpt_tree *bpt, bpt_node *curr_node){
    bpt_node *half;
    linked_list *left_keys, *left_children;
    int node_num = (bpt->max_children % 2 == 0) ? (bpt->max_children / 2) :
	(bpt->max_children / 2) + 1;

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
     * At this moment, the 'half' node has left part of
     * keys and children.
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
    first_key = ll_get_iter_data(keys);
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
static void
bpt_insert_internal(bpt_tree *bpt, bpt_node *curr_node, void *new_key,
		    void *new_value, int new_child_index, void *new_child){
    assert(bpt != NULL);
    assert(curr_node != NULL);
    assert(new_key != NULL);

    /* Does this node have room to store a new key ? */
    if (ll_get_length(curr_node->keys) < bpt->max_children){
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
	/*
	 * We have the maximum number of children in this node already. So,
	 * adding a new key-value exeeds the limit. Split the current node,
	 * distribute the keys and children stored there.
	 */
	bpt_node *right_half, *child;
	void *copied_up_key = NULL;

	printf("*split for %lu*\n", (uintptr_t) new_key);

	/* Add the new key */
	ll_asc_insert(curr_node->keys, new_key);
	if (curr_node->is_leaf == true)
	    ll_asc_insert(curr_node->children, new_value);
	else
	    ll_index_insert(curr_node->children, new_child, new_child_index);

	/* Split keys and children */
	right_half = bpt_node_split(bpt, curr_node);

	printf("debug : left (%d len : %p) => \n", ll_get_length(curr_node->keys),
	       curr_node);
	bpt_dump_list(curr_node->keys);
	printf("debug : right (%d len : %p) => \n", ll_get_length(right_half->keys),
	       right_half);
	bpt_dump_list(right_half->keys);

	/* Get the key that will go up and/or will be deleted */
	copied_up_key = bpt_get_copied_up_key(right_half->keys);

	/*
	 * Connect split nodes at the same depth.
	 *
	 * When there is other node on the right side of 'right_half', make
	 * its 'prev' point to the 'right_half'. Skip if the 'right_half' is
	 * the rightmost node.
	 */
	right_half->prev = curr_node;
	right_half->next = curr_node->next;
	curr_node->next = right_half;
	if (right_half->next != NULL)
	    right_half->next->prev = right_half;

	if (!curr_node->is_leaf){
	    void *p;

	    /* Delete the copied up key from the internal node */
	    (void) ll_remove_first_data(right_half->keys);
	    assert((p = ll_remove_first_data(right_half->children)) == NULL);
	    printf("debug : removed internal node's key = %lu. Left key num = %d\n",
		   (uintptr_t) copied_up_key, ll_get_length(right_half->keys));

	    /*
	     * After split, all children still point to the left split node.
	     * Then, update the parent member of all of the right half children.
	     */
	    ll_begin_iter(right_half->children);
	    while((child = (bpt_node *) ll_get_iter_data(right_half->children)) != NULL){
		child->parent = right_half;
	    }
	    ll_end_iter(right_half->children);

	    /* Use the utility for debug */
	    printf("debug : after split, dump children's keys\n");
	    printf("Left internal node's children :\n");
	    bpt_dump_children_keys(curr_node);
	    printf("Right internal node's children :\n");
	    bpt_dump_children_keys(right_half);
	}

	if (!curr_node->parent){
	    /* Create a new root */
	    bpt_node *new_top;

	    assert(curr_node->is_root == true);
	    assert(right_half->is_root == true);
	    assert(right_half->parent == NULL);

	    new_top = bpt_gen_root_callbacks_node(bpt);
	    new_top->is_root = true;
	    new_top->is_leaf = curr_node->is_root = right_half->is_root = false;
	    curr_node->parent = right_half->parent = bpt->root = new_top;

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
	     * To determine the new child index, we will iterate all children
	     * that the current node's parent stores. The new child should be
	     * placed right after the current node.
	     */
	    linked_list *parent_children = curr_node->parent->children;
	    int index = 0;

	    ll_begin_iter(parent_children);
	    for (; index < ll_get_length(parent_children); index++)
		if (curr_node == ll_get_iter_data(parent_children))
		    break;
	    ll_end_iter(parent_children);

	    printf("Recursive call of bpt_insert() with key = %lu\n",
		   (uintptr_t) copied_up_key);

	    bpt_insert_internal(bpt, curr_node->parent,
				copied_up_key, NULL, index + 1, right_half);
	}
    }
}

bool
bpt_insert(bpt_tree *bpt, void *new_key, void *new_data){
    bpt_node *leaf_node;
    bool found_same_key = false;

    found_same_key = bpt_search(bpt, new_key, &leaf_node);

    /* Prohibit duplicate keys */
    if (found_same_key)
	return false;
    else{
	bpt_insert_internal(bpt, leaf_node, new_key, new_data, 0, NULL);

	return true;
    }
}

static bpt_node *
bpt_ref_index_child(bpt_node *curr_node, int index){
    return (bpt_node *) ll_ref_index_data(curr_node->children, index);
}

static bool
bpt_search_internal(bpt_node *curr_node, void *new_key, bpt_node **leaf_node){
    linked_list *keys;
    void *existing_key;
    int iter, diff, children_index;

    printf("debug : bpt_search() for key = %lu in node '%p'\n",
	   (uintptr_t) new_key, curr_node);

    /* Set the last searched node. This search can be last */
    *leaf_node = curr_node;

    /*
     * Iterate each bpt's key and compare it with the new key.
     * Search for an exact match or find the first smaller value than
     * the new key user indicated. The latter means we can insert
     * the 'new_key' before the larger existing key.
     */
    children_index = 0;
    keys = curr_node->keys;

    ll_begin_iter(keys);
    for (iter = 0; iter < ll_get_length(keys); iter++){
	existing_key = ll_get_iter_data(keys);

	diff = keys->key_compare_cb(keys->key_access_cb(existing_key),
				    keys->key_access_cb(new_key));
	if (diff == 0 || diff == 1)
	    break;

	children_index++;
    }
    ll_end_iter(keys);

    if (diff == 0){
	/* Exact key match */
	if (curr_node->is_leaf){
	    printf("debug : bpt_search_internal() found the same key in leaf node\n");

	    return true;
	}else{
	    /* Search for the right child */
	    return bpt_search_internal(bpt_ref_index_child(curr_node, children_index + 1),
				       new_key, leaf_node);
	}
    }else if (diff == 1){
	/* Found larger key value than the 'new_key' */
	if (curr_node->is_leaf)
	    return false;
	else{
	    /* Search for the left child */
	    return bpt_search_internal(bpt_ref_index_child(curr_node, children_index),
				       new_key, leaf_node);
	}
    }else{
	/* The key was bigger than all the existing keys */
	if (curr_node->is_leaf)
	    return false;
	else{
	    /* Search for the rightmost child */
	    return bpt_search_internal(bpt_ref_index_child(curr_node,
							   ll_get_length(curr_node->children) - 1),
				       new_key, leaf_node);
	}
    }
}

/*
 * Wrapper function of bpt_search_internal().
 */
bool
bpt_search(bpt_tree *bpt, void* new_key, bpt_node **leaf_node){

    if (bpt != NULL && bpt->root != NULL && new_key != NULL)
	return bpt_search_internal(bpt->root, new_key, leaf_node);

    return false;
}

/*
 * Return the minimum key from one subtree.
 *
 * Go down the subtree until we reach the leftmost child of the leaf node.
 */
static void *
bpt_ref_subtree_minimum_key(bpt_node *node){
    while(!node->is_leaf)
	node = (bpt_node *) node->children->head->data;

    return node->keys->head->data;
}

static bpt_node *
bpt_ref_right_child_by_key(bpt_node *node, void *key){
    bpt_node *key_iter, *child_iter, *right_child;

    ll_begin_iter(node->keys);
    ll_begin_iter(node->children);
    while(true){
	key_iter = ll_get_iter_data(node->keys);
	child_iter = ll_get_iter_data(node->children);

	if (node->keys->key_compare_cb(key_iter, key) == 0)
	    break;
    }
    /* The right child for the key is the next pointer */
    right_child = (bpt_node *) ll_get_iter_data(node->children);
    assert(right_child != NULL);
    ll_end_iter(node->keys);
    ll_end_iter(node->children);

    return right_child;
}

/*
 * Don't expect children can perform 'key_access_cb' and 'key_compare_cb'.
 * So, merging children uses some basic APIs for linked list such as
 * ll_tail_insert() or ll_remove_first_data().
 *
 * Update the curr_node's parent's 'keys' and 'children' according to
 * the merge.
 */
static void
bpt_merge_nodes(bpt_node *curr_node, bool with_right){
    linked_list *merged_keys, *merged_children;
    bpt_node *curr_child, *removed_child = NULL;
    node *node;
    void *deleted_key;
    int index = 0;

    printf("debug : bpt_merge_nodes() with %s node\n", with_right ? "right" : "left");

    if (with_right){
	/* Merge keys */
	merged_keys = ll_merge(curr_node->keys, curr_node->next->keys);

	/* Merge children of current node with the ones of the next node */
	while(ll_get_length(curr_node->next->children) > 0)
	    ll_tail_insert(curr_node->children,
			   ll_remove_first_data(curr_node->next->children));
	merged_children = curr_node->children;
	node = curr_node->parent->children->head;
    }else{
	/* Merge keys */
	merged_keys = ll_merge(curr_node->prev->keys, curr_node->keys);

	/* Merge children of the prev node and ones of curr_node */
	while(ll_get_length(curr_node->children) > 0)
	    ll_tail_insert(curr_node->prev->children,
			   ll_remove_first_data(curr_node->children));
	merged_children = curr_node->prev->children;
	node = curr_node->prev->parent->children->head;
    }

    /*
     * Now 'node' points to the merged children. Search for the removed child
     * and get the index of the child to pass as an argument of ll_index_remove().
     */
    while(true){
	curr_child = (bpt_node *) node->data;

	if (with_right && curr_child == curr_node){
	    /* Remove the next node of the current node */
	    printf("debug : found the curr_node to merge with right child\n");
	    removed_child = curr_child->next;
	    if (curr_node->next->next)
		curr_node->next->next->prev = curr_node;
	    curr_node->next = curr_node->next->next;
	    break;
	}else if (!with_right && curr_child == curr_node->prev){
	    /* Remove the current node */
	    printf("debug : found the prev node to merge with the current node\n");
	    removed_child = curr_node;
	    if (curr_node->next)
		curr_node->next->prev = curr_node->prev;
	    curr_node->prev->next = curr_node->next;
	    break;
	}

	/* Iterate all children */
	if ((node = node->next) == NULL)
	    break;
	index++;
    }

    /* Remove a parent's key which has become unnecessary by merge */
    assert((deleted_key = ll_index_remove(curr_node->parent->keys, index)) != NULL);
    /* Detach the removed child from the parent children as well. */
    assert(ll_index_remove(curr_node->parent->children, index + 1) != NULL);

    printf("debug : merging nodes removed key = %lu at index = %d in %s parent node\n",
	   (uintptr_t) deleted_key, index, curr_node->parent->is_root ? "root" : "non-root");

    /* Reconnect all the merged results */
    if (with_right){
	curr_node->keys = merged_keys;
	curr_node->children = merged_children;
    }else{
	curr_node->prev->keys = merged_keys;
	curr_node->prev->children = merged_children;
    }

    /* The removed child must be empty */
    assert(ll_get_length(removed_child->keys) == 0);
    assert(ll_get_length(removed_child->children) == 0);

    /* The parent must delete the pointer to the removed child */
    ll_begin_iter(curr_node->parent->children);
    while((curr_child = ll_get_iter_data(curr_node->parent->children)) != NULL){
	if (curr_child == removed_child)
	    assert(0);
    }
    ll_end_iter(curr_node->parent->children);

    printf("debug : merging updates number of parent's keys to %d\n", ll_get_length(curr_node->parent->keys));
}

static void
bpt_replace_index(bpt_node *curr_node, bool from_right){
    void *prev_index, *replaced_index;
    void *child;
    void *key;
    bpt_node *right_child;

    /*
     * Iterate key and child simultaneously.
     *
     * When we find the pointer of 'curr_node', we can tell which
     * key to remove from the parent's node.
     */
    ll_begin_iter(curr_node->parent->keys);
    ll_begin_iter(curr_node->parent->children);
    prev_index = replaced_index = ll_get_iter_data(curr_node->parent->keys);
    child = ll_get_iter_data(curr_node->parent->children);
    while(true){
	if (from_right && child == curr_node){
	    /*
	     * If we borrow from the right child, then the
	     * current 'replaced_index' should be removed.
	     */
	    right_child = ll_get_iter_data(curr_node->parent->children);
	    break;
	}else if (!from_right && child == curr_node){
	    /*
	     * If we borrow from the left child, then the
	     * previous key, 'prev_index' should be removed.
	     */
	    replaced_index = prev_index;
	    right_child = child;
	    break;
	}
	prev_index = replaced_index;
	replaced_index = ll_get_iter_data(curr_node->parent->keys);
	child = ll_get_iter_data(curr_node->parent->children);
    }
    ll_end_iter(curr_node->parent->keys);
    ll_end_iter(curr_node->parent->children);

    assert(ll_remove_by_key(curr_node->parent->keys, replaced_index) != NULL);
    curr_node->parent->n_keys--;
    assert((key = bpt_ref_subtree_minimum_key(right_child)) != NULL);
    ll_asc_insert(curr_node->parent->keys, key);

    printf("debug : index key = %lu was replace with %lu\n",
	   (uintptr_t) replaced_index, (uintptr_t) key);
}

static bpt_node*
bpt_ref_key_between_children(bpt_node *left, bpt_node *right){
    void *key, *child;

    assert(left->parent != NULL);
    assert(right->parent != NULL);
    assert(left->parent == right->parent);

    ll_begin_iter(left->parent->keys);
    ll_begin_iter(left->parent->children);
    key = ll_get_iter_data(left->parent->keys);
    child = ll_get_iter_data(left->parent->children);
    while(true){
	if (child == left)
	    break;
    }
    assert(right == ll_get_iter_data(left->parent->children));
    ll_end_iter(left->parent->keys);
    ll_end_iter(left->parent->children);

    return key;
}

static void
bpt_delete_internal(bpt_tree *bpt, bpt_node *curr_node, void *removed_key){
    int min_key_num;

    min_key_num = (bpt->max_children % 2 == 0) ?
	(bpt->max_children % 2 - 1) : (bpt->max_children % 2);

    printf("debug : bpt_delete_internal() for %p\n"
           "debug : current node = %s and %s, the number of keys = %d, the number of children = %d\n",
	   curr_node,
	   curr_node->is_root ? "root" : "non-root",
	   curr_node->is_leaf ? "leaf" : "non-leaf",
	   ll_get_length(curr_node->keys), ll_get_length(curr_node->children));

    /* Need a root promotion ? */
    if (curr_node->is_root && ll_get_length(curr_node->children) == 1){
	bpt_node *child;

	assert(curr_node->prev == NULL);
	assert(curr_node->next == NULL);

	child = ll_remove_first_data(curr_node->children);
	child->is_root = true;
	child->parent = NULL;
	bpt->root = child;

	ll_destroy(curr_node->keys);
	ll_destroy(curr_node->children);

	free(curr_node);

	printf("debug : completed root promotion\n");

	return;
    }

    /* The other form of root promotion, which shrinks tree's height by merge */
    if (!curr_node->is_root &&
	ll_get_length(curr_node->keys) == 0 &&
	ll_get_length(curr_node->children) == 1){
	/*
	 * The previous call of bpt_delete_internal() has merged the last two
	 * children of this node and deleted one left index in between. We must
	 * ensure that we don't have any keys left in this node and that in this
	 * scenario, we have a parent that stores one key.
	 */
	assert(curr_node->parent != NULL);
	assert(curr_node->parent->is_root == true);
	assert(ll_get_length(curr_node->parent->keys) == 1);

	if (HAVE_SAME_PARENT(curr_node->prev, curr_node)){
	    bpt_node *last_child;
	    void *min_key;

	    /*
	     * Move the current node's children to the previous node.
	     * Meanwhile, it's possible that the parent's key can't be
	     * used as is. In such cases, obtain the minimum value from the
	     * right child and utilize it as the key value added to the
	     * previous node. This ensures indexes are stored correctly.
	     */
	    last_child = ll_remove_first_data(curr_node->children);
	    min_key = bpt_ref_subtree_minimum_key(last_child);
	    ll_asc_insert(curr_node->prev->keys, min_key);
	    ll_tail_insert(curr_node->prev->children, last_child);

	    printf("debug : the tree height has shrunk, with the insertion of the new key = %lu\n",
		   (uintptr_t) min_key);

	    /* Reconnect nodes. The previous node will become the new root */
	    bpt->root = curr_node->prev;
	    curr_node->prev->is_root = true;
	    curr_node->prev->next = NULL;
	    curr_node->prev->parent = NULL;

	    /* Free the current root and the current node */
	    ll_destroy(curr_node->parent->keys);
	    ll_destroy(curr_node->parent->children);
	    free(curr_node->parent);

	    curr_node->prev = NULL;
	    curr_node->parent = NULL;
	    ll_destroy(curr_node->keys);
	    ll_destroy(curr_node->children);
	    free(curr_node);

	    /* Done with the key deletion */
	    return;
	}

	if (HAVE_SAME_PARENT(curr_node, curr_node->next)){
	    void *key;

	    /*
	     * In this scenario of tree height shrink, we can just move the
	     * last parent's key and the current child node to the next node.
	     * This is because we ensure that the parent key is bigger than
	     * any other keys in this node or any other child nodes of this
	     * node. Utilizing the parent's key can keep the entire order of
	     * indexes and children.
	     */
	    key = ll_remove_first_data(curr_node->parent->keys);
	    ll_asc_insert(curr_node->next->keys, key);
	    ll_insert(curr_node->next->children,
		      ll_remove_first_data(curr_node->children));

	    printf("debug : the tree height has shrunk, with the insertion of the new key = %lu\n",
		   (uintptr_t) key);

	    /* Reconnect nodes. The next node will become the new root */
	    bpt->root = curr_node->next;
	    curr_node->next->is_root = true;
	    curr_node->next->prev = NULL;
	    curr_node->next->parent = NULL;

	    /* Free the current root and the current node */
	    ll_destroy(curr_node->parent->keys);
	    ll_destroy(curr_node->parent->children);
	    free(curr_node->parent);

	    curr_node->next = NULL;
	    curr_node->parent = NULL;
	    ll_destroy(curr_node->keys);
	    ll_destroy(curr_node->children);
	    free(curr_node);

	    /* Done with the key deletion */
	    return;
	}
    }

    /*
     * The main part of the deletion process
     */
    if (((int) ll_get_length(curr_node->keys)) - 1 >= min_key_num){
	if (curr_node->is_leaf){
	    /*
	     * If this is a leaf node, it must contain the removed key, because
	     * bpt_search() before the first call of bpt_delete_internal() already
	     * proves the key exists.
	     */
	    assert(ll_remove_by_key(curr_node->keys, removed_key) != NULL);
	    curr_node->n_keys--;
	    printf("debug : key = '%lu' was removed on the leaf node\n", (uintptr_t) removed_key);
	    /* TODO : Remove the record as well */
	}else{
	    /* This internal node might or might not have the key */
	    if (ll_has_key(curr_node->keys, removed_key)){
		/* Replace the index with the right child's minimum key */
		void *key;
		bpt_node *right_child;

		right_child = bpt_ref_right_child_by_key(curr_node, removed_key);
		ll_remove_by_key(curr_node->keys, removed_key);
		curr_node->n_keys--;
		assert((key = bpt_ref_subtree_minimum_key(right_child)) != NULL);
		printf("debug : %lu was removed and %lu was inserted as the min key\n",
		       (uintptr_t) removed_key, (uintptr_t) key);
		ll_asc_insert(curr_node->keys, key);
	    }
	}

	/* Continue to update the indexes. Go up by recursive call */
	if (!curr_node->is_root)
	    bpt_delete_internal(bpt, curr_node->parent, removed_key);

    }else{
	bool has_left_sibling = false, borrowed_from_left = false,
	    has_right_sibling = false, borrowed_from_right = false;
	void *borrowed_key;

	if (curr_node->is_leaf){
	    /* This leaf node must contain the removed key */
	    assert(ll_remove_by_key(curr_node->keys, removed_key) != NULL);
	    curr_node->n_keys--;
	    printf("debug : %lu was removed on the leaf node\n", (uintptr_t) removed_key);
	    /* TODO : Remove the record as well */
	}else{
	    /* This internal node may not have the removed key */
	    if (ll_has_key(curr_node->keys, removed_key) == true){
		/* Replace the index with right child's minimum key */
		void *key;
		bpt_node *right_child;

		right_child = bpt_ref_right_child_by_key(curr_node, removed_key);
		ll_remove_by_key(curr_node->keys, removed_key);
		curr_node->n_keys--;
		assert((key = bpt_ref_subtree_minimum_key(right_child)) != NULL);
		ll_asc_insert(curr_node->keys, key);

		printf("debug : %lu was removed and %lu was inserted as the min key\n",
		       (uintptr_t) removed_key, (uintptr_t) key);

		if (curr_node->parent)
		    bpt_delete_internal(bpt, curr_node->parent, removed_key);

		return;
	    }
	}

	/* Is the left node an available sibling to borrow a key ? */
	if (HAVE_SAME_PARENT(curr_node, curr_node->prev)){
	    assert(curr_node->parent != NULL);
	    has_left_sibling = true;
	    if (((int) ll_get_length(curr_node->prev->keys)) - 1 >= min_key_num){

		borrowed_from_left = true;
		printf("*borrowing from the left node*\n");
		printf("debug : the number of current node's keys = %d\n",
		       ll_get_length(curr_node->keys));

		if (curr_node->is_leaf){
		    borrowed_key = ll_tail_remove(curr_node->prev->keys);
		    ll_insert(curr_node->keys, borrowed_key);
		    ll_insert(curr_node->children, ll_tail_remove(curr_node->prev->children));

		    printf("debug : borrowed the max key = %lu from the left sibling\n",
			   (uintptr_t) borrowed_key);

		    /* Update the parent's index */
		    bpt_replace_index(curr_node, false);
		}else{
		    void *middle_key, *largest_key;
		    bpt_node *borrowed_child;

		    /*
		     * If this internal node borrows a key from other node, then get
		     * the intermediate parent's key as the new key value for this node,
		     * and remove it from the parent and write the largest key as the
		     * new parent's key.
		     */
		    assert((largest_key = ll_tail_remove(curr_node->prev->keys)) != NULL);
		    middle_key = bpt_ref_key_between_children(curr_node->prev, curr_node);

		    ll_asc_insert(curr_node->keys, middle_key);
		    assert(ll_remove_by_key(curr_node->parent->keys, middle_key) != NULL);
		    ll_asc_insert(curr_node->parent->keys, largest_key);

		    printf("[FROM] Left internal node's children :\n");
		    bpt_dump_list(curr_node->prev->keys);
		    printf("[FROM] Right internal nodes's children :\n");
		    bpt_dump_list(curr_node->keys);

		    /* Whenever we borrow an internal node's child, update its attributes */
		    assert((borrowed_child = (bpt_node *) ll_tail_remove(curr_node->prev->children)) != NULL);
		    borrowed_child->parent = curr_node;
		    ll_insert(curr_node->children, (void *) borrowed_child);

		    printf("debug : the new current node's key = %lu, new parent's index key = %lu\n",
			   (uintptr_t) middle_key, (uintptr_t) largest_key);
		}
	    }
	}

	/* The left node wasn't available. How about the right sibling ? */
	if (!borrowed_from_left && HAVE_SAME_PARENT(curr_node, curr_node->next)){
	    assert(curr_node->parent != NULL);
	    has_right_sibling = true;
	    if (((int) ll_get_length(curr_node->next->keys)) - 1 >= min_key_num){

		borrowed_from_right = true;
		printf("*borrowing from the right node*\n");

		printf("debug : the number of current node's keys = %d\n",
		       ll_get_length(curr_node->keys));

		if (curr_node->is_leaf){
		    borrowed_key = ll_remove_first_data(curr_node->next->keys);
		    ll_tail_insert(curr_node->keys, borrowed_key);
		    ll_tail_insert(curr_node->children,
				   ll_remove_first_data(curr_node->next->children));

		    printf("debug : borrowed the min key = %lu from the right sibling\n",
			   (uintptr_t) borrowed_key);

		    /* Update the parent's index */
		    bpt_replace_index(curr_node, true);
		}else{
		    void *middle_key, *smallest_key;
		    bpt_node *borrowed_child;

		    assert((smallest_key = ll_remove_first_data(curr_node->next->keys)) != NULL);
		    middle_key = bpt_ref_key_between_children(curr_node, curr_node->next);

		    ll_asc_insert(curr_node->keys, middle_key);
		    assert(ll_remove_by_key(curr_node->parent->keys, middle_key) != NULL);
		    ll_asc_insert(curr_node->parent->keys, smallest_key);

		    printf("[FROM] Right internal node's children :\n");
		    bpt_dump_list(curr_node->next->keys);
		    printf("[TO] Left internal nodes's children :\n");
		    bpt_dump_list(curr_node->keys);

		    /* Whenever we borrow an internal node's child, update its attributes */
		    assert((borrowed_child = (bpt_node *) ll_remove_first_data(curr_node->next->children)) != NULL);
		    borrowed_child->parent = curr_node;
		    ll_tail_insert(curr_node->children, borrowed_child);

		    printf("debug : the new current node's key = %lu, new parent's index key = %lu\n",
			   (uintptr_t) middle_key, (uintptr_t) smallest_key);
		}
	    }
	}

	if (borrowed_from_left == false && borrowed_from_right == false){
	    /* Merge nodes if possible */
	    printf("debug : borrowing a key didn't happen\n");
	    if (has_left_sibling){
		bpt_merge_nodes(curr_node, false);
		printf("debug : merged with the left node\n");
		/*
		 * Note : Don't interact with the curr_node's 'keys'
		 * and 'children' from here. It's already merged with
		 * the previous node, although other attributes are fine.
		 */
	    }else if (has_right_sibling){
		bpt_merge_nodes(curr_node, true);
		printf("debug : merged with the right node\n");
	    }else{
		printf("debug : merging nodes didn't happen\n");
	    }
	}

	/* Continue to update the indexes. Go up by recursive call */
	if (!curr_node->is_root)
	    bpt_delete_internal(bpt, curr_node->parent, removed_key);
    }
}

bool
bpt_delete(bpt_tree *bpt, void *key){
    bpt_node *leaf_node;
    bool found_same_key = false;

    printf("debug : bpt_delete() for root = %p\n", bpt->root);

    found_same_key = bpt_search(bpt, key, &leaf_node);

    /* Remove the found key */
    if (found_same_key){
	bpt_delete_internal(bpt, leaf_node, key);

	return true;
    }else{
	printf("debug : the key to be removed was not found\n");

	return false;
    }
}

void
bpt_destroy(bpt_tree *bpt){}
