#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "b_plus_tree.h"

#define HAVE_SAME_PARENT(n1, n2) \
    (n1 != NULL && n2 != NULL && n1->parent == n2->parent)

/* static void bpt_borrow_key_from_sibling(){} */

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
    while((p = ll_get_iter_data(list)) != NULL){
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
	 bpt_free_cb children_key_free, uint16_t m){
    bpt_tree *tree;

    if(m < 3){
	fprintf(stderr,
		"b+ tree's order needs to be larger than three\n");
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
 * ll_split() returns the *left* half, so swap the
 * return value within this function. This is required
 * to connect the split nodes with other existing nodes
 * before and after the two split nodes.
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

	printf("*split for %lu*\n", (uintptr_t) new_key);

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

	/*
	 * Connect split nodes at the same depth.
	 *
	 * When there is other node on the right side of 'right_half',
	 * make its 'prev' point to the 'right_half'. Skip if the
	 * 'right_half' is the rightmost node.
	 */
	right_half->prev = curr_node;
	right_half->next = curr_node->next;
	curr_node->next = right_half;
	if (right_half->next != NULL)
	    right_half->next->prev = right_half;

	if (!curr_node->is_leaf){
	    void *p;

	    /* Delete the copied up key and the corresponding child */
	    (void) ll_remove_first_data(right_half->keys);
	    assert((p = ll_remove_first_data(right_half->children)) == NULL);
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
	    linked_list *parent_children = curr_node->parent->children;
	    int index = 0;

	    ll_begin_iter(parent_children);
	    for (; index < ll_get_length(parent_children); index++)
		if (curr_node == ll_get_iter_data(parent_children))
		    break;
	    ll_end_iter(parent_children);

	    /* We must find the target child */
	    assert(index < ll_get_length(parent_children));

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

    /* Prohibit duplicate keys */
    if (found_same_key)
	return false;
    else{
	bpt_insert_internal(t, last_node, new_key, new_data, 0, NULL);

	return true;
    }
}

static bpt_node *
bpt_fetch_index_child(bpt_node *curr_node, int index){
    return (bpt_node *) ll_ref_index_data(curr_node->children, index);
}

bool
bpt_search(bpt_node *curr_node, void *new_key, bpt_node **last_node){
    linked_list *keys;
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
    *last_node = curr_node;

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
	/* We might hit NULL during key iteration. Skip NULL. */
	if ((existing_key = ll_get_iter_data(keys)) == NULL)
	    continue;

	diff = keys->key_compare_cb(keys->key_access_cb(existing_key),
				    keys->key_access_cb(new_key));
	if (diff == 0 || diff == 1)
	    break;

	children_index++;
    }
    ll_end_iter(keys);

    if (diff == 0){
	/* Exact key match */
	if (curr_node->is_leaf)
	    return true;
	else{
	    /* Search for the right child */
	    return bpt_search(bpt_fetch_index_child(curr_node, children_index + 1),
			      new_key, last_node);
	}
    }else if (diff == 1){
	/* Found larger key value than the 'new_key' */
	if (curr_node->is_leaf)
	    return false;
	else{
	    /* Search for the left child */
	    return bpt_search(bpt_fetch_index_child(curr_node, children_index),
			      new_key, last_node);
	}
    }else{
	/* The key was bigger than all the existing keys */
	if (curr_node->is_leaf)
	    return false;
	else{
	    /* Search for the rightmost child */
	    return bpt_search(bpt_fetch_index_child(curr_node,
						    ll_get_length(curr_node->children) - 1),
			      new_key, last_node);
	}
    }
}

/*
 * Return the minimum key from one subtree.
 *
 * Go down the subtree until we reach the leftmost
 * child of the leaf node.
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

	if (key_iter == key){
	    /* The right child for the key is the next pointer */
	    right_child = ll_get_iter_data(node->children);
	    assert(right_child != NULL);

	    return (bpt_node *) right_child;
	}
    }
    ll_end_iter(node->keys);
    ll_end_iter(node->children);

    assert(0); /* Must be unreachable */

    return NULL; /* Make compiler silent */
}

