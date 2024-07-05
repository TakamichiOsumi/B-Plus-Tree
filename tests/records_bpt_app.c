#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../b_plus_tree.h"

#define NAME_LEN 16
typedef struct employee {
    uintptr_t id;
    char name[NAME_LEN];
} employee;

static void *
employee_key_access(void *data){
    return data;
}

static int
employee_key_compare(void *key1, void *key2){
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

static void
employee_free(void *data){}

static void *
employee_key_access_from_employee(void *data){
    return data;
}

static void
records_bpt_test(){
    bpt_tree *tree;
    uintptr_t i, records_num = 64;
    employee *emp, *emp_ary;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    3);

    emp_ary = (employee *) malloc(sizeof(employee) * records_num);

    /* Create and insert a new entry */
    for (i = 1; i <= records_num; i++){
	emp_ary[i].id = i;
	memset(emp_ary[i].name, '\0', NAME_LEN);
	snprintf(emp_ary[i].name, NAME_LEN, "%lu", i);
	bpt_insert(tree, (void *) i, &emp_ary[i]);
    }

    /* All search should succeed */
    for (i = 1; i <= records_num; i++){
	assert(bpt_search(tree, (void *) i, NULL, (void *) &emp) == true);
	printf("debug : searched employee with id = %lu\n", emp->id);
	assert(emp->id == i);
    }

    /* All delete should succeed */
    for (i = 1; i <= records_num; i++){
	assert(bpt_delete(tree, (void *) i, (void **) &emp) == true);
	printf("debug : deleted employee with id = %lu\n", i);
	assert(emp->id == i);
    }

    /* Clean up resources */
    free(emp_ary);

    bpt_destroy(tree);
}

int
main(int argc, char **argv){

    printf("Perform the tests for record search, insert and delete...\n");

    records_bpt_test();

    printf("All tests are done gracefully\n");

    return 0;
}
