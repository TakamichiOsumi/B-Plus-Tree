#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../b_plus_tree.h"

#define NAME_LEN 16
typedef struct student {
    int class_id;
    int student_no;
    int class_no;
    char name[NAME_LEN];
} student;

static void *
student_key_access(void *data){
    return data;
}

static int
student_key_compare(void *key1, void *key2){
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
student_free(void *data){}

static void
records_bpt_test(){
    bpt_tree *tree;
    uintptr_t i, records_num = 1024;
    student *std, *std_ary;

    tree = bpt_init(student_key_access,
		    student_key_compare,
		    student_free,
		    3);

    std_ary = (student *) malloc(sizeof(student) * records_num);

    /* Create and insert a new entry */
    for (i = 1; i <= records_num; i++){
	std_ary[i].student_no = i;
	bpt_insert(tree, (void *) i, &std_ary[i]);
    }

    /* All search should succeed */
    for (i = 1; i <= records_num; i++){
	assert(bpt_search(tree, (void *) i, NULL, (void *) &std) == true);
	printf("debug : searched employee with id = %d\n", std->student_no);
	assert(std->student_no == i);
    }

    /* All delete should succeed */
    for (i = 1; i <= records_num; i++){
	assert(bpt_delete(tree, (void *) i, (void **) &std) == true);
	printf("debug : deleted student with id = %d\n", std->student_no);
	assert(std->student_no == i);
    }

    /* Clean up resources */
    free(std_ary);

    bpt_destroy(tree);
}

int
main(int argc, char **argv){

    printf("Perform the tests for record search, insert and delete...\n");

    records_bpt_test();

    printf("All tests are done gracefully\n");

    return 0;
}