static void
bpt_delete_internal(bpt_tree *t, bpt_node *curr_node, void *removed_key){
    linked_list *keys = curr_node->keys;
    int min_key;

    min_key = (t->m % 2 == 0) ? (t->m % 2 - 1) : (t->m % 2);

    if (ll_get_length(keys) - 1 >= min_key){
	printf("debug : Delete key = %lu\n", (uintptr_t) removed_key);

	if (curr_node->is_leaf){
	    ll_remove_by_key(keys, removed_key);
	    /* TODO : Remove the record as well */
	}else{
	    void *deleted_key;

	    /* Does the key exist in this node ? */
	    deleted_key = ll_remove_by_key(keys, removed_key);

	    if (deleted_key != NULL){
		void *min_key;
		bpt_node *right_child;

		right_child = bpt_ref_right_child_by_key(curr_node, removed_key);
		assert((min_key = bpt_ref_subtree_minimum_key(right_child)) != NULL);
		printf("debug : %lu was removed and %lu was inserted as the min key\n",
		       (uintptr_t) removed_key, (uintptr_t) min_key);
		ll_asc_insert(keys, min_key);
	    }

	    /*
	     * This internal node might or might not have the key.
	     * Just Continue to update the indexes. Go up.
	     */
	    /* bpt_delete_internal(); */
	}
    }else{
	linked_list *ref_keys, *ref_children;
	bool left_sibling_exist = false, borrowed_from_left = false,
	    right_sibling_exist = false, borrowed_from_right = false;

	if (curr_node->is_leaf){

	    if (HAVE_SAME_PARENT(curr_node, curr_node->prev)){
		ref_keys = curr_node->prev->keys;
		ref_children = curr_node->prev->children;
		left_sibling_exist = true;

		if (ll_get_length(ref_keys) - 1 >= min_key){
		    void *largest_key;

		    /* Remove the key of current node */
		    ll_remove_by_key(curr_node->keys, removed_key);

		    /* Borrow one key and child from left sibling */
		    largest_key = ll_tail_remove(ref_keys);
		    ll_insert(curr_node->keys, largest_key);
		    ll_insert(curr_node->children,
			      ll_tail_remove(ref_children));

		    borrowed_from_left = true;
		    printf("debug : borrowed the largest key = %lu from the left sibling\n",
			   (uintptr_t) largest_key);

		    if (curr_node->parent != NULL){
			/* Replace parent's index */
			ll_remove_by_key(curr_node->parent->keys,
					 removed_key);
			ll_asc_insert(curr_node->parent->keys,
				      largest_key);
			printf("debug : index replaced from %lu to %lu\n",
			       (uintptr_t) removed_key, (uintptr_t) largest_key);
		    }
		}

		if (borrowed_from_left == false && HAVE_SAME_PARENT(curr_node, curr_node->next)){
		    ref_keys = curr_node->next->keys;
		    ref_children = curr_node->next->children;
		    right_sibling_exist = true;

		    if (ll_get_length(ref_keys) - 1 >= min_key){
			void *smallest_key;

			/* Remove the key of current node */
			ll_remove_by_key(curr_node->keys, removed_key);
			printf("debug : Delete key = %lu\n", (uintptr_t) removed_key);

			/* Borrow one key and child from right sibling */
			smallest_key = ll_remove_first_data(ref_keys);
			ll_tail_insert(curr_node->keys, smallest_key);
			ll_tail_insert(curr_node->children,
				       ll_remove_first_data(ref_children));

			borrowed_from_right = true;
			printf("debug : Borrowed the smallest key = %lu from the right sibling\n",
			       (uintptr_t) smallest_key);

			if (curr_node->parent != NULL){
			    ll_remove_by_key(curr_node->parent->keys,
					     removed_key);
			    ll_asc_insert(curr_node->parent->keys,
					  smallest_key);
			    printf("debug : index replaced from %lu to %lu\n",
				   (uintptr_t) removed_key, (uintptr_t) smallest_key);
			}
		    }
		}
	    }

	    /*
	     * Firstly, the number of current node key is smaller than the
	     * minimum key. Also, failed to find available key from both
	     * nodes. Merge the current node's keys and next node ones.
	     */
	    if (borrowed_from_left == false || borrowed_from_right == false){
		printf("both sides of nodes aren't available\n");

		/* Merge nodes */
	    }
	}else{
	    printf("remove a key from internal node\n");
	}
    }
}

bool
bpt_delete(bpt_tree *bpt, void *key){
    bpt_node *last_node;
    bool found_same_key = false;

    found_same_key = bpt_search(bpt->root, key, &last_node);

    /* Remove the found key */
    if (found_same_key){
	bpt_delete_internal(bpt, last_node, key);

	return true;
    }else
	return false;
}

void
bpt_destroy(bpt_tree *bpt){}
