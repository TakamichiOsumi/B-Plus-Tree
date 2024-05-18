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
 * Will store employee *. Return one id by casting data
 * to employee *. The comparison of two ids will be done
 * with employee_key_compare().
 */
static void *
employee_key_access_from_employee(void *data){
    return data;
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
    bpt_node *last_node = NULL;

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
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 1, &last_node) == true);
    assert(last_node == tree->root);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 2, &last_node) == true);
    assert(last_node == tree->root);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 4, &last_node) == true);
    assert(last_node == tree->root);

    /* Non-existing keys */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 0, &last_node) == false);
    assert(last_node == tree->root);
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 5, &last_node) == false);
    assert(last_node == tree->root);
}

static void
search_two_depth_nodes_test(void){
    bpt_tree *tree;
    bpt_node *left, *right, *last_node;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    3);

    /* the root node */
    ll_asc_insert(tree->root->keys, (void *) 4);
    tree->root->is_leaf = false;

    /* the left leaf node */
    left = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(left->keys, (void *) 1);
    ll_asc_insert(left->keys, (void *) 2);
    ll_asc_insert(left->keys, (void *) 3);
    ll_tail_insert(tree->root->children, (void *) left);
    left->is_root = false;
    left->is_leaf = true;
    left->parent = tree->root;

    /* the right leaf node */
    right = bpt_gen_root_callbacks_node(tree);
    ll_asc_insert(right->keys, (void *) 4);
    ll_asc_insert(right->keys, (void *) 5);

    ll_tail_insert(tree->root->children, (void *) right);
    right->is_root = false;
    right->is_leaf = true;
    right->parent = tree->root;

    /* Connect leaves */
    left->next = right;

    /* Now, do the search of existing keys */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 1, &last_node) == true);
    assert(last_node == left);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 4, &last_node) == true);
    assert(last_node == right);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 5, &last_node) == true);
    assert(last_node == right);

    /* Search non-existing keys */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 0, &last_node) == false);
    assert(last_node == left);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 6, &last_node) == false);
    assert(last_node == right);
}

static void
search_three_depth_nodes_test(void){
    bpt_tree *tree;
    /* internal nodes */
    bpt_node *left_internal, *right_internal;
    /* leaf nodes */
    bpt_node *leftmost, *second_from_left, *middle,
	*second_from_right, *rightmost;
    /* for test */
    bpt_node *last_node;

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
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 13, &last_node) == true);
    assert(last_node == second_from_right);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 16, &last_node) == true);
    assert(last_node == rightmost);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 25, &last_node) == true);
    assert(last_node == rightmost);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 11, &last_node) == true);
    assert(last_node == middle);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 4, &last_node) == true);
    assert(last_node == leftmost);

    /* Search for non-existing keys */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 2, &last_node) == false);
    assert(last_node == leftmost);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 14, &last_node) == false);
    assert(last_node == second_from_right);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 30, &last_node) == false);
    assert(last_node == rightmost);
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
    assert(bpt_insert(tree, (void *) 3, &e3) == false); /* duplicate key */
    assert(bpt_insert(tree, (void *) 4, &e4) == true);
    assert(bpt_insert(tree, (void *) 5, &e5) == true);
    assert(bpt_insert(tree, (void *) 6, &e6) == true);

    /* After the node split, are all nodes correctly connected ? */
    left = NULL;
    assert(bpt_search(tree->root, (void *) 1, &left) == true);
    right = NULL;
    assert(bpt_search(tree->root, (void *) 5, &right) == true);
    assert(left->next == right);
    assert(left->parent == tree->root);
    assert(ll_get_length(tree->root->keys) == 1); /* 4 */
    assert(ll_get_length(left->keys) == 3); /* 1, 2, 3 */
    assert(ll_get_length(right->keys) == 3); /* 4, 5, 6 */

    /* Check parent/children relationship too */
    assert(ll_get_index_node(tree->root->children, 0) == left);
    assert(ll_get_index_node(tree->root->children, 1) == right);
}

