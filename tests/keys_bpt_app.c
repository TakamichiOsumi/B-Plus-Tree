#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../b_plus_tree.h"

/*
 * This file tests combination of integer keys casted to uintptr_t
 * and values which points to employee object.
 */
typedef struct employee {
    uintptr_t id;
    char *name;
} employee;

/*
 * Compare two (originally) integer values. Ignore metadata.
 */
static int
employee_key_compare(void *key1, void *key2, void *metadata){
    uintptr_t k1 = (uintptr_t) key1,
        k2 = (uintptr_t) key2;

    if (k1 < k2){
        return -1;
    }else if (k1 == k2){
        return 0;
    }else{
        return 1;
    }
}

/*
 * Don't free keys and children. The object is not allocated
 * in the tests.
 */
static void employee_key_free(void *data){}

static void employee_record_free(void *data){}

/*
 * Execute bpt_search based on uintptr_t arrays, expecting all
 * search should succeed.
 */
static void
app_loop_bpt_search(bpt_tree *bpt, int count, uintptr_t answers[]){
    int i;
    bpt_node *node;

    for (i = 0; i < count; i++){
	node = NULL;
	assert(bpt_search(bpt, (void *) answers[i], &node, NULL) == true);
	assert(node != NULL);
    }
}

/* Check only one node */
static void
one_node_keys_comparison_test(bpt_node *node, uintptr_t answers[]){
    int i = 0;
    void *p;

    bpt_node_validity(node);

    ll_begin_iter(node->keys);
    while ((p = ll_get_iter_data(node->keys)) != NULL){
	if ((uintptr_t) p != answers[i]){
	    printf("debug : the expectation is not same as leaf value. %lu vs. %lu\n",
		   (uintptr_t) p, answers[i]);
	    assert(0);
	}else{
	    printf("debug : found %lu expectedly\n", answers[i]);
	}
	i++;
    }
    ll_end_iter(node->keys);
}

/* Check the full nodes at the same level from left to right */
static void
full_keys_comparison_test(bpt_node *node, uintptr_t answers[]){
    int i = 0;
    uintptr_t *p;

    assert(node != NULL);

    while(true){
	bpt_node_validity(node);

	/* Check each key at the same level of node */
	ll_begin_iter(node->keys);
	while((p = (void *) ll_get_iter_data(node->keys)) != NULL){
	    if ((uintptr_t) p != answers[i]){
		printf("debug : the expected value is not same as leaf node value (%lu vs. %lu)\n",
		       (uintptr_t) p, answers[i]);
		assert(0);
	    }else{
		printf("debug : found %lu expectedly\n", answers[i]);
	    }
	    i++;
	}
	ll_end_iter(node->keys);

	/* Move to the next leaf node */
	if ((node = node->next) == NULL)
	    break;
    }
}

/* Check all the keys at the same level from right to left */
static void
reverse_full_keys_comparison_test(bpt_node *node, uintptr_t answers[]){
    int i = 0, j = 0;
    uintptr_t *p;

    assert(node != NULL);

    while(true){
	for (i = ll_get_length(node->keys) - 1; i >= 0; i--){
	    p = ll_ref_index_data(node->keys, i);
	    if ((uintptr_t) p != answers[j]){
		printf("debug : the expectation is different from the order of leaves (%lu vs. %lu)\n",
		       (uintptr_t) p, (uintptr_t) answers[j]);
		assert(0);
	    }else{
		printf("debug : iterating leaf nodes hits %lu\n", (uintptr_t) p);
	    }
	    j++;
	}
	if ((node = node->prev) == NULL)
	    break;
    }
}

/*
 * All tests in this file just focus on the key and index management
 * and doesn't take care of record management. Therefore, just have
 * one data for the whole tests and let other tests handle the record
 * management.
 *
 * See other sets of tests in records_bpt_app.c.
 */
static employee emp = { 1, "dummy" };

