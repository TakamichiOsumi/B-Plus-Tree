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
    key_type type;
    uint16_t key_size;
    void *key;
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
     * Lists of key_element
     */
    linked_list *key_attributes;

} composite_key_store;

void *bkh_malloc(size_t size);
void bkh_free(void *p);
void *bkh_append_int(void *key_sequence, void *int_ptr);
void *bkh_int_handler(void *key_sequence, void *int_ptr);

#endif
