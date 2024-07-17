#ifndef __BPT_KEY_HANDLER__
#define __BPT_KEY_HANDLER__

#include <stdint.h>

/*
 * Define all key types to support.
 */
typedef enum key_type {
    BPT_INT,
    BPT_DOUBLE,
    BPT_STRING,
    BPT_BOOLEAN,
} key_type;

/*
 * Define key sizes of fixed-size variables.
 *
 * String type is calculated by user input.
 */
#define INT_SIZE (sizeof(int))
#define DOUBLE_SIZE (sizeof(double))
#define BOOLEAN_SIZE (sizeof(bool))

/*
 * Define one key for each key type.
 */
typedef struct bpt_key {

    /* Key data type */
    key_type type;

    /*
     * Fixed size except for STRING type.
     */
    uintptr_t key_size;

    /*
     * Type specific callbacks to read and write data.
     */
    void *(*key_writer)(void *, void *);

    void *(*key_reader)(void *, void *);

} bpt_key;

/*
 * Build one unique key from multiple keys.
 *
 * Assign one composite_key_store to one b+ tree.
 */
typedef struct composite_key_store {

    /*
     * Length of one sequence of unique key
     */
    uintptr_t full_key_size;

    /*
     * Array of 'bpt_key *'
     */
    bpt_key **keys_metadata;

} composite_key_store;

void *bkh_malloc(size_t size);
void bkh_free(void *p);

void *bkh_int_write(void *key_sequence, void *int_ptr);
void *bkh_int_read(void *key_sequence, void *int_ptr);
void *bkh_double_write(void *key_sequence, void *double_ptr);
void *bkh_double_read(void *key_sequence, void *double_ptr);
void *bkh_bool_write(void *key_sequence, void *bool_ptr);
void *bkh_bool_read(void *key_sequence, void *bool_ptr);
void *bkh_str_write(void *key_sequence, void *str_ptr);
void *bkh_str_read(void *key_sequence, void *str_ptr);

bpt_key *bpt_create_key_metadata(key_type type, int str_size);
void bpt_free_key_metadata(bpt_key *key_metadata);

#endif