static void
search_single_node_test(void){
    bpt_tree *tree;
    bpt_node *node;

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    4, NULL);

    /* Construct one root without any leaf nodes */
    ll_asc_insert(tree->root->keys, (void *) 4);
    ll_tail_insert(tree->root->children, (void *) &emp);
    ll_asc_insert(tree->root->keys, (void *) 1);
    ll_tail_insert(tree->root->children, (void *) &emp);
    ll_asc_insert(tree->root->keys, (void *) 2);
    ll_tail_insert(tree->root->children, (void *) &emp);

    /* Exact key match */
    assert(bpt_search(tree, (void *) 1, NULL, NULL) == true);
    assert(bpt_search(tree, (void *) 2, NULL, NULL) == true);
    assert(bpt_search(tree, (void *) 4, NULL, NULL) == true);

    /* Non-existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 50, &node, NULL) == false);
    assert(node == tree->root);
    node = NULL;
    assert(bpt_search(tree, (void *) 5, &node, NULL) == false);
    assert(node == tree->root);

    /* Clean up */
    bpt_destroy(tree);
}

static void
search_two_depth_nodes_test(void){
    bpt_tree *tree;
    bpt_node *left, *right, *node;

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    3, NULL);

    /* Root node */
    ll_asc_insert(tree->root->keys, (void *) 5);
    tree->root->is_leaf = false;

    /* Left leaf node */
    left = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(left->keys, (void *) 2);
    ll_asc_insert(left->keys, (void *) 3);
    ll_asc_insert(left->keys, (void *) 4);
    ll_tail_insert(tree->root->children, (void *) left);
    left->is_root = false;
    left->is_leaf = true;
    left->parent = tree->root;

    /* Right leaf node */
    right = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(right->keys, (void *) 5);
    ll_asc_insert(right->keys, (void *) 6);

    ll_tail_insert(tree->root->children, (void *) right);
    right->is_root = false;
    right->is_leaf = true;
    right->parent = tree->root;

    /* Connect leaves */
    left->next = right;

    /* Now, do the search of existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 2, &node, NULL) == true);
    assert(node == left);

    node = NULL;
    assert(bpt_search(tree, (void *) 5, &node, NULL) == true);
    assert(node == right);

    node = NULL;
    assert(bpt_search(tree, (void *) 6, &node, NULL) == true);
    assert(node == right);

    /* Search non-existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node, NULL) == false);
    assert(node == left);

    node = NULL;
    assert(bpt_search(tree, (void *) 7, &node, NULL) == false);
    assert(node == right);

    /* Clean up */
    bpt_destroy(tree);
}

