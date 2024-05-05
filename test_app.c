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
emp_key_compare(bpt_key *key1, bpt_key *key2){
    if ((uintptr_t) key1->key < (uintptr_t) key2->key){
	return -1;
    }else if ((uintptr_t) key1->key == (uintptr_t) key2->key){
	return 0;
    }else{
	return 1;
    }
}

static void
emp_free(void *emp){}

int
main(int argc, char **argv){
    bpt_tree *tree;
    bpt_key *my_key;
    employee_data my_data;

    tree = bpt_init(emp_key_compare, emp_free, 5);

    my_key = bpt_gen_key(sizeof(int), (void *) e1.id);
    bpt_insert(tree, my_key, (void *) &e1);

    my_key = bpt_gen_key(sizeof(int), (void *) e2.id);
    bpt_insert(tree, my_key, (void *) &e2);

    return 0;
}
