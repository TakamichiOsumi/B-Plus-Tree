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

    printf("%lu vs %lu\n", k1, k2);
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
    void *p;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    4);

    /* Construct one root without any leaf nodes */
    ll_asc_insert(tree->root->keys, (void *) 1);
    ll_asc_insert(tree->root->children, (void *) & e1);
    ll_asc_insert(tree->root->keys, (void *) 2);
    ll_asc_insert(tree->root->children, (void *) & e2);
    ll_asc_insert(tree->root->keys, (void *) 3);
    ll_asc_insert(tree->root->children, (void *) & e3);
    ll_asc_insert(tree->root->keys, (void *) 4);
    ll_asc_insert(tree->root->children, (void *) & e4);

    ll_begin_iter(tree->root->keys);
    while((p = ll_get_iter_node(tree->root->keys)) != NULL){
	printf("keys %lu\n", (uintptr_t) p);
    }
    ll_end_iter(tree->root->keys);

    assert(bpt_search(tree->root, (void *) 1) == tree->root);
    assert(bpt_search(tree->root, (void *) 0) == tree->root);
}

int
main(int argc, char **argv){

    printf("<search bpt key test>\n");
    search_single_node_test();

    printf("All tests are done gracefully\n");

    return 0;
}
