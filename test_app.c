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

/*
static void
emp_print(void *p){
    employee_data *emp = (employee_data *) p;

    printf("id = %d, name = %s\n",
	   (int) emp->id, emp->name);
}
*/

static void
app_search_bpt_test(void){
    bpt_tree *tree;
    bpt_key *my_key;

    tree = bpt_init(emp_key_access,
		    emp_key_compare,
		    emp_free, 4);
}

int
main(int argc, char **argv){

    printf("<search bpt key test>");
    app_search_bpt_test();

    printf("All tests are done gracefully\n");

    return 0;
}