static void
search_three_depth_nodes_test(void){
    bpt_tree *tree;
    /* Internal nodes */
    bpt_node *left_internal, *right_internal;
    /* Leaf nodes */
    bpt_node *leftmost, *second_from_left, *middle,
	*second_from_right, *rightmost;
    /* For test */
    bpt_node *node;

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    5, NULL);

    /* Root node */
    ll_asc_insert(tree->root->keys, (void *) 13);
    tree->root->is_leaf = false;

    /* Left internal node */
    left_internal = bpt_gen_root_callbacks_node(tree);
    ll_tail_insert(left_internal->keys, (void *) 9);
    ll_tail_insert(left_internal->keys, (void *) 11);
    left_internal->is_root = false;
    left_internal->is_leaf = false;
    left_internal->parent = tree->root;

    /* Right internal node */
    right_internal = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(right_internal->keys, (void *) 16);
    right_internal->is_root = false;
    right_internal->is_leaf = false;
    right_internal->parent = tree->root;

    left_internal->next = right_internal;

    /* Leftmost leaf node */
    leftmost = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(leftmost->keys, (void *) 1);
    ll_asc_insert(leftmost->keys, (void *) 4);
    leftmost->is_root = false;
    leftmost->is_leaf = true;
    leftmost->parent = left_internal;

    /* Second node from the left */
    second_from_left = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(second_from_left->keys, (void *) 9);
    ll_asc_insert(second_from_left->keys, (void *) 10);
    second_from_left->is_root = false;
    second_from_left->is_leaf = true;
    second_from_left->parent = left_internal;

    /* Middle leaf node */
    middle = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(middle->keys, (void *) 11);
    ll_asc_insert(middle->keys, (void *) 12);
    middle->is_root = false;
    middle->is_leaf = true;
    middle->parent = left_internal;

    /* Second node from the right */
    second_from_right = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(second_from_right->keys, (void *) 13);
    ll_asc_insert(second_from_right->keys, (void *) 15);
    second_from_right->is_root = false;
    second_from_right->is_leaf = true;
    second_from_right->parent = right_internal;

    /* Rightmost leaf node */
    rightmost = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(rightmost->keys, (void *) 16);
    ll_asc_insert(rightmost->keys, (void *) 20);
    ll_asc_insert(rightmost->keys, (void *) 25);
    rightmost->is_root = false;
    rightmost->is_leaf = true;
    rightmost->parent = right_internal;

    /* Connect all leaves */
    leftmost->next = second_from_left;
    second_from_left->next = middle;
    middle->next = second_from_right;
    second_from_right->next = rightmost;

    /* Let upper nodes have children */
    ll_tail_insert(tree->root->children, (void *) left_internal);
    ll_tail_insert(tree->root->children, (void *) right_internal);

    ll_tail_insert(left_internal->children, (void *) leftmost);
    ll_tail_insert(left_internal->children, (void *) second_from_left);
    ll_tail_insert(left_internal->children, (void *) middle);

    ll_tail_insert(right_internal->children, (void *) second_from_right);
    ll_tail_insert(right_internal->children, (void *) rightmost);

    /* The tree construction is done. Do the tests */

    /* Search for existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 13, &node, NULL) == true);
    assert(node == second_from_right);

    node = NULL;
    assert(bpt_search(tree, (void *) 16, &node, NULL) == true);
    assert(node == rightmost);

    node = NULL;
    assert(bpt_search(tree, (void *) 25, &node, NULL) == true);
    assert(node == rightmost);

    node = NULL;
    assert(bpt_search(tree, (void *) 11, &node, NULL) == true);
    assert(node == middle);

    node = NULL;
    assert(bpt_search(tree, (void *) 4, &node, NULL) == true);
    assert(node == leftmost);

    /* Search for non-existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 2, &node, NULL) == false);
    assert(node == leftmost);

    node = NULL;
    assert(bpt_search(tree, (void *) 14, &node, NULL) == false);
    assert(node == second_from_right);

    node = NULL;
    assert(bpt_search(tree, (void *) 30, &node, NULL) == false);
    assert(node == rightmost);

    bpt_dump_whole_tree(tree);

    /* Clean up */
    bpt_destroy(tree);
}

static void
insert_and_create_two_depth_tree(void){
    bpt_tree *tree;
    bpt_node *left, *right;

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    5, NULL);

    assert(bpt_insert(tree, (void *) 1, &emp) == true);
    assert(bpt_insert(tree, (void *) 2, &emp) == true);
    assert(bpt_insert(tree, (void *) 3, &emp) == true);
    assert(bpt_insert(tree, (void *) 3, &emp) == false); /* Duplicate key */
    assert(bpt_insert(tree, (void *) 4, &emp) == true);
    assert(bpt_insert(tree, (void *) 5, &emp) == true);
    assert(bpt_insert(tree, (void *) 6, &emp) == true);

    /* After the node split, are all nodes correctly connected ? */
    right = left = NULL;
    assert(bpt_search(tree, (void *) 1, &left, NULL) == true);
    assert(bpt_search(tree, (void *) 5, &right, NULL) == true);
    assert(left->next == right);
    assert(left->parent == tree->root);
    assert(ll_get_length(tree->root->keys) == 1); /* 4 */
    assert(ll_get_length(left->keys) == 3); /* 1, 2, 3 */
    assert(ll_get_length(right->keys) == 3); /* 4, 5, 6 */

    /* Check parent/children relationship too */
    assert(ll_ref_index_data(tree->root->children, 0) == left);
    assert(ll_ref_index_data(tree->root->children, 1) == right);

    /* Clean up */
    bpt_destroy(tree);
}

