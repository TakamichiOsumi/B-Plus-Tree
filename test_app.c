#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "b_plus_tree.h"

/* dummy data */
typedef struct employee_data {
    uintptr_t id;
    char *name;
} employee_data;

employee_data *iter,
    e1 = { 1, "foo" },
    e2 = { 2, "bar" },
    e3 = { 3, "bazz" },
    e4 = { 4, "xxxx" },
    e5 = { 5, "yyyy" },
    e6 = { 6, "zzzz" },
    e7 = { 7, "xyz"  },
    e8 = { 8, "xyzz" };

static int
emp_key_compare(void *k1, void *k2){
    if ((uintptr_t) k1 < (uintptr_t) k2){
	return -1;
    }else if ((uintptr_t) k1 == (uintptr_t) k2){
	return 0;
    }else{
	return 1;
    }
}

static void *
emp_key_access(void *key){
    return (void *) key;
}

/* do nothing. no dynamic memory */
static void
emp_free(void *emp){}

/*
static void
emp_print(void *p){
    employee_data *emp = (employee_data *) p;

    printf("id = %d, name = %s\n",
	   (int) emp->id, emp->name);
}
*/

/* Single node test case */
static void
app_search_single_node_test(void){
    bpt_tree *tree;
    bpt_node *root;

    tree = bpt_init(emp_key_access,
		    emp_key_compare,
		    emp_free, 4);

    root = bpt_gen_node();
    root->keys = ll_init(tree->key_access,
			 tree->key_compare,
			 NULL);
    root->children = ll_init(tree->key_access,
			     tree->key_compare,
			     NULL);
    ll_asc_insert(root->keys, (void *) e1.id);
    ll_asc_insert(root->children, (void *) &e1);
    ll_asc_insert(root->keys, (void *) e2.id);
    ll_asc_insert(root->children, (void *) &e2);

    assert(bpt_search(tree->root, (void *) e1.id) != NULL);
    assert(bpt_search(tree->root, (void *) e2.id) != NULL);
    /* Doesn't exist, but this means we find a node to insert this id. */
    assert(bpt_search(tree->root, (void *) e5.id) != NULL);
}

int
main(int argc, char **argv){

    printf("<search bpt key test>\n");
    app_search_single_node_test();

    printf("All tests are done gracefully\n");

    return 0;
}
