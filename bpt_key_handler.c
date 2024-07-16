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
 * Return dynamically allocated bpt_key * data for 'type'.
 *
 * The 'str_size' is valid only for BPT_STRING. It's ignored
 * in other types.
 */
bpt_key *
bpt_create_key_metadata(key_type type, int str_size){
    bpt_key *key_metadata;

    if (type < BPT_INT || BPT_BOOLEAN < type){
	fprintf(stderr, "detected invalid key type.\n");
	return NULL;
    }

    if (type == BPT_STRING && str_size <= 1){
	fprintf(stderr, "detected invalid key size for string type\n");
	return NULL;
    }

    key_metadata = bkh_malloc(sizeof(bpt_key));
    switch(type){
	case BPT_INT:
	    key_metadata->type = BPT_INT;
	    key_metadata->key_size = INT_SIZE;
	    key_metadata->key_writer = bkh_int_write;
	    key_metadata->key_reader = bkh_int_read;
	    break;
	case BPT_DOUBLE:
	    key_metadata->type = BPT_DOUBLE;
	    key_metadata->key_size = DOUBLE_SIZE;
	    key_metadata->key_writer = bkh_double_write;
	    key_metadata->key_reader = bkh_double_read;
	    break;
	case BPT_STRING:
	    key_metadata->type = BPT_STRING;
	    key_metadata->key_size = str_size;
	    key_metadata->key_writer = bkh_str_write;
	    key_metadata->key_reader = bkh_str_read;
	    break;
	case BPT_BOOLEAN:
	    key_metadata->type = BPT_BOOLEAN;
	    key_metadata->key_size = BOOLEAN_SIZE;
	    key_metadata->key_writer = bkh_bool_write;
	    key_metadata->key_reader = bkh_bool_read;
	    break;
	default:
	    assert(0);
	    break;
    }

    return key_metadata;
}

void
bpt_free_key_metadata(bpt_key *key_metadata){
    if (key_metadata != NULL)
	free(key_metadata);
}
