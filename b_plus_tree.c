#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Linked-List/linked_list.h"
#include "b_plus_tree.h"

/* Macros for b+ tree node */
#define HAVE_SAME_PARENT(n1, n2) \
    (n1 && n2 && n1->parent == n2->parent)

/* Macros for key numbers */
#define KEY_LEN(n) (ll_get_length(n->keys))
#define GET_MIN_KEY_NUM(max_keys)				\
    ((max_keys % 2 == 0) ? (max_keys / 2 - 1) : (max_keys / 2))

/* Macros for children numbers */
#define CHILDREN_LEN(n) (ll_get_length(n->children))
#define GET_MAX_CHILDREN_NUM(max_keys) (max_keys + 1)
#define GET_MIN_CHILDREN_NUM(max_keys)				\
    ((max_keys % 2 == 0) ? (max_keys / 2) : (max_keys / 2 + 1))

/* Macros for iteration */
#define ITER_BPT_KEY(node)			\
    ll_get_iter_data(node->keys)
#define ITER_BPT_CHILD(node)				\
    ((bpt_node *) ll_get_iter_data(node->children))

/*
 * Return the bpt_node * child from the node by specified index
 * value.
 */
static bpt_node *
bpt_ref_index_child(bpt_node *curr, int index){
    assert(curr->is_leaf == false);

    return ((bpt_node *) ll_ref_index_data(curr->children, index));
}

/*
 * Insert this function for any node updates to check whether it
 * satisfies either valid condition below.
 *
 * The node has the same numbers of keys and values if it's a
 * leaf node. Or, the node has one more indexes than children
 * if it's a root or internal node.
 */
void
bpt_node_validity(bpt_node *node){
    if (node->is_leaf)
	assert(ll_get_length(node->keys) == ll_get_length(node->children));
    else
	assert(ll_get_length(node->keys) + 1 == ll_get_length(node->children));
}

/*
 * Dump the entire tree keys from the top to the bottom.
 */
void
bpt_dump_whole_tree(bpt_tree *bpt){
    bpt_node *leftmost, *curr;
    int i;

    if (bpt == NULL || bpt->root == NULL)
	return;

    curr = bpt->root;

    while(curr != NULL){
	/* Remember the leftmost node */
	leftmost = curr->is_leaf ? NULL : bpt_ref_index_child(curr, 0);

	/* Dump keys of all vertical nodes */
	printf("debug : ");
	while(true){
	    printf("[");
	    ll_begin_iter(curr->keys);
	    for (i = 0; i < KEY_LEN(curr); i++){
		printf("%lu, ",
		       (uintptr_t) ll_get_iter_data(curr->keys));
	    }
	    ll_end_iter(curr->keys);
	    printf("] ");

	    curr = curr->next;
	    if (curr == NULL)
		break;
	}

	/* Iterate the children */
	curr = leftmost;
	printf("\n");
    }
}

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
bpt_free_node(bpt_node *node){
    if (node != NULL){
	ll_destroy(node->keys);
	ll_destroy(node->children);

	printf("debug : free node = %p\n", node);
	free(node);
    }
}

/* Dump one list with its length */
static void
bpt_dump_list(char *prefix, linked_list *list){
    void *p;

    printf("debug : %s", prefix);
    printf("debug : list length = %d\n",
	   ll_get_length(list));

    ll_begin_iter(list);
    while((p = ll_get_iter_data(list)) != NULL){
	printf("debug : \t%lu\n", (uintptr_t) p);
    }
    ll_end_iter(list);
}

/* Dump internal node's children. Don't dump records */
static void
bpt_dump_children_keys(char *prefix, bpt_node *curr){
    bpt_node *child;
    void *key;

    if (curr->is_leaf)
	return;

    printf("debug : %s", prefix);

    ll_begin_iter(curr->children);
    while((child = ITER_BPT_CHILD(curr)) != NULL){
	printf("debug : \t\t [ ");
	ll_begin_iter(child->keys);
	while((key = ITER_BPT_KEY(child)) != NULL){
	    printf("%lu, ", (uintptr_t) key);
	}
	ll_end_iter(child->keys);
	printf("]\n");
    }
    ll_end_iter(curr->children);
}

/*
 * Return empty and nullified node.
 *
 * Exported for API tests.
 */
