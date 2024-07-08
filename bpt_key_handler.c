#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "b_plus_tree.h"

void *
bkh_malloc(size_t size){
    void *p;

    if ((p = malloc(size)) == NULL){
	perror("malloc");
	exit(-1);
    }

    return p;
}

void
bkh_free(void *p){
    if (p != NULL)
	free(p);
}

/*
 * Write an integer value from 'int_ptr' to 'key_sequence'
 */
void *
bkh_int_write(void *key_sequence, void *int_ptr){
    int *p = key_sequence;

    *p = *((int *) int_ptr);
    key_sequence += INT_SIZE;

    return key_sequence;
}

/*
 * Read an integer value from 'key_sequence' to 'int_ptr'
 */
void *
bkh_int_read(void *key_sequence, void *int_ptr){
    int *ip = (int *) int_ptr;

    *ip = *((int *) key_sequence);
    key_sequence += INT_SIZE;

    return key_sequence;
}

/*
 * Write a double value from 'double_ptr' to 'key_sequence'
 */
void *
bkh_double_write(void *key_sequence, void *double_ptr){
    double *dp = (double *) key_sequence;

    *dp = *((double *) double_ptr);
    key_sequence += DOUBLE_SIZE;

    return key_sequence;
}

/*
 * Read a double value from 'key_sequence' to 'double_ptr'
 */
void *
bkh_double_read(void *key_sequence, void *double_ptr){
    double *dp = (double *) double_ptr;

    *dp = *((double *) key_sequence);
    key_sequence += DOUBLE_SIZE;

    return key_sequence;
}

/*
 * Write a bool value from 'bool_ptr' to 'key_sequence'
 */
void *
bkh_bool_write(void *key_sequence, void *bool_ptr){
    bool *bp = (bool *) key_sequence;

    *bp = *((bool *) bool_ptr);
    key_sequence += BOOLEAN_SIZE;

    return key_sequence;
}

/*
 * Read a bool value from 'key_sequence' from 'bool_ptr'
 */
void *
bkh_bool_read(void *key_sequence, void *bool_ptr){
    bool *bp = (bool *) bool_ptr;

    *bp = *((bool *) key_sequence);
    key_sequence += BOOLEAN_SIZE;

    return key_sequence;
}
