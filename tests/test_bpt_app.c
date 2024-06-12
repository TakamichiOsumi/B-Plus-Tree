#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../b_plus_tree.h"

typedef struct employee {
    uintptr_t id;
    char *name;
} employee;

/*
 * linked_list *keys
 *
 * Will store uintptr_t *. Return one id.
 */
static void *
employee_key_access(void *data){
    return data;
}

static int
employee_key_compare(void *key1, void *key2){
    uintptr_t k1 = (uintptr_t) key1,
        k2 = (uintptr_t) key2;

    /* printf("%lu vs. %lu\n", k1, k2); */

    if (k1 < k2){
        return -1;
    }else if (k1 == k2){
        return 0;
    }else{
        return 1;
    }
}

static void
employee_free(void *data){}

/*
 * linked_list *children
 *
 * Will store employee *. The data' key won't be
 * used at this moment.
 */
static void *
employee_key_access_from_employee(void *data){
    return data;
}

static void
leaf_keys_comparison_test(linked_list *keys, uintptr_t answers[]){
    int i;
    void *p;

    assert(keys != NULL);

    ll_begin_iter(keys);
    for (i = 0; i < ll_get_length(keys); i++){
	p = ll_get_iter_data(keys);
	if ((uintptr_t) p != answers[i]){
	    printf("the value is not same as expectation, %lu vs %lu\n",
		   (uintptr_t) p, answers[i]);
	    exit(-1);
	}else{
	    printf("debug : found %lu expectedly\n", answers[i]);
	}
    }
    ll_end_iter(keys);
}

/* Check only one node */
static void
one_node_keys_comparison_test(bpt_node *node, uintptr_t answers[]){
    int i = 0;
    void *p;

    ll_begin_iter(node->keys);
    while ((p = ll_get_iter_data(node->keys)) != NULL){
	if ((uintptr_t) p != answers[i]){
	    printf("the expected value is not same as leaf node value. %lu vs. %lu\n",
		   (uintptr_t) p, answers[i]);
	    assert(0);
	}else{
	    printf("debug : found %lu expectedly\n", answers[i]);
	}
	i++;
    }
    ll_end_iter(node->keys);
}