static void
insert_and_create_three_depth_tree(void){
    bpt_tree *tree;
    bpt_node *node;
    uintptr_t sorted_output[] =
	{ 2, 4, 9, 10, 11, 12, 14, 15, 18, 20, 30 };

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    3, NULL);

    assert(bpt_insert(tree, (void *) 18, &emp) == true);
    assert(bpt_insert(tree, (void *) 15, &emp) == true);
    assert(bpt_insert(tree, (void *) 2,  &emp) == true);
    assert(bpt_insert(tree, (void *) 4,  &emp) == true);
    assert(bpt_insert(tree, (void *) 9,  &emp) == true);
    assert(bpt_insert(tree, (void *) 30, &emp) == true);
    assert(bpt_insert(tree, (void *) 10, &emp) == true);
    assert(bpt_insert(tree, (void *) 11, &emp) == true);
    assert(bpt_insert(tree, (void *) 12, &emp) == true);
    assert(bpt_insert(tree, (void *) 14, &emp) == true);
    assert(bpt_insert(tree, (void *) 20, &emp) == true);

    /* Debug */
    node = NULL;
    assert(bpt_search(tree, (void *) 4, &node, NULL) == true);

    /* Check the order of leaf node values */
    full_keys_comparison_test(node, sorted_output);

    /* Search failure */
    assert(bpt_search(tree, (void *)  0, NULL, NULL) == false);
    assert(bpt_search(tree, (void *) 17, NULL, NULL) == false);
    assert(bpt_search(tree, (void *) 35, NULL, NULL) == false);

    /* Search success */
    node = NULL;
    assert(bpt_search(tree, (void *) 20, &node, NULL) == true);
    assert(node != NULL);

    node = NULL;
    assert(bpt_search(tree, (void *) 30, &node, NULL) == true);
    assert(node != NULL);

    node = NULL;
    assert(bpt_search(tree, (void *) 12, &node, NULL) == true);
    assert(node != NULL);

    /* Clean up */
    bpt_destroy(tree);
}

static void
test_even_number_max_keys(void){
    bpt_tree *tree;
    bpt_node *node;
    uintptr_t answer = 1,
	answers[] = { 1, 2, 3, 4, 5,
		      6, 7, 8, 9, 10,
		      11, 12, 13, 14, 15,
		      16, 17, 18, 19, 20 },
	reversed[]  = { 20, 19, 18, 17, 16,
			15, 14, 13, 12, 11,
			10, 9, 8, 7, 6, 5,
			4, 3, 2, 1 };

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    4, NULL);

    /* Insert 20 keys */
    for (answer = 20; answer >= 1; answer--)
	assert(bpt_insert(tree, (void *) answer, &emp) == true);

    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);

    /* Iterate all the registered keys */
    full_keys_comparison_test(node, answers);

    /* All search should be successful */
    app_loop_bpt_search(tree, 20, answers);

    /* Iterate in reverse order */
    node = NULL;
    assert(bpt_search(tree, (void *) 20, &node, NULL) == true);
    assert(node != NULL);

    /* Check the doubly linked list of leaf nodes */
    reverse_full_keys_comparison_test(node, reversed);

    /* Clean up */
    bpt_destroy(tree);
}

static void
remove_from_one_root(void){
    bpt_tree *tree;
    uintptr_t answers1[] = { 1, 2, 3 },
	answers2[] = { 1, 3 };

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    5, NULL);

    assert(bpt_insert(tree, (void *) 1, (void *) &emp) == true);
    assert(bpt_insert(tree, (void *) 2, (void *) &emp) == true);
    assert(bpt_insert(tree, (void *) 3, (void *) &emp) == true);
    assert(bpt_insert(tree, (void *) 4, (void *) &emp) == true);

    /* Set up only one root node */
    assert(tree->root->is_root == true);
    assert(tree->root->is_leaf == true);
    assert(ll_get_length(tree->root->keys) == 4);

    assert(bpt_delete(tree, (void *) 4, NULL) == true);
    assert(ll_get_length(tree->root->keys) == 3);
    full_keys_comparison_test(tree->root, answers1);

    /* Failure case */
    assert(bpt_delete(tree, (void *) 0, NULL) == false);
    assert(ll_get_length(tree->root->keys) == 3);

    /* Removal of second key in keys */
    assert(bpt_delete(tree, (void *) 2, NULL) == true);
    assert(ll_get_length(tree->root->keys) == 2);

    full_keys_comparison_test(tree->root, answers2);

    /* Clean up */
    bpt_destroy(tree);
}