static void
insert_and_create_three_depth_tree(void){
    bpt_tree *tree;
    bpt_node *last_node;
    void *p;
    int ans_index = 0, sorted_output[] =
	{ 2, 4, 9, 10, 11, 12, 14, 15, 18, 20, 30};

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

    /* debug */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 4, &last_node) == true);
    assert(last_node != NULL);

    while(true){
	ll_begin_iter(last_node->keys);
	while((p = ll_get_iter_node(last_node->keys)) != NULL){
	    if ((uintptr_t) p != sorted_output[ans_index++]){
		printf("the expected order is not same as the leaves order\n");
		exit(-1);
	    }
	}
	ll_end_iter(last_node->keys);
	if ((last_node = last_node->next) == NULL)
	    break;
    }

    /* search failure */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 0, &last_node) == false);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 17, &last_node) == false);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 35, &last_node) == false);

    /* search success */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 20, &last_node) == true);
    assert(last_node != NULL);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 30, &last_node) == true);
    assert(last_node != NULL);

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 12, &last_node) == true);
    assert(last_node != NULL);
}

static void
test_even_number_m(void){
    bpt_tree *tree;
    bpt_node *last_node;
    void *p;
    uintptr_t answer = 1;
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

    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 1, &last_node) == true);
    assert(last_node != NULL);

    /* Iterate all the registered keys */
    answer = 1;
    while(true){
	ll_begin_iter(last_node->keys);
	while((p = ll_get_iter_node(last_node->keys)) != NULL){
	    if ((uintptr_t) p != answer){
		printf("the expectation contradicted the order of leaves (%lu vs. %lu)\n",
		       (uintptr_t) p, (uintptr_t) answer);
		exit(-1);
	    }
	    answer++;
	}
	ll_end_iter(last_node->keys);
	if ((last_node = last_node->next) == NULL)
	    break;
    }

    for (answer = 1; answer <= 20; answer++){
	last_node = NULL;
	assert(bpt_search(tree->root, (void *) answer, &last_node) == true);
    }

    /* Iterate in reverse order */
    last_node = NULL;
    assert(bpt_search(tree->root, (void *) 20, &last_node) == true);
    assert(last_node != NULL);

    answer = 20;
    while(true){
	for (i = ll_get_length(last_node->keys) - 1; i >= 0; i--){
	    p = ll_get_index_node(last_node->keys, i);
	    if ((uintptr_t) p != answer){
		printf("the expectation contradicted the order of leaves (%lu vs. %lu)\n",
		       (uintptr_t) p, (uintptr_t) answer);
	    }else{
		printf("debug : iterating leaf nodes got %lu\n",
		       (uintptr_t) p);
	    }
	    answer--;
	}
	if ((last_node = last_node->prev) == NULL)
	    break;
    }
}

static void
remove_from_one_root(void){
    bpt_tree *tree;
    void *p;
    uintptr_t answer = 1;
    int i;

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

    ll_begin_iter(tree->root->keys);
    for (i = 0; i < ll_get_length(tree->root->keys); i++){
	p = ll_get_iter_node(tree->root->keys);
	if (answer != (uintptr_t) p){
	    printf("the value is not same as expectation, %lu vs %lu\n",
		   answer, (uintptr_t) p);
	    exit(-1);
	}
	answer++;
    }
    ll_end_iter(tree->root->keys);
}

/*
 * Ignore the values of 'M' in each tree.
 *
 * That doesn't matter for this set of tests.
 */
static void
test_bpt_search(void){
    printf("<search key test from single node>\n");
    search_single_node_test();

    printf("<search key from depth 2 tree>\n");
    search_two_depth_nodes_test();

    printf("<search key from depth 3 tree>\n");
    search_three_depth_nodes_test();
}

static void
test_bpt_insert(void){
    printf("<create depth 2 tree>\n");
    insert_and_create_two_depth_tree();

    printf("<create depth 3 tree>\n");
    insert_and_create_three_depth_tree();

    printf("<other variant of depth 3 tree>\n");
    test_even_number_m();
}

static void
test_bpt_remove(void){
    printf("<remove key from 1 node>\n");
    remove_from_one_root();
}

int
main(int argc, char **argv){

    test_bpt_search();
    test_bpt_insert();
    test_bpt_remove();

    printf("All tests are done gracefully\n");

    return 0;
}