/* Check the full nodes at the same level */
static void
full_keys_comparison_test(bpt_node *node, uintptr_t answers[]){
    int i = 0;
    void *p;

    assert(node != NULL);

    while(true){
	/* Check each key at the same level of node */
	ll_begin_iter(node->keys);
	while((p = (void *) ll_get_iter_data(node->keys)) != NULL){
	    if ((uintptr_t) p != answers[i]){
		printf("the expected value is not same as leaf node value. %lu vs. %lu\n",
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

employee *iter,
    e1 = { 1, "foo" },
    e2 = { 2, "bar" },
    e3 = { 3, "bazz" },
    e4 = { 4, "xxxx" },
    e5 = { 5, "yyyy" },
    e6 = { 6, "aaa"  },
    e7 = { 7, "bbb"  },
    e8 = { 8, "ccc"  };

static void
search_single_node_test(void){
    bpt_tree *tree;
    bpt_node *node = NULL;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    4);

    /* Construct one root without any leaf nodes */
    ll_asc_insert(tree->root->keys, (void *) 4);
    ll_asc_insert(tree->root->children, (void *) &e4);
    ll_asc_insert(tree->root->keys, (void *) 1);
    ll_asc_insert(tree->root->children, (void *) &e1);
    ll_asc_insert(tree->root->keys, (void *) 2);
    ll_asc_insert(tree->root->children, (void *) &e2);

    /* Exact key match */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node) == true);
    assert(node == tree->root);

    node = NULL;
    assert(bpt_search(tree, (void *) 2, &node) == true);
    assert(node == tree->root);

    node = NULL;
    assert(bpt_search(tree, (void *) 4, &node) == true);
    assert(node == tree->root);

    /* Non-existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 50, &node) == false);
    assert(node == tree->root);
    node = NULL;
    assert(bpt_search(tree, (void *) 5, &node) == false);
    assert(node == tree->root);
}

static void
search_two_depth_nodes_test(void){
    bpt_tree *tree;
    bpt_node *left, *right, *node;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    3);

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
    assert(bpt_search(tree, (void *) 2, &node) == true);
    assert(node == left);

    node = NULL;
    assert(bpt_search(tree, (void *) 5, &node) == true);
    assert(node == right);

    node = NULL;
    assert(bpt_search(tree, (void *) 6, &node) == true);
    assert(node == right);

    /* Search non-existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node) == false);
    assert(node == left);

    node = NULL;
    assert(bpt_search(tree, (void *) 7, &node) == false);
    assert(node == right);
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

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    5);

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
    assert(bpt_search(tree, (void *) 13, &node) == true);
    assert(node == second_from_right);

    node = NULL;
    assert(bpt_search(tree, (void *) 16, &node) == true);
    assert(node == rightmost);

    node = NULL;
    assert(bpt_search(tree, (void *) 25, &node) == true);
    assert(node == rightmost);

    node = NULL;
    assert(bpt_search(tree, (void *) 11, &node) == true);
    assert(node == middle);

    node = NULL;
    assert(bpt_search(tree, (void *) 4, &node) == true);
    assert(node == leftmost);

    /* Search for non-existing keys */
    node = NULL;
    assert(bpt_search(tree, (void *) 2, &node) == false);
    assert(node == leftmost);

    node = NULL;
    assert(bpt_search(tree, (void *) 14, &node) == false);
    assert(node == second_from_right);

    node = NULL;
    assert(bpt_search(tree, (void *) 30, &node) == false);
    assert(node == rightmost);
}

static void
insert_and_create_two_depth_tree(void){
    bpt_tree *tree;
    bpt_node *left, *right;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    5);

    assert(bpt_insert(tree, (void *) 1, &e1) == true);
    assert(bpt_insert(tree, (void *) 2, &e2) == true);
    assert(bpt_insert(tree, (void *) 3, &e3) == true);
    assert(bpt_insert(tree, (void *) 3, &e3) == false); /* Duplicate key */
    assert(bpt_insert(tree, (void *) 4, &e4) == true);
    assert(bpt_insert(tree, (void *) 5, &e5) == true);
    assert(bpt_insert(tree, (void *) 6, &e6) == true);

    /* After the node split, are all nodes correctly connected ? */
    right = left = NULL;
    assert(bpt_search(tree, (void *) 1, &left) == true);
    assert(bpt_search(tree, (void *) 5, &right) == true);
    assert(left->next == right);
    assert(left->parent == tree->root);
    assert(ll_get_length(tree->root->keys) == 1); /* 4 */
    assert(ll_get_length(left->keys) == 3); /* 1, 2, 3 */
    assert(ll_get_length(right->keys) == 3); /* 4, 5, 6 */

    /* Check parent/children relationship too */
    assert(ll_ref_index_data(tree->root->children, 0) == left);
    assert(ll_ref_index_data(tree->root->children, 1) == right);
}

static void
insert_and_create_three_depth_tree(void){
    bpt_tree *tree;
    bpt_node *node;
    uintptr_t sorted_output[] =
	{ 2, 4, 9, 10, 11, 12, 14, 15, 18, 20, 30 };

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free, 3);

    assert(bpt_insert(tree, (void *) 18, &e1) == true);
    assert(bpt_insert(tree, (void *) 15, &e1) == true);
    assert(bpt_insert(tree, (void *) 2,  &e1) == true);
    assert(bpt_insert(tree, (void *) 4,  &e1) == true);
    assert(bpt_insert(tree, (void *) 9,  &e1) == true);
    assert(bpt_insert(tree, (void *) 30, &e1) == true);
    assert(bpt_insert(tree, (void *) 10, &e1) == true);
    assert(bpt_insert(tree, (void *) 11, &e1) == true);
    assert(bpt_insert(tree, (void *) 12, &e1) == true);
    assert(bpt_insert(tree, (void *) 14, &e1) == true);
    assert(bpt_insert(tree, (void *) 20, &e1) == true);

    /* Debug */
    node = NULL;
    assert(bpt_search(tree, (void *) 4, &node) == true);

    /* Check the order of leaf node values */
    full_keys_comparison_test(node, sorted_output);

    /* Search failure */
    node = NULL;
    assert(bpt_search(tree, (void *) 0, &node) == false);

    node = NULL;
    assert(bpt_search(tree, (void *) 17, &node) == false);

    node = NULL;
    assert(bpt_search(tree, (void *) 35, &node) == false);

    /* Search success */
    node = NULL;
    assert(bpt_search(tree, (void *) 20, &node) == true);
    assert(node != NULL);

    node = NULL;
    assert(bpt_search(tree, (void *) 30, &node) == true);
    assert(node != NULL);

    node = NULL;
    assert(bpt_search(tree, (void *) 12, &node) == true);
    assert(node != NULL);
}

