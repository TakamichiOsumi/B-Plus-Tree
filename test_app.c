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
    bpt_key *key1, *key2;

    key1 = (bpt_key *) k1;
    key2 = (bpt_key *) k2;
    if ((uintptr_t) key1->key < (uintptr_t) key2->key){
	return -1;
    }else if ((uintptr_t) key1->key == (uintptr_t) key2->key){
	return 0;
    }else{
	return 1;
    }
}

static void *
emp_key_access(void *emp){
    employee_data *e = (employee_data *) emp;

    return (void *) e->id;
}

/* do nothing. */
static void
emp_free(void *emp){}

static void
emp_print(void *p){
    employee_data *emp = (employee_data *) p;

    printf("id = %d, name = %s\n",
	   (int) emp->id, emp->name);
}

int
main(int argc, char **argv){
    bpt_tree *tree;
    bpt_node *n;
    bpt_key *my_key;

    tree = bpt_init(emp_key_access,
		    emp_key_compare,
		    emp_free, 4);

    my_key = bpt_gen_key(sizeof(int), (void *) e1.id);
    bpt_insert(tree, my_key, (void *) &e1);

    my_key = bpt_gen_key(sizeof(int), (void *) e2.id);
    bpt_insert(tree, my_key, (void *) &e2);

    my_key = bpt_gen_key(sizeof(int), (void *) e3.id);
    bpt_insert(tree, my_key, (void *) &e3);

    my_key = bpt_gen_key(sizeof(int), (void *) e4.id);
    bpt_insert(tree, my_key, (void *) &e4);

    my_key = bpt_gen_key(sizeof(int), (void *) e5.id);
    bpt_insert(tree, my_key, (void *) &e5);

    /* dump the left most node */
    n = tree->left_most;
    ll_begin_iter(n->children);
    while((iter = (employee_data *) ll_get_iter_node(n->children)) != NULL){
	emp_print(iter);
    }
    ll_end_iter(n->children);

    return 0;
}