static void
remove_from_two_depth_tree(void){
    bpt_tree *tree;
    bpt_node *node, *node2;
    uintptr_t i,
	leaves0[] = { 1, 2, 3, 4, 5, 6 },
	leaves1[] = { 1, 2, 5, 6 },
	leaves2[] = { 1, 5, 6 },
	leaves3[] = { 1, 6 },
	indexes1[] = { 2, 5 },
	indexes2[] = { 5, 6 };

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    3, NULL);

    /* Create the two depth tree */
    for (i = 1; i <= 6; i++)
	assert(bpt_insert(tree, (void *) i, &emp) == true);

    /* Dump the tree */
    bpt_dump_whole_tree(tree);

    /* Is the tree same as the expectation ? */
    assert(ll_get_length(tree->root->keys) == 2);

    /* Basic check of the left child */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    assert(ll_get_length(node->keys) == 2);

    /* Check the current all leaves */
    full_keys_comparison_test(node, leaves0);

    /* The middle child */
    node = NULL;
    assert(bpt_search(tree, (void *) 3, &node, NULL) == true);
    assert(ll_get_length(node->keys) == 2);

    /* The right child */
    node = NULL;
    assert(bpt_search(tree, (void *) 5, &node, NULL) == true);
    assert(ll_get_length(node->keys) == 2);

    /* Removal of one key from the middle child */
    node = NULL;
    assert(bpt_delete(tree, (void *) 4, NULL) == true);
    assert(bpt_search(tree, (void *) 3, &node, NULL) == true);
    assert(ll_get_length(node->keys) == 1);

    /* Removal to trigger borrowing from left child */
    node = NULL;
    assert(bpt_delete(tree, (void *) 3, NULL) == true);
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    full_keys_comparison_test(node, leaves1);

    /*
     * Before we go forward, check the current values of indexes.
     * Prove that we have '2' and '5' in the root node expectedly.
     */
    full_keys_comparison_test(node->parent, indexes1);

    /*
     * Another removal. This causes many things.
     *
     * (1) Borrow one key from the right child.
     *
     * (2) Along with the 1st step, update the index for the right node.
     *     This maintains the tree's key and children relationship among
     *     the middle node and the right node and parent node's key.
     *     The index key = 5 must be replaced by 6, the minimum value
     *     of the right child.
     *
     * (3) Remove the other 2 in the index because it's the removed key.
     *     The recursive call of bpt_delete_internal() handles this. Then,
     *     one of the index was removed, so fetch the minimum value of
     *     the right child, 5 and replace 2 with it.
     *
     * After all, the indexes should contain '5' and '6'.
     */
    node = NULL;
    assert(bpt_delete(tree, (void *) 2, NULL) == true);
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    full_keys_comparison_test(node, leaves2);

    /* Check the index updates */
    assert(ll_get_length(node->parent->keys) == 2);
    full_keys_comparison_test(node->parent, indexes2);

    /* Confirm that the remaining index key is only 6 after 5 removal */
    node = NULL;
    assert(bpt_delete(tree, (void *) 5, NULL) == true);
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    assert(ll_get_length(node->parent->keys) == 1);
    assert(node->parent->keys->head->data == (void *) 6);

    /* and that leaves must have only 1 and 6 */
    full_keys_comparison_test(node, leaves3);

    /* Ensure the two nodes are different but share the same parent */
    node = node2 = NULL;
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    assert(bpt_search(tree, (void *) 6, &node2, NULL) == true);
    assert(node->parent == node2->parent);

    /* Remove the value to trigger promotion */
    node = NULL;
    assert(bpt_delete(tree, (void *) 6, NULL) == true);
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    assert(node->is_root == true);

    /* Clean up */
    bpt_destroy(tree);
}

