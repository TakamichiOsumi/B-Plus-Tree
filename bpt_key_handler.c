#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "b_plus_tree.h"
#include "bpt_key_handler.h"

#include "Linked-List/linked_list.h"

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

/*
 * Write a string value from 'str_ptr' to 'key_sequence'.
 *
 * The caller must ensure that the str_ptr has ended with
 * null-termination, since this function depends on strlen().
 */
void *
bkh_str_write(void *key_sequence, void *str_ptr){
    char *cp = (char *) key_sequence,
	*s = (char *) str_ptr;
    int i, str_len = strlen(s);

    for (i = 0; i < str_len; i++){
	*cp = *s;
	cp++;
	s++;
    }
    *cp = '\0';

    /* printf("The written data : '%s'\n", (char *) key_sequence); */

    key_sequence += (str_len + 1);

    return key_sequence;
}

/*
 * Read a string value from 'key_sequence' to 'str_ptr'
 *
 * The caller must ensure that the str_ptr has enough
 * space to copy the string in key_sequence.
 */
void *
bkh_str_read(void *key_sequence, void *str_ptr){
    char *cp = (char *) key_sequence,
	*s = (char *) str_ptr;
    int i, str_len = strlen(cp);

    for (i = 0; i < str_len; i++){
	*s = *cp;
	s++;
	cp++;
    }
    *s = '\0';

    key_sequence += (str_len + 1);

    return key_sequence;
}

/*
 * Genereate composite keys metadata from primary keys
 * components.
 */
composite_key_store *
bpt_generate_composite_keys(linked_list *primary_keys){
    composite_key_store *cks;
    uintptr_t type;
    int i;
    /* int i, full_key_length = 0; */

    if (primary_keys == NULL || primary_keys->head == NULL)
	return NULL;

    cks = bkh_malloc(sizeof(composite_key_store));
    cks->key_attributes = bkh_malloc(sizeof(bpt_key) * ll_get_length(primary_keys));

    ll_begin_iter(primary_keys);
    for (i = 0; i < ll_get_length(primary_keys); i++){
	type = (uintptr_t) ll_get_iter_data(primary_keys);
	switch(type){
	    case BPT_INT:
		break;
	    case BPT_DOUBLE:
		break;
	    case BPT_STRING:
		break;
	    case BPT_BOOLEAN:
		break;
	    default:
		break;
	}
    }
    ll_end_iter(primary_keys);

    return cks;
}

void
bpt_destroy_composite_keys(composite_key_store *cks){}
