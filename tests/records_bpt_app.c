#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../b_plus_tree.h"

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
records_test_bpt_search(){
    bpt_tree *tree;

    tree = bpt_init(employee_key_access,
		    employee_key_compare,
		    employee_free,
		    employee_key_access_from_employee,
		    employee_key_compare,
		    employee_free,
		    3);
}

static void
records_test_bpt_insert(){
}

static void
records_test_bpt_remove(){
}

int
main(int argc, char **argv){

    printf("Perform the tests for record search, insert and delete...\n");

    records_test_bpt_search();
    records_test_bpt_insert();
    records_test_bpt_remove();

    printf("All tests are done gracefully\n");

    return 0;
}