static void
test_even_number_max_children(void){
    bpt_tree *tree;
    bpt_node *node;
    void *p;
    uintptr_t answer = 1,
	answers[] = { 1, 2, 3, 4, 5,
		      6, 7, 8, 9, 10,
		      11, 12, 13, 14, 15,
		      16, 17, 18, 19, 20 };
    int i;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free, 4 /* even number */);

    /* Insert 20 keys */
    for (answer = 20; answer >= 1; answer--)
	assert(bpt_insert(tree, (void *) answer, &e1) == true);

    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node) == true);

    /* Iterate all the registered keys */
    full_keys_comparison_test(node, answers);

    /* All search should be successful */
    for (answer = 1; answer <= 20; answer++){
	node = NULL;
	assert(bpt_search(tree, (void *) answer, &node) == true);
    }

    /* Iterate in reverse order */
    node = NULL;
    assert(bpt_search(tree, (void *) 20, &node) == true);
    assert(node != NULL);

    answer = 20;
    while(true){
	for (i = ll_get_length(node->keys) - 1; i >= 0; i--){
	    p = ll_ref_index_data(node->keys, i);
	    if ((uintptr_t) p != answer){
		printf("the expectation contradicted the order of leaves (%lu vs. %lu)\n",
		       (uintptr_t) p, (uintptr_t) answer);
	    }else{
		printf("debug : iterating leaf nodes got %lu\n",
		       (uintptr_t) p);
	    }
	    answer--;
	}
	if ((node = node->prev) == NULL)
	    break;
    }
}

static void
remove_from_one_root(void){
    bpt_tree *tree;
    uintptr_t answers1[] = { 1, 2, 3 },
	answers2[] = { 1, 3 };

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free, 5);

    assert(bpt_insert(tree, (void *) 1, &e1) == true);
    assert(bpt_insert(tree, (void *) 2, &e1) == true);
    assert(bpt_insert(tree, (void *) 3, &e1) == true);
    assert(bpt_insert(tree, (void *) 4, &e1) == true);

    /* Set up only one root node */
    assert(tree->root->is_root == true);
    assert(tree->root->is_leaf == true);
    assert(ll_get_length(tree->root->keys) == 4);

    assert(bpt_delete(tree, (void *) 4) == true);
    assert(ll_get_length(tree->root->keys) == 3);
    leaf_keys_comparison_test(tree->root->keys, answers1);

    /* Failure case */
    assert(bpt_delete(tree, (void *) 0) == false);
    assert(ll_get_length(tree->root->keys) == 3);

    /* Removal of second key in keys */
    assert(bpt_delete(tree, (void *) 2) == true);
    assert(ll_get_length(tree->root->keys) == 2);

    leaf_keys_comparison_test(tree->root->keys, answers2);
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

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free, 3);

    /* Create the two depth tree */
    for (i = 1; i <= 6; i++)
	assert(bpt_insert(tree, (void *) i, &e1) == true);

    /* Is the tree same as the expectation ? */
    assert(ll_get_length(tree->root->keys) == 2);

    /* Basic check of the left child */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node) == true);
    assert(ll_get_length(node->keys) == 2);
    full_keys_comparison_test(node, leaves0);

    /* The middle child */
    node = NULL;
    assert(bpt_search(tree, (void *) 3, &node) == true);
    assert(ll_get_length(node->keys) == 2);

    /* The right child */
    node = NULL;
    assert(bpt_search(tree, (void *) 5, &node) == true);
    assert(ll_get_length(node->keys) == 2);

    /* Removal of one key from the middle child */
    node = NULL;
    assert(bpt_delete(tree, (void *) 4) == true);
    assert(bpt_search(tree, (void *) 3, &node) == true);
    assert(ll_get_length(node->keys) == 1);

    /* Removal to trigger borrowing from left child */
    node = NULL;
    assert(bpt_delete(tree, (void *) 3) == true);
    assert(bpt_search(tree, (void *) 1, &node) == true);
    full_keys_comparison_test(node, leaves1);

    /*
     * Before we go forward, check the current values of indexes.
     * Prove that we have '2' and '5' in the root node expectedly.
     */
    full_keys_comparison_test(node->parent, indexes1);

    /*
     * Another Removal. Many things happen by this removal.
     *
     * (1) Borrow one key from the right child.
     *
     * (2) With the 1st step, update the index for the right node.
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
    assert(bpt_delete(tree, (void *) 2) == true);
    assert(bpt_search(tree, (void *) 1, &node) == true);
    full_keys_comparison_test(node, leaves2);

    /* Check the index updates */
    assert(ll_get_length(node->parent->keys) == 2);
    full_keys_comparison_test(node->parent, indexes2);

    /* Confirm that the remaining index key is only 6 after 5 removal */
    node = NULL;
    assert(bpt_delete(tree, (void *) 5) == true);
    assert(bpt_search(tree, (void *) 1, &node) == true);
    assert(ll_get_length(node->parent->keys) == 1);
    assert(node->parent->keys->head->data == (void *) 6);

    /* and that leaves must have only 1 and 6 */
    full_keys_comparison_test(node, leaves3);

    /* Ensure the two nodes are different but share the same parent */
    node = node2 = NULL;
    assert(bpt_search(tree, (void *) 1, &node) == true);
    assert(bpt_search(tree, (void *) 6, &node2) == true);
    assert(node->parent == node2->parent);

    /* Remove the value to trigger promotion */
    node = NULL;
    assert(bpt_delete(tree, (void *) 6) == true);
    assert(bpt_search(tree, (void *) 1, &node) == true);
    assert(node->is_root == true);
}