static void
remove_from_three_depth_tree(){
    bpt_tree *tree;
    bpt_node *node;
    uintptr_t i,
	/* First inserted data */
	insertion[] = { 1, 4, 7, 10, 17, 19,
			20, 21, 25, 28, 31, 42 },
	/* Test data sets */
	indexes0[]  = { 20 },
	indexes1[]  = { 7, 17 },
	indexes2[]  = { 25, 31 },
	indexes3[]  = { 21, 31 },
	indexes4[]  = { 31, 42 },
	indexes5[]  = { 7, 17, 20 },
	/* Check the leaves */
	leaves0[] = { 1, 4, 7, 10, 17, 19, 20, 42 },
	leaves1[] = { 1, 4, 7, 17, 19, 20, 42 },
	leaves2[] = { 1, 4, 7, 17, 19, 20 },
	leaves3[] = { 1, 4, 7, 9, 17, 19, 20 },
	leaves4[] = { 1, 9, 17, 19, 20 };

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    3, NULL);

    for (i = 0; i < 12; i++)
	bpt_insert(tree, (void *) insertion[i], &emp);

    app_loop_bpt_search(tree, 12, insertion);

    /* Does the whole tree match the expected structure ? */
    node = NULL;
    assert(ll_get_length(tree->root->keys) == 1);
    assert(ll_get_length(tree->root->children) == 2);
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    assert(node->parent->parent->is_root);
    assert(ll_get_length(node->parent->keys) == 2);
    assert(ll_get_length(node->next->next->next->parent->keys) == 2);
    assert(node->parent != node->next->next->next->parent);

    /* The root */
    one_node_keys_comparison_test(node->parent->parent,
				  indexes0);

    /* The left node */
    one_node_keys_comparison_test(node->parent,
				  indexes1);

    /* The left node */
    one_node_keys_comparison_test(node->next->next->next->parent,
				  indexes2);

    /* The leaf nodes */
    full_keys_comparison_test(node, insertion);

    /* Start the tests of bpt_delete() */
    assert(bpt_delete(tree, (void *) 28, NULL) == true);
    assert(bpt_delete(tree, (void *) 25, NULL) == true);

    /* Check the values of indexes */
    assert(tree->root->keys->head->data == (void *) 20);
    node = (bpt_node *) ll_ref_index_data(tree->root->children, 1);
    one_node_keys_comparison_test(node, indexes3);

    /* Check the root's right child after '21' removal */
    assert(bpt_delete(tree, (void *) 21, NULL) == true);
    assert(tree->root->keys->head->data == (void *) 20);
    node = (bpt_node *) ll_ref_index_data(tree->root->children, 1);
    assert(ll_get_length(node->keys) == 2);
    one_node_keys_comparison_test(node, indexes4);

    /* The internal node borrowing */
    assert(bpt_delete(tree, (void *) 31, NULL) == true);

    /* The leaf nodes */
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    full_keys_comparison_test(node, leaves0);
    app_loop_bpt_search(tree, 8, leaves0);

    /* Tiggers a new internal node borrowing */
    assert(bpt_delete(tree, (void *) 10, NULL) == true);

    /* Test key search */
    app_loop_bpt_search(tree, 7, leaves1);

    node = NULL;
    bpt_search(tree, (void *) 1, &node, NULL);
    full_keys_comparison_test(node, leaves1);

    /* Test the parent attributes for debugging */
    node = NULL;
    bpt_search(tree, (void *) 1, &node, NULL);
    assert(node->parent == node->next->parent &&
	   node->next->parent == node->next->next->parent);
    assert(node->parent != node->next->next->next->parent);

    node = NULL;
    bpt_search(tree, (void *) 20, &node, NULL);
    assert(node->parent == node->next->parent);

    /* Shrinks the height of the tree */
    assert(bpt_delete(tree, (void *) 42, NULL) == true);
    assert(ll_get_length(tree->root->keys) == 3);
    one_node_keys_comparison_test(tree->root, indexes5);

    /* Test key search */
    app_loop_bpt_search(tree, 6, leaves2);

    /*
     * Insert some new data so as to test another scenario
     * of tree's height shrink.
     */
    printf("debug : rebuild the tree to depth 3 tree\n");

    assert(bpt_insert(tree, (void *) 8, &emp) == true);
    assert(bpt_insert(tree, (void *) 9, &emp) == true);
    assert(bpt_insert(tree, (void *) 10, &emp) == true);

    assert(bpt_delete(tree, (void *) 8, NULL) == true);
    assert(bpt_delete(tree, (void *) 10, NULL) == true);

    /* Check the leaves just in case */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    full_keys_comparison_test(node, leaves3);

    assert(bpt_delete(tree, (void *) 7, NULL) == true);
    assert(bpt_delete(tree, (void *) 4, NULL) == true);

    app_loop_bpt_search(tree, 5, leaves4);

    bpt_search(tree, (void *) 17, &node, NULL);
    assert(ll_get_length(node->keys) == 2);

    /* Trigger a borrowing from right child */
    assert(bpt_delete(tree, (void *) 9, NULL) == true);

    assert(bpt_delete(tree, (void *) 17, NULL) == true);
    assert(bpt_delete(tree, (void *) 19, NULL) == true);
    assert(bpt_delete(tree, (void *) 20, NULL) == true);

    /* Except for one key, all keys should be removed */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node, NULL) == true);
    assert(node->parent == NULL);
    assert(node->prev == NULL);
    assert(node->next == NULL);
    assert(ll_get_length(tree->root->keys) == 1);
    assert(tree->root->keys->head->data == (void *) 1);

    assert(bpt_delete(tree, (void *) 1, NULL) == true);
    assert(ll_get_length(tree->root->keys) == 0);

    /* Clean up */
    bpt_destroy(tree);
}