bpt_node *
bpt_gen_node(void){
    bpt_node *node;

    node = (bpt_node *) bpt_malloc(sizeof(bpt_node));
    node->is_root = node->is_leaf = false;
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

/*
 * Create a new bpt_tree * object.
 */
bpt_tree *
bpt_init(bpt_key_access_cb keys_key_access,
	 bpt_key_compare_cb keys_key_compare,
	 bpt_free_cb keys_key_free,
	 uint16_t max_keys){
    bpt_tree *tree;

    if(max_keys < 2){
	fprintf(stderr,
		"the number of max keys should be larger than or equal to two\n");
	return NULL;
    }

    if (keys_key_access == NULL || keys_key_compare == NULL){
	fprintf(stderr,
		"NULL 'keys_key_access' or 'keys_key_compare' callback is invalid\n");
	return NULL;
    }

    tree = (bpt_tree *) bpt_malloc(sizeof(bpt_tree));
    tree->max_keys = max_keys;

    /* Set up the initial empty node with empty lists */
    tree->root = bpt_gen_node();
    tree->root->is_root = tree->root->is_leaf = true;
    tree->root->keys = ll_init(keys_key_access,
			       keys_key_compare,
			       keys_key_free);

    /*
     * Manipulation of children like insert should be
     * designed independently from application-defined
     * keys callbacks.
     *
     * This idea allows insertion of a new record to omit
     * unnecessary duplicate computation or comparison of
     * key value in record.
     *
     * For instance, if after the insertion of a new key,
     * it's necessary to extract key value from the sequence
     * of record data again and compare it with other
     * existing records, then the insertion leads to the key
     * comparisons twice for the new key and for the record.
     * This should be avoided.
     *
     * Therefore, any manipulation of children should depend
     * on some basic APIs without key access and compare
     * callbacks.
     */
    tree->root->children = ll_init(NULL, NULL, NULL);

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
 *
 * When this function is called, the number of keys has been bigger than
 * the tree's 'max_keys'. This function splits the overwhelmed current
 * node and distributes its keys and children. See the top assert().
 */
static bpt_node *
bpt_node_split(bpt_tree *bpt, bpt_node *curr){
    bpt_node *half;
    linked_list *left_keys, *left_children;
    int node_num = GET_MIN_CHILDREN_NUM(bpt->max_keys);

    /* Ensure that keys have overflowed */
    assert(bpt->max_keys + 1 == KEY_LEN(curr));

    /* Create an empty node with null keys and children */
    half = bpt_gen_node();

    /* Move the internal data of node */
    half->keys = ll_split(curr->keys, node_num);

    /*
     * Split children.
     *
     * When this is a leaf node, the number of keys and children must be same.
     * This ensures each record lines up in accordance with corresponding key
     * and allows record to be accessed from the leaf node.
     *
     * On the other hand, when this is an internal node, make the (future) left
     * node, which is 'curr' in this function, have one more child than keys.
     * This ensures the left node can access its correct children.
     */
    half->children = ll_split(curr->children,
			      curr->is_leaf ? node_num : node_num + 1);

    /* Copy other attributes to share */
    half->is_root = curr->is_root;
    half->is_leaf = curr->is_leaf;
    half->parent = curr->parent;

    /*
     * Swap the two nodes to return the right half. At this moment, the 'half'
     * node has left part of keys and children.
     */
    left_keys = half->keys;
    left_children = half->children;

    half->keys = curr->keys;
    half->children = curr->children;

    curr->keys = left_keys;
    curr->children = left_children;

    return half;
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
bpt_insert_internal(bpt_tree *bpt, bpt_node *curr, void *new_key,
		    void *new_value, int new_child_index, void *new_child){
    assert(bpt != NULL);
    assert(curr != NULL);
    assert(new_key != NULL);

    /*
     * --------------------------------------
     * The main part of the insertion process
     * --------------------------------------
     */

    /* Does this node have room to store a new key ? */
    if (KEY_LEN(curr) < bpt->max_keys){
	if (curr->is_leaf){
	    /* Store the pair of key and record */
	    printf("debug : add key = %lu to node (%p)\n",
		   (uintptr_t) new_key, curr);
	    ll_asc_insert(curr->keys, new_key);
	    ll_tail_insert(curr->children, new_value);

	    /* Verify the node property */
	    bpt_node_validity(curr);
	}else{
	    /* Get a copied up key from lower node */
	    printf("debug : insert a copied up key to curr->children[%d]\n",
		   new_child_index);
	    ll_asc_insert(curr->keys, new_key);
	    ll_index_insert(curr->children, new_child, new_child_index);

	    /* Verify the node property */
	    bpt_node_validity(curr);
	}
    }else{
	/*
	 * We have the maximum number of children in this node already. So,
	 * adding a new key-value exeeds the limit. Split the current node,
	 * distribute the keys and children stored there.
	 */
	bpt_node *right_half, *child;
	void *copied_up_key = NULL;
	int key_idx;

	printf("debug : split triggered by %lu\n", (uintptr_t) new_key);

	/*
	 * Add the new key and value (or child). This temporarily
	 * make the number of keys larger than the b+ tree's property.
	 * But bpt_node_split() called below keeps it and balance of
	 * the whole tree.
	 */
	key_idx = ll_asc_insert(curr->keys, new_key);
	if (curr->is_leaf == true)
	    ll_index_insert(curr->children, new_value, key_idx);
	else
	    ll_index_insert(curr->children, new_child, new_child_index);

	/* Split keys and children */
	right_half = bpt_node_split(bpt, curr);

	bpt_dump_list("dump info about the split left node",
		      curr->keys);
	bpt_dump_list("dump info about the split right node",
		      right_half->keys);

	/* Get the key that will go up and/or will be deleted */
	copied_up_key = ll_ref_index_data(right_half->keys, 0);

	/*
	 * Connect split nodes at the same depth. When there is other node
	 * on the right side of 'right_half', make its 'prev' point to the
	 * 'right_half'. Skip if the 'right_half' is the rightmost node.
	 */
	right_half->prev = curr;
	right_half->next = curr->next;
	curr->next = right_half;
	if (right_half->next != NULL)
	    right_half->next->prev = right_half;

	if (!curr->is_leaf){
	    /* Delete the copied up key from the right node */
	    printf("debug : delete the copied up key = %lu from the right node\n",
		   (uintptr_t) copied_up_key);
	    (void) ll_remove_first_data(right_half->keys);

	    /*
	     * After split, all children still point to the left split node.
	     * Then, update the parent member of all of the right half children.
	     */
	    ll_begin_iter(right_half->children);
	    while((child = ITER_BPT_CHILD(right_half)) != NULL)
		child->parent = right_half;
	    ll_end_iter(right_half->children);

	    /* Use the utility for debug */
	    bpt_dump_children_keys("splitted left internal node's children :\n",
				   curr);
	    bpt_dump_children_keys("splitted right internal node's children :\n",
				   right_half);
	}

	if (!curr->parent){
	    /* Create a new root */
	    bpt_node *new_top;

	    assert(curr->is_root == true);
	    assert(right_half->is_root == true);
	    assert(right_half->parent == NULL);

	    new_top = bpt_gen_root_callbacks_node(bpt);
	    new_top->is_root = true;
	    new_top->is_leaf = curr->is_root = right_half->is_root = false;
	    curr->parent = right_half->parent = bpt->root = new_top;

	    ll_asc_insert(new_top->keys, copied_up_key);

	    /* Make the split nodes children for the new root */
	    printf("debug : created a new root with key = %lu\n",
		   (uintptr_t) copied_up_key);
	    ll_tail_insert(new_top->children, curr);
	    ll_tail_insert(new_top->children, right_half);

	    /* Verify the node property */
	    bpt_node_validity(new_top);
	}else{
	    /*
	     * Propagate the key insertion to the upper node. Notify the
	     * upper node of the index to insert a new split right child.
	     * To determine the new child index, we will iterate all children
	     * that the current node's parent stores. The new child should be
	     * placed right after the current node.
	     */
	    linked_list *parent_children = curr->parent->children;
	    int index = 0;

	    ll_begin_iter(parent_children);
	    for (; index < CHILDREN_LEN(curr->parent); index++)
		if (curr == ll_get_iter_data(parent_children))
		    break;
	    ll_end_iter(parent_children);

	    printf("debug : recursive call of bpt_insert() with key = %lu\n",
		   (uintptr_t) copied_up_key);

	    bpt_insert_internal(bpt, curr->parent,
				copied_up_key, NULL, index + 1, right_half);
	}
    }
}

/*
 * Wrapper function of bpt_insert_internal().
 */
bool
bpt_insert(bpt_tree *bpt, void *new_key, void *new_data){
    bpt_node *leaf_node;
    bool found_same_key = false;

    found_same_key = bpt_search(bpt, new_key, &leaf_node, NULL);

    /* Prohibit duplicate keys */
    if (found_same_key)
	return false;
    else{
	bpt_insert_internal(bpt, leaf_node, new_key, new_data, 0, NULL);

	return true;
    }
}

/*
 * Refer to or delete the pair of key and record on the leaf node.
 *
 * The 'exec_delete' flag decides whether the pair is deleted or not.
 */
static void *
bpt_get_key_value_from_leaf(bpt_node *leaf, bool exec_delete, void *search_key){
    void *key, *record;
    int delete_index = 0;

    assert(leaf->is_leaf == true);

    ll_begin_iter(leaf->keys);
    key = ITER_BPT_KEY(leaf);
    while(true){
	if (key == search_key)
	    break;
	/* Move to the next key */
	key = ITER_BPT_KEY(leaf);
	delete_index++;
    }
    ll_end_iter(leaf->keys);

    if (!exec_delete){
	/* The caller requires only record reference */
	record = ll_ref_index_data(leaf->children, delete_index);
    }else{
	/* The caller requires deletion of the key value pair */
	(void) ll_index_remove(leaf->keys, delete_index);
	record = ll_index_remove(leaf->children, delete_index);
    }

    assert(record != NULL);

    return record;
}

/*
 * The main internal processing of B+ tree search.
 */
static bool
bpt_search_internal(bpt_node *curr, void *new_key, bpt_node **leaf_node,
		    void **record){
    linked_list *keys;
    int diff, children_index;

    printf("debug : bpt_search() for key = %lu in node '%p'\n",
	   (uintptr_t) new_key, curr);

    /* Set the last searched node first. This call can be last */
    if (leaf_node != NULL)
	*leaf_node = curr;

    /*
     * Iterate each bpt's key and compare it with the new key. Break
     * if we could search for an exact match or find the first larger
     * value than the new key user indicated.
     *
     * In the latter case, the current index is the one to select the
     * next child to pick up.
     *
     * When we couldn't find any larger values in the keys, then go down
     * to the rightmost child for search.
     */
    keys = curr->keys;

    /*
     * This is an empty node. This code path gets hit when one tries to
     * search tree's root with no data. Need to address since any initial
     * insert depends on the search of the root without data.
     */
    if (KEY_LEN(curr) == 0)
	return false;

    ll_begin_iter(keys);
    for (children_index = 0; children_index < KEY_LEN(curr); children_index++){
	diff = keys->key_compare_cb(keys->key_access_cb(ll_get_iter_data(keys)),
				    keys->key_access_cb(new_key));
	if (diff == 0 || diff == 1)
	    break;
    }
    ll_end_iter(keys);

    if (diff == 0){
	/* Exact key match */
	if (curr->is_leaf){
	    printf("debug : bpt_search_internal() found the same key in leaf node\n");

	    /* When the pointer to the record is required, set it for user */
	    if (record != NULL)
		*record = bpt_get_key_value_from_leaf(curr, false, new_key);

	    return true;
	}else{
	    /* Search for the right child */
	    return bpt_search_internal(bpt_ref_index_child(curr, children_index + 1),
				       new_key, leaf_node, record);
	}
    }else if (diff == 1){
	/*
	 * Found larger key value than the 'new_key'. The next child for search is
	 * the one whose index is children_index. This child's subtree should contain
	 * values smaller than the 'new_key' only. Therefore, this is a valid choice.
	 */
	if (curr->is_leaf)
	    return false;
	else{
	    /* Search for the left child */
	    return bpt_search_internal(bpt_ref_index_child(curr, children_index),
				       new_key, leaf_node, record);
	}
    }else{
	/* The key was bigger than all the existing keys */
	if (curr->is_leaf)
	    return false;
	else{
	    /* Search for the rightmost child */
	    return bpt_search_internal(bpt_ref_index_child(curr,
							   CHILDREN_LEN(curr) - 1),
				       new_key, leaf_node, record);
	}
    }
}

/*
 * Wrapper function of bpt_search_internal().
 *
 * Allow 'leaf_node' and 'record' to be null, when user doesn't need to
 * interact with the leaf node or its value.
 */
bool
bpt_search(bpt_tree *bpt, void* new_key, bpt_node **leaf_node,
	   void **record){
    if (bpt != NULL && bpt->root != NULL && new_key != NULL)
	return bpt_search_internal(bpt->root, new_key, leaf_node, record);

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

/*
 * Return the reference of the leftmost leaf node for debugging.
 */
bpt_node *
bpt_ref_leftmost_leaf_node(bpt_tree *tree){
    bpt_node *node;

    if (tree == NULL || tree->root == NULL)
	return NULL;

    node = tree->root;
    while(!node->is_leaf)
	node = (bpt_node *) node->children->head->data;

    return node;
}

/*
 * Return the right bpt_node * child for the key.
 */
static bpt_node *
bpt_ref_right_child_by_key(bpt_node *node, void *key){
    bpt_node *key_iter, *child_iter;

    ll_begin_iter(node->keys);
    ll_begin_iter(node->children);
    while(true){
	key_iter = ITER_BPT_KEY(node);
	child_iter = ITER_BPT_CHILD(node);

	if (node->keys->key_compare_cb(key_iter, key) == 0)
	    break;
    }
    /* The right child for the key is the next pointer */
    child_iter = ITER_BPT_CHILD(node);
    assert(child_iter != NULL);
    ll_end_iter(node->keys);
    ll_end_iter(node->children);

    return child_iter;
}

/*
 * Don't expect children can perform key_access_cb and key_compare_cb.
 * So, merging children uses some basic APIs for linked list such as
 * ll_tail_insert() or ll_remove_first_data().
 *
 * Update the parent's keys and children according to the merge.
 */
static void *
bpt_merge_nodes(bpt_node *curr, bool with_right){
    linked_list *merged_keys, *merged_children;
    bpt_node *curr_child, *removed_child = NULL;
    node *np;
    void *deleted_key;
    int index = 0;

    if (with_right){
	/* Merge keys */
	merged_keys = ll_merge(curr->keys, curr->next->keys);
	/* Merge children of current node with the ones of the next node */
	while(CHILDREN_LEN(curr->next) > 0){
	    curr_child = (bpt_node *) ll_remove_first_data(curr->next->children);
	    /* If this node is an internal node, update the children's parents */
	    if (!curr->is_leaf)
		curr_child->parent = curr;
	    ll_tail_insert(curr->children, curr_child);
	}
	merged_children = curr->children;
	np = curr->parent->children->head;
    }else{
	/* Merge keys */
	merged_keys = ll_merge(curr->prev->keys, curr->keys);
	/* Merge children of the prev node and ones of curr */
	while(CHILDREN_LEN(curr) > 0){
	    curr_child = (bpt_node *) ll_remove_first_data(curr->children);
	    /* Update children parent member if necessary */
	    if (!curr->is_leaf)
		curr_child->parent = curr->prev;
	    ll_tail_insert(curr->prev->children, curr_child);
	}
	merged_children = curr->prev->children;
	np = curr->prev->parent->children->head;
    }

    /*
     * Now 'np' points to the merged children. Search for the removed child
     * and get the index of the child to pass as an argument of ll_index_remove().
     */
    while(true){
	curr_child = (bpt_node *) np->data;

	if (with_right && curr_child == curr){
	    /* Remove the next node of the current node */
	    printf("debug : found the current node to merge with right child\n");
	    removed_child = curr_child->next;
	    if (curr->next->next)
		curr->next->next->prev = curr;
	    curr->next = curr->next->next;
	    break;
	}else if (!with_right && curr_child == curr->prev){
	    /* Remove the current node */
	    printf("debug : found the previous node to merge with the current node\n");
	    removed_child = curr;
	    if (curr->next)
		curr->next->prev = curr->prev;
	    curr->prev->next = curr->next;
	    break;
	}

	/* Iterate all children */
	if ((np = np->next) == NULL)
	    break;
	index++;
    }

    /* Remove a parent's key which has become unnecessary by merge */
    assert((deleted_key = ll_index_remove(curr->parent->keys, index)) != NULL);
    /* Detach the removed child from the parent children as well. */
    assert(ll_index_remove(curr->parent->children, index + 1) != NULL);

    printf("debug : this merge removed key = %lu at index = %d in %s parent node\n",
	   (uintptr_t) deleted_key, index, curr->parent->is_root ? "root" : "non-root");

    /* Reconnect all the merged results */
    if (with_right){
	curr->keys = merged_keys;
	curr->children = merged_children;
    }else{
	curr->prev->keys = merged_keys;
	curr->prev->children = merged_children;
    }

    /*
     * Debug.
     *
     * (1) The removed child must have no left keys and children.
     * (2) The parent must delete the pointer to this removed child.
     */
    assert(KEY_LEN(removed_child) == 0);
    assert(CHILDREN_LEN(removed_child) == 0);
    ll_begin_iter(curr->parent->children);
    while((curr_child = ITER_BPT_CHILD(curr->parent)) != NULL){
	if (curr_child == removed_child)
	    assert(0);
    }
    ll_end_iter(curr->parent->children);

    /* Debugging on the removed child is done. Free the child */
    bpt_free_node(removed_child);

    return deleted_key;
}

static void
bpt_replace_index(bpt_node *curr, bool from_right){
    void *prev_index, *replaced_index;
    void *child;
    void *key;
    bpt_node *right_child;

    /*
     * Iterate key and child simultaneously. When we find the pointer
     * of 'curr', we can tell which key to remove from the parent's
     * node.
     */
    ll_begin_iter(curr->parent->keys);
    ll_begin_iter(curr->parent->children);
    prev_index = replaced_index = ITER_BPT_KEY(curr->parent);
    child = ITER_BPT_CHILD(curr->parent);
    while(true){
	if (from_right && child == curr){
	    /*
	     * If we borrow from the right child, then the
	     * current 'replaced_index' should be removed.
	     */
	    right_child = ITER_BPT_CHILD(curr->parent);
	    break;
	}else if (!from_right && child == curr){
	    /*
	     * If we borrow from the left child, then the
	     * previous key, 'prev_index' should be removed.
	     */
	    replaced_index = prev_index;
	    right_child = child;
	    break;
	}
	prev_index = replaced_index;
	replaced_index = ITER_BPT_KEY(curr->parent);
	child = ITER_BPT_CHILD(curr->parent);
    }
    ll_end_iter(curr->parent->keys);
    ll_end_iter(curr->parent->children);

    assert(ll_remove_by_key(curr->parent->keys, replaced_index) != NULL);
    assert((key = bpt_ref_subtree_minimum_key(right_child)) != NULL);
    ll_asc_insert(curr->parent->keys, key);

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
    key = ITER_BPT_KEY(left->parent);
    child = ITER_BPT_CHILD(left->parent);
    while(true){
	if (child == left)
	    break;
	/* Iterate the next pair of key and child */
	key = ITER_BPT_KEY(left->parent);
	child = ITER_BPT_CHILD(left->parent);
    }
    assert(right == ll_get_iter_data(left->parent->children));
    ll_end_iter(left->parent->keys);
    ll_end_iter(left->parent->children);

    return key;
}


/*
 * Return true if either type of root promotion occurred.
 *
 * Otherwise, return false.
 */
static bool
bpt_root_promoted(bpt_tree *bpt, bpt_node *curr){

    /*
     * First condition for root promotion.
     *
     * If this is a leaf node, skip this condition block. Otherwise,
     * deleting one key (and its record) from one root node without
     * any bpt_node * children hits this path wrongly.
     */
    if (curr->is_root && !curr->is_leaf && CHILDREN_LEN(curr) == 1){
	bpt_node *child;

	assert(curr->prev == NULL);
	assert(curr->next == NULL);

	child = ll_remove_first_data(curr->children);

	/* Reconnect nodes */
	child->is_root = true;
	child->parent = NULL;
	bpt->root = child;

	/* Free the unnecessary node */
	bpt_free_node(curr);

	printf("debug : completed root promotion\n");

	return true;
    }

    /*
     * The second condition.
     *
     * This shrinks tree's height by merge. The current node has no key and
     * it is the last child for the parent.
     */
    if (curr->parent && curr->parent->is_root && KEY_LEN(curr->parent) == 1 &&
	KEY_LEN(curr) == 0 && CHILDREN_LEN(curr) == 1){
	/*
	 * The previous call of bpt_delete_internal() has merged the last two
	 * children of this current node and deleted one last index within this
	 * node. Besides, the current node's parent is root and it has one left
	 * key only.
	 */
	if (HAVE_SAME_PARENT(curr->prev, curr)){
	    bpt_node *child;
	    void *min_key;

	    /*
	     * Move the current node's children to the previous node.
	     *
	     * Meanwhile, it's possible that the parent's key can't be
	     * used as is, for a previous node key.
	     *
	     * Suppose the previous call of bpt_delete_internal()
	     * deleted the right child key of this node. This means
	     * the parent's key won't exist any more on the right
	     * subtree and writing the parent's key in the previous
	     * node causes a wrong key insertion.
	     *
	     * In this case, obtain the minimum value from the left child
	     * again and utilize it as the key value added to the previous
	     * node. This ensures indexes are stored correctly.
	     */
	    child = (bpt_node *) ll_remove_first_data(curr->children);
	    min_key = bpt_ref_subtree_minimum_key(child);
	    ll_asc_insert(curr->prev->keys, min_key);
	    child->parent = curr->prev;
	    ll_tail_insert(curr->prev->children, child);

	    /*
	     * If there are some children below the current node,
	     * make them point to the previous node as parent.
	     */
	    while(CHILDREN_LEN(curr) > 0){
		child = (bpt_node *) ll_remove_first_data(curr->children);
		if (!curr->is_leaf)
		    child->parent = curr->prev;
		ll_tail_insert(curr->prev->children, child);
	    }

	    printf("debug : the tree height has shrunk, with key migration = %lu\n",
		   (uintptr_t) min_key);

	    /*
	     * Reconnect nodes.
	     *
	     * The previous node will become the new root.
	     */
	    bpt->root = curr->prev;
	    curr->prev->is_root = true;
	    curr->prev->next = NULL;
	    curr->prev->parent = NULL;

	    /* Free the current root and the current node */
	    bpt_free_node(curr->parent);
	    bpt_free_node(curr);

	    /* Done with the key deletion */
	    return true;
	}

	if (HAVE_SAME_PARENT(curr, curr->next)){
	    void *key;
	    bpt_node *child;

	    /*
	     * In this scenario of tree height shrink, we can just move the
	     * last parent's key and the current child node to the next node.
	     * This is because we ensure that the parent key is bigger than
	     * any other keys in this node or any other child nodes of this
	     * node. Utilizing the parent's key can keep the entire order of
	     * indexes and children.
	     */
	    key = ll_remove_first_data(curr->parent->keys);
	    ll_asc_insert(curr->next->keys, key);

	    /*
	     * If there are some children below the current node, make them
	     * point to the next node as parent.
	     */
	    while(CHILDREN_LEN(curr) > 0){
		child = (bpt_node *) ll_remove_first_data(curr->children);
		if (!curr->is_leaf)
		    child->parent = curr->next;
		ll_insert(curr->next->children, child);
	    }

	    printf("debug : the tree height has shrunk, with key migration = %lu\n",
		   (uintptr_t) key);

	    /*
	     * Reconnect nodes.
	     *
	     * The next node will become the new root
	     */
	    bpt->root = curr->next;
	    curr->next->is_root = true;
	    curr->next->prev = NULL;
	    curr->next->parent = NULL;

	    /* Free the current root and the current node */
	    bpt_free_node(curr->parent);
	    bpt_free_node(curr);

	    /* Done with the key deletion */
	    return true;
	}
    }

    return false;
}

/*
 * The main internal processing of B+ tree deletion.
 */
static void
bpt_delete_internal(bpt_tree *bpt, bpt_node *curr, void *removed_key,
		    void **record){
    int min_key_num = GET_MIN_KEY_NUM(bpt->max_keys);

    assert(bpt != NULL);
    assert(curr != NULL);
    assert(removed_key != NULL);

    printf("debug : bpt_delete_internal() for %p\n"
           "debug : current node = %s and %s, the number of keys = %d, the number of children = %d\n",
	   curr,
	   curr->is_root ? "root" : "non-root",
	   curr->is_leaf ? "leaf" : "non-leaf",
	   KEY_LEN(curr), CHILDREN_LEN(curr));

    /*
     * If the root promotion happens, then we have reached to
     * the top of the tree and done with all the necessary
     * steps to keep the B+ tree properties.
     *
     * Return and close this key deletion process.
     */
    if (bpt_root_promoted(bpt, curr))
	return;

    /*
     * --------------------------------------
     * The main part of deletion process
     * --------------------------------------
     */

    if (curr->is_leaf){
	/*
	 * The call of bpt_search() before the first call of
	 * bpt_delete_internal() already proves that the key exists.
	 * So, this leaf node must contain the removed key.
	 */
	if (record == NULL){
	    /*
	     * Discard the corresponding record when user indicates
	     * the record is not necessary.
	     */
	    (void) bpt_get_key_value_from_leaf(curr, true,
					       removed_key);
	}else{
	    *record = bpt_get_key_value_from_leaf(curr, true,
						  removed_key);
	}

	printf("debug : removed key = '%lu' on the leaf node\n",
	       (uintptr_t) removed_key);

	/* Verify the node property */
	bpt_node_validity(curr);
    }else{
	/*
	 * This internal node might or might not have the index.
	 * If the indexes contain the key, then remove and replace
	 * it with its right child's minimum key.
	 */
	if (ll_has_key(curr->keys, removed_key)){
	    void *key;
	    bpt_node *right_child;

	    right_child = bpt_ref_right_child_by_key(curr, removed_key);
	    ll_remove_by_key(curr->keys, removed_key);
	    assert((key = bpt_ref_subtree_minimum_key(right_child)) != NULL);
	    printf("debug : removed %lu and inserted %lu as the min key\n",
		   (uintptr_t) removed_key, (uintptr_t) key);
	    ll_asc_insert(curr->keys, key);

	    /* Verify the node property */
	    bpt_node_validity(curr);
	}
    }

    /*
     * We are done with key deletion of this node. Execute the following codes
     * and keep the b+ tree property.
     */

    if (KEY_LEN(curr) >= min_key_num){
	/* The current node has sufficient keys. Go up if possible */
	if (!curr->is_root)
	    bpt_delete_internal(bpt, curr->parent, removed_key, record);

	return;
    }else{
	/* The current node doesn't have enough keys. Try borrowing first */
	bool has_left_sibling = false, borrowed_from_left = false,
	    has_right_sibling = false, borrowed_from_right = false;
	void *borrowed_key;

	/* Is the left node an available sibling to borrow a key ? */
	if (HAVE_SAME_PARENT(curr, curr->prev)){
	    assert(curr->parent != NULL);

	    has_left_sibling = true;

	    if (KEY_LEN(curr->prev) > min_key_num){
		borrowed_from_left = true;

		printf("debub : borrowing from the left node\n"
		       "debug : the number of current node's keys = %d\n",
		       KEY_LEN(curr));

		if (curr->is_leaf){
		    /* Borrowing between the leaf nodes */
		    borrowed_key = ll_tail_remove(curr->prev->keys);
		    ll_insert(curr->keys, borrowed_key);
		    ll_insert(curr->children, ll_tail_remove(curr->prev->children));

		    printf("debug : borrowed the max key = %lu from the left sibling\n",
			   (uintptr_t) borrowed_key);

		    /* Update the parent's index */
		    bpt_replace_index(curr, false);

		    /* Verify the node property */
		    bpt_node_validity(curr);

		    /* Continue to update the indexes. Go up by recursive call */
		    if (!curr->is_root)
			bpt_delete_internal(bpt, curr->parent, removed_key, record);
		    return;
		}else{
		    /* Borrowing between the internal nodes */
		    void *middle_key, *largest_key;
		    bpt_node *borrowed_child;

		    /*
		     * If this internal node borrows a key from other node, then get
		     * the intermediate parent's key as the new key value for this node,
		     * and remove it from the parent and write the largest key as the
		     * new parent's key.
		     */
		    assert((largest_key = ll_tail_remove(curr->prev->keys)) != NULL);
		    middle_key = bpt_ref_key_between_children(curr->prev, curr);

		    ll_asc_insert(curr->keys, middle_key);
		    assert(ll_remove_by_key(curr->parent->keys, middle_key) != NULL);
		    ll_asc_insert(curr->parent->keys, largest_key);

		    bpt_dump_list("from left internal node's children",
				  curr->prev->keys);
		    bpt_dump_list("to right internal node's children",
				  curr->keys);

		    /* Whenever we borrow an internal node's child, update its attributes */
		    assert((borrowed_child = (bpt_node *) ll_tail_remove(curr->prev->children)) != NULL);
		    borrowed_child->parent = curr;
		    ll_insert(curr->children, (void *) borrowed_child);

		    printf("debug : the new current node's key = %lu, new parent's index key = %lu\n",
			   (uintptr_t) middle_key, (uintptr_t) largest_key);

		    /* Verify the node property */
		    bpt_node_validity(curr);
		}
	    }
	}

	/* The left node wasn't available. How about the right sibling ? */
	if (!borrowed_from_left && HAVE_SAME_PARENT(curr, curr->next)){
	    assert(curr->parent != NULL);
	    has_right_sibling = true;

	    if (KEY_LEN(curr->next) > min_key_num){
		borrowed_from_right = true;

		printf("debug : borrowing from the right node\n"
		       "debug : the number of current node's keys = %d\n",
		       KEY_LEN(curr));

		if (curr->is_leaf){
		    /* Borrowing between the leaf nodes */
		    borrowed_key = ll_remove_first_data(curr->next->keys);
		    ll_tail_insert(curr->keys, borrowed_key);
		    ll_tail_insert(curr->children,
				   ll_remove_first_data(curr->next->children));

		    printf("debug : borrowed the min key = %lu from the right sibling\n",
			   (uintptr_t) borrowed_key);

		    /* Update the parent's index */
		    bpt_replace_index(curr, true);

		    /* Verify the node property */
		    bpt_node_validity(curr);

		    /* Continue to update the indexes. Go up by recursive call */
		    if (!curr->is_root)
			bpt_delete_internal(bpt, curr->parent, removed_key, record);

		    return;
		}else{
		    /* Borrowing between the internal nodes */
		    void *middle_key, *smallest_key;
		    bpt_node *borrowed_child;

		    /* Remove the smallest key from the right node by borrowing */
		    assert((smallest_key = ll_remove_first_data(curr->next->keys)) != NULL);
		    middle_key = bpt_ref_key_between_children(curr, curr->next);

		    ll_asc_insert(curr->keys, middle_key);
		    assert(ll_remove_by_key(curr->parent->keys, middle_key) != NULL);
		    ll_asc_insert(curr->parent->keys, smallest_key);

		    bpt_dump_list("from right internal node's children",
				  curr->next->keys);
		    bpt_dump_list("to left internal node's children",
				  curr->keys);

		    /* Whenever we borrow an internal node's child, update its attributes */
		    borrowed_child = (bpt_node *) ll_remove_first_data(curr->next->children);
		    assert(borrowed_child != NULL);
		    borrowed_child->parent = curr;
		    ll_tail_insert(curr->children, borrowed_child);

		    printf("debug : the new current node's key = %lu, the new parent's index key = %lu\n",
			   (uintptr_t) middle_key, (uintptr_t) smallest_key);

		    /* Verify the node property */
		    bpt_node_validity(curr);
		}
	    }
	}

	/* Merge nodes if possible */
	if (borrowed_from_left == false && borrowed_from_right == false){

	    printf("debug : borrowing a key didn't happen\n");

	    if (has_left_sibling){
		/*
		 * Note that after the merge with the left node,
		 * the current (right) node will be free-ed. Then,
		 * refer to any attributes of 'curr' won't be
		 * allowed after the bpt_merge_nodes(). Save
		 * the parameter necessary for any subsequent
		 * processing in advance.
		 */
		void *deleted_key;
		bpt_node *prev = curr->prev;

		printf("debug : bpt_merge_nodes() with left node\n");

		/* The current node gets merged with the previous one */
		deleted_key = bpt_merge_nodes(curr, false);
		printf("debug : this merge decrements the number of parent's keys to '%d'\n",
		       KEY_LEN(prev->parent));

		/*
		 * Incorporate the split key from parent to the current node,
		 * if this is an internal node.
		 */
		if (prev->is_leaf == false){
		    ll_asc_insert(prev->keys, deleted_key);
		    printf("debug : incorporate the split key = %lu from parent to child\n",
			   (uintptr_t) deleted_key);
		}

		/* Verify the node property */
		bpt_node_validity(prev);

		/*
		 * Go up by using the saved reference of previous sibling.
		 * Avoid the call of the bpt_delete_internal() that uses
		 * current node below.
		 */
		if (prev->parent)
		    bpt_delete_internal(bpt, prev->parent, removed_key, record);

		return;

	    }else if (has_right_sibling){
		/*
		 * This path merges the current node with the next node.
		 * Any left data in the next node will be moved to the current
		 * node. Thus, after the bpt_merge_nodes(), attributes of the
		 * current node are available.
		 */
		void *deleted_key;

		printf("debug : bpt_merge_nodes() with right node\n");

		deleted_key = bpt_merge_nodes(curr, true);
		printf("debug : this merge decrements the number of parent's keys to '%d'\n",
		       KEY_LEN(curr->parent));

		/*
		 * Incorporate the split key from parent to the current node,
		 * if this is an internal node.
		 */
		if (curr->is_leaf == false){
		    ll_asc_insert(curr->keys, deleted_key);
		    printf("debug : incorporate the split key = %lu from parent to child\n",
			   (uintptr_t) deleted_key);
		}

		/* Verify the node property */
		bpt_node_validity(curr);
	    }else{
		printf("debug : merging nodes didn't happen either\n");
	    }
	}

	/* Continue to update the indexes. Go up by recursive call */
	if (!curr->is_root)
	    bpt_delete_internal(bpt, curr->parent, removed_key, record);
    }
}

bool
bpt_delete(bpt_tree *bpt, void *key, void **record){
    bpt_node *leaf_node;
    bool found_same_key = false;

    printf("debug : bpt_delete() for root = %p with key = %p\n", bpt->root, key);

    found_same_key = bpt_search(bpt, key, &leaf_node, NULL);

    /* Remove the found key */
    if (found_same_key){
	printf("debug : call bpt_delete_internal() with leaf node = %p\n", leaf_node);

	bpt_delete_internal(bpt, leaf_node, key, record);

	return true;
    }else{
	printf("debug : the key to be removed was not found\n");

	return false;
    }
}

/* Free the entire tree from the root to the bottom */
void
bpt_destroy(bpt_tree *bpt){
    bpt_node *leftmost, *prev, *curr;

    if (bpt == NULL)
	return;

    if (bpt->root == NULL){
	free(bpt);
	return;
    }

    prev = curr = bpt->root;

    while(curr != NULL){
	/* Remember the leftmost node */
	leftmost = curr->is_leaf ? NULL : bpt_ref_index_child(curr, 0);

	/* Free nodes vertically */
	while(true){
	    prev = curr;
	    curr = curr->next;
	    bpt_free_node(prev);
	    if (curr == NULL)
		break;
	}

	/* Iterate the children */
	curr = leftmost;
    }

    free(bpt);
}