static void
remove_from_three_depth_tree(){
    bpt_tree *tree;
    bpt_node *node;
    uintptr_t i,
	insertion[] = { 1, 4, 7, 10, 17, 19,
			20, 21, 25, 28, 31, 42 },
	answers0[] = { 20 },
	answers1[] = { 7, 17 },
	answers2[] = { 25, 31 },
	answers3[] = { 21, 31 },
	answers4[] = { 31, 42 },
	answers5[] = { 7 },
	answers6[] = { 20, 42 },
	answers7[] = { 1, 4, 7, 10, 17, 19, 20, 42 },
	answers8[] = { 7, 17 },
	answers9[] = { 42 },
	answers10[] = { 1, 4, 7, 17, 19, 20, 42 },
	answers11[] = { 1, 4, 7, 17, 19, 20 },
	answers12[] = { 7, 17, 20 },
	answers13[] = { 1, 4, 7, 9, 17, 19, 20 };

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free, 3);

    for (i = 0; i < 12; i++){
	bpt_insert(tree, (void *) insertion[i], &e1);
	printf("debug : done with insertion of %lu\n", insertion[i]);
    }

    for (i = 0; i < 12; i++){
	node = NULL;
	assert(bpt_search(tree, (void *) insertion[i], &node) == true);
	printf("debug : found %lu\n", insertion[i]);
    }

    /* Does the whole tree match the expected structure ? */
    node = NULL;
    assert(ll_get_length(tree->root->keys) == 1);
    assert(ll_get_length(tree->root->children) == 2);
    assert(bpt_search(tree, (void *) 1, &node) == true);
    assert(node->parent->parent->is_root);
    assert(ll_get_length(node->parent->keys) == 2);
    assert(ll_get_length(node->next->next->next->parent->keys) == 2);
    assert(node->parent != node->next->next->next->parent);

    /* The root */
    one_node_keys_comparison_test(node->parent->parent, answers0);

    /* The left node */
    one_node_keys_comparison_test(node->parent, answers1);

    /* The left node */
    one_node_keys_comparison_test(node->next->next->next->parent, answers2);

    /* The leaf nodes */
    full_keys_comparison_test(node, insertion);

    /* Start the tests of bpt_delete() */
    assert(bpt_delete(tree, (void *) 28) == true);
    assert(bpt_delete(tree, (void *) 25) == true);

    /* Check the values of indexes */
    assert(tree->root->keys->head->data == (void *) 20);
    one_node_keys_comparison_test((bpt_node *) ll_ref_index_data(tree->root->children, 1),
				  answers3);

    /* Check the root's right child after '21' removal */
    assert(bpt_delete(tree, (void *) 21) == true);
    assert(tree->root->keys->head->data == (void *) 20);
    node = (bpt_node *) ll_ref_index_data(tree->root->children, 1);
    assert(ll_get_length(node->keys) == 2);
    one_node_keys_comparison_test(node, answers4);

    /* The internal node borrowing */
    printf("debug : start the internal node borrowing\n");
    assert(bpt_delete(tree, (void *) 31) == true);

    /* The root */
    assert(ll_get_length(tree->root->keys) == 1);
    assert(tree->root->keys->head->data == (void *) 17);

    /* The left child */
    node = (bpt_node *) ll_ref_index_data(tree->root->children, 0);
    assert(ll_get_length(node->keys) == 1);
    one_node_keys_comparison_test(node, answers5);

     /* The right child */
    node = (bpt_node *) ll_ref_index_data(tree->root->children, 1);
    assert(ll_get_length(node->keys) == 2);
    one_node_keys_comparison_test(node, answers6);

    /* The leaf nodes */
    assert(bpt_search(tree, (void *) 1, &node));
    full_keys_comparison_test(node, answers7);
    for (i = 0; i < 8; i++){
	node = NULL;
	assert(bpt_search(tree, (void *) answers7[i], &node) == true);
	assert(node != NULL);
    }

    /* Another removal triggers a new internal node borrowing */
    assert(bpt_delete(tree, (void *) 10) == true);

    /* The root */
    assert(tree->root->keys->head->data == (void *) 20);

    /* The left child */
    node = (bpt_node *) ll_ref_index_data(tree->root->children, 0);
    assert(ll_get_length(node->keys) == 2);
    one_node_keys_comparison_test(node, answers8);

    /* The right child */
    node = (bpt_node *) ll_ref_index_data(tree->root->children, 1);
    assert(ll_get_length(node->keys) == 1);
    one_node_keys_comparison_test(node, answers9);

    bpt_search(tree, (void *) 1, &node);
    assert(ll_get_length(node->parent->children) == 3);

    for (i = 0; i < 7; i++){
	node = NULL;
	assert(bpt_search(tree, (void *) answers10[i], &node) == true);
    }

    /* Test the 'parent' attributes for debug */
    node = NULL;
    bpt_search(tree, (void *) 1, &node);
    assert(node->parent == node->next->parent &&
	   node->next->parent == node->next->next->parent);
    assert(node->parent != node->next->next->next->parent);

    node = NULL;
    bpt_search(tree, (void *) 20, &node);
    assert(node->parent == node->next->parent);

    /* This removal shrinks the height of the tree */
    assert(bpt_delete(tree, (void *) 42) == true);
    assert(ll_get_length(tree->root->keys) == 3);
    one_node_keys_comparison_test(tree->root, answers12);

    for (i = 0; i < 6; i++){
	node = NULL;
	assert(bpt_search(tree, (void *) answers11[i], &node) == true);
	printf("debug : found %lu\n", (uintptr_t) answers11[i]);
    }

    /*
     * Insert some new data so as to test another scenario of tree's height
     * shrink.
     */
    printf("debug : rebuild the tree to depth 3 tree\n");

    assert(bpt_insert(tree, (void *) 8, &e1) == true);
    assert(bpt_insert(tree, (void *) 9, &e1) == true);
    assert(bpt_insert(tree, (void *) 10, &e1) == true);

    assert(bpt_delete(tree, (void *) 8) == true);
    assert(bpt_delete(tree, (void *) 10) == true);

    /* Check the leaves */
    node = NULL;
    assert(bpt_search(tree, (void *) 1, &node) == true);
    full_keys_comparison_test(node, answers13);
}

static void
test_bpt_search(void){
    printf("<Search key test from single node>\n");
    search_single_node_test();

    printf("<Search key from depth 2 tree>\n");
    search_two_depth_nodes_test();

    printf("<Search key from depth 3 tree>\n");
    search_three_depth_nodes_test();
}

static void
test_bpt_insert(void){
    printf("<Create depth 2 tree>\n");
    insert_and_create_two_depth_tree();

    printf("<Create depth 3 tree>\n");
    insert_and_create_three_depth_tree();

    printf("<Other variant of depth 3 tree>\n");
    test_even_number_max_children();
}

static void
test_bpt_remove(void){
    printf("<Remove key from 1 root node>\n");
    remove_from_one_root();

    printf("<Remove key from depth 2 tree>\n");
    remove_from_two_depth_tree();

    printf("<Remove key from depth 3 tree>\n");
    remove_from_three_depth_tree();
}

int
main(int argc, char **argv){

    test_bpt_search();
    test_bpt_insert();
    test_bpt_remove();

    printf("All tests are done gracefully\n");

    return 0;
}
