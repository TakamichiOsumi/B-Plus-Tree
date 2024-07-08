#ifndef __BPT_KEY_HANDLER__
#define __BPT_KEY_HANDLER__

#include <stdint.h>

#include "Linked-List/linked_list.h"

/*
 * Define key sizes of fixed-size variables.
 *
 * String type is calculated by user input.
 */
#define INT_SIZE (sizeof(INT))
#define DOUBLE_SIZE (sizeof(DOUBLE))
#define BOOLEAN_SIZE (sizeof(bool))

/*
 * Define all key types to support
 */
typedef enum key_type {
    INT,
    DOUBLE,
    STRING,
    BOOLEAN,
} key_type;

/*
 * Define one key for each key type.
 */
typedef struct bpt_key {
    /* Key data type */
    key_type type;

    /*
     * Usually fixed size.
     *
     * But, length is variable when the type is STRING.
     */
    uint16_t key_size;

    void *(*key_handler)(void *);
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
    int full_key_size;

    /*
     * Lists of bpt_key
     */
    linked_list *key_attributes;

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


#endif
