#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "b_plus_tree.h"

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

    printf("k1 = %lu vs k2 = %lu\n", k1, k2);

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
    employee *e;

    e = (employee *) data;

    return (void *) e->id;
}

employee *iter,
    e1 = { 1, "foo" },
    e2 = { 2, "bar" },
    e3 = { 3, "bazz" },
    e4 = { 4, "xxxx" },
    e5 = { 5, "yyyy" };

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
search_three_depth_nodes_test(void){}

int
main(int argc, char **argv){

    printf("<search bpt key test from single node>\n");
    search_single_node_test();

    printf("<search bpt key from depth 2 tree>\n");
    search_two_depth_nodes_test();

    printf("<search bpt key from depth 3 tree>\n");
    search_three_depth_nodes_test();

    printf("All tests are done gracefully\n");

    return 0;
}