static void
keys_test_more_data(uint16_t max_keys){
    bpt_tree *tree;
    uintptr_t i, max = 4096;

    tree = bpt_init(employee_key_compare,
		    employee_key_free,
		    employee_record_free,
		    max_keys, NULL);

    for (i = 1; i < max; i++)
	assert(bpt_insert(tree, (void *) i, (void *) &emp) == true);

    for (i = max - 1; i >= 1; i--)
	assert(bpt_search(tree, (void *) i, NULL, NULL) == true);

    for (i = 1; i < max; i++)
	assert(bpt_delete(tree, (void *) i, NULL) == true);

    assert(ll_get_length(tree->root->keys) == 0);
    assert(tree->root->is_leaf == true);
    assert(tree->root->is_root == true);

    /* Do the same thing in reverse order */
    for (i = max; i >= 1; i--)
	assert(bpt_insert(tree, (void *) i, (void *) &emp) == true);

    for (i = max; i >= 1; i--)
	assert(bpt_delete(tree, (void *) i, NULL) == true);

    assert(ll_get_length(tree->root->keys) == 0);
    assert(tree->root->is_leaf == true);
    assert(tree->root->is_root == true);

    /* Clean up */
    bpt_destroy(tree);
}

static void
keys_test_bpt_search(void){
    printf("<Search key test from single node>\n");
    search_single_node_test();

    printf("<Search key from depth 2 tree>\n");
    search_two_depth_nodes_test();

    printf("<Search key from depth 3 tree>\n");
    search_three_depth_nodes_test();
}

static void
keys_test_bpt_insert(void){
    printf("<Create depth 2 tree>\n");
    insert_and_create_two_depth_tree();

    printf("<Create depth 3 tree>\n");
    insert_and_create_three_depth_tree();

    printf("<Other variant of depth 3 tree>\n");
    test_even_number_max_keys();
}

static void
keys_test_bpt_remove(void){
    printf("<Remove key from 1 root node>\n");
    remove_from_one_root();

    printf("<Remove key from depth 2 tree>\n");
    remove_from_two_depth_tree();

    printf("<Remove key from depth 3 tree>\n");
    remove_from_three_depth_tree();
}

static void
keys_test_combined(){
    printf("<Insert and remove larger number of keys>\n");
    keys_test_more_data(3);

    printf("<Tree with higher value of max keys -part1->\n");
    keys_test_more_data(8);

    printf("<Tree with higher value of max keys -part2->\n");
    keys_test_more_data(9);
}

int
main(int argc, char **argv){

    printf("Perform the tests for key search, insert and delete...\n");

    keys_test_bpt_search();
    keys_test_bpt_insert();
    keys_test_bpt_remove();

    printf("Perform more advanced tests...\n");

    keys_test_combined();

    printf("All tests are done gracefully\n");

    return 0;
}
