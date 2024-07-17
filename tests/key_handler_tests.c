#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bpt_key_handler.h"
#include "../b_plus_tree.h"

static void
test_int_handler(void){
    void *key;
    int write = 10, read = -1;

    key = bkh_malloc(INT_SIZE);

    bkh_int_write(key, (void *) &write);
    bkh_int_read(key, (void *) &read);

    printf("integer key = %d\n", *((int *) key));

    assert(read == write);

    bkh_free(key);
}

static void
test_double_handler(void){
    void *key;
    double write = -100.0, read = 1.0;

    key = bkh_malloc(DOUBLE_SIZE);

    bkh_double_write(key, (void *) &write);
    bkh_double_read(key, (void *) &read);

    printf("double key = %f\n", *((double *) key));

    assert(read == write);

    bkh_free(key);
}

static void
test_bool_handler(void){
    void *key;
    bool write = true, read = false;

    key = bkh_malloc(BOOLEAN_SIZE);

    bkh_bool_write(key, (void *) &write);
    bkh_bool_read(key, (void *) &read);

    printf("bool key = %s\n", *((bool *) key) == true ? "true" : "false");

    assert(write == read);

    bkh_free(key);
}

static void
test_string_handler(){
    void *key;
    char *src = "Hello World", *dest;

    key = bkh_malloc(strlen(src) + 1);
    dest = bkh_malloc(strlen(src) + 1);

    bkh_str_write(key, (void *) src);
    bkh_str_read(key, (void *) dest);

    assert(strcmp(src, dest) == 0);
}

/*
 * Test handlers for 'double' and 'int' by creating a single
 * key sequence that consists of those two types.
 */
static void
test_combinatition_keys_handlers(){
    composite_key_store *cks;
    double dwrite = -15.0, dread = 0.0;
    int iwrite = 15, iread = 0;
    void *buf;

    cks = malloc(sizeof(composite_key_store));
    cks->keys_metadata = (bpt_key **) malloc(sizeof(bpt_key *) * 2);

    cks->full_key_size = DOUBLE_SIZE + INT_SIZE;
    cks->keys_metadata[0] = bpt_create_key_metadata(BPT_DOUBLE, 0);
    cks->keys_metadata[1] = bpt_create_key_metadata(BPT_INT, 0);

    buf = malloc(cks->full_key_size);
    memset(buf, '\0', cks->full_key_size);

    /* Write */
    buf = cks->keys_metadata[0]->key_writer(buf, (void *) &dwrite);
    buf = cks->keys_metadata[1]->key_writer(buf, (void *) &iwrite);

    /* Make the buffer point to the original position */
    buf -= cks->full_key_size;

    /* Read */
    buf = cks->keys_metadata[0]->key_reader(buf, (void *) &dread);
    buf = cks->keys_metadata[1]->key_reader(buf, (void *) &iread);

    assert(dread == dwrite);
    assert(iread == iwrite);

    /*
     * Clean up. Never forget to rewind the pointer to the
     * original buffer position. Otherwise, free() fails.
     */
    buf -= cks->full_key_size;
    free(buf);

    free(cks->keys_metadata);
    free(cks);
}

/*
 * Test handlers for 'string', 'bool' and 'int' by creating a single
 * key sequence that consists of those three types.
 *
 * Suppose that the maximum size of the string is 20.
 */
static void
test_combinatition_keys_handlers_v2(){
    composite_key_store *cks;
    char *swrite = "Hello World", *sread;
    bool bwrite = true, bread = false;
    int iwrite = -100, iread = 0;
    void *buf;

    cks = malloc(sizeof(composite_key_store));
    cks->keys_metadata = (bpt_key **) malloc(sizeof(bpt_key *) * 3);

    cks->full_key_size = 20 + BOOLEAN_SIZE + INT_SIZE;
    cks->keys_metadata[0] = bpt_create_key_metadata(BPT_STRING, 20);
    cks->keys_metadata[1] = bpt_create_key_metadata(BPT_BOOLEAN, 0);
    cks->keys_metadata[2] = bpt_create_key_metadata(BPT_INT, 0);

    buf = malloc(cks->full_key_size);
    memset(buf, '\0', cks->full_key_size);

    /* Write */
    buf = cks->keys_metadata[0]->key_writer(buf, (void *) swrite);
    /* Skip the left unused space for string variable */
    buf = buf + (cks->keys_metadata[0]->key_size - (strlen(swrite) + 1));
    buf = cks->keys_metadata[1]->key_writer(buf, (void *) &bwrite);
    buf = cks->keys_metadata[2]->key_writer(buf, (void *) &iwrite);

    /* Make the buffer point to the original position */
    buf -= cks->full_key_size;

    /* Read */
    sread = malloc(sizeof(char) * cks->keys_metadata[0]->key_size);
    buf = cks->keys_metadata[0]->key_reader(buf, (void *) sread);
    /* Skip the left unused space for string variable */
    buf = buf + (cks->keys_metadata[0]->key_size - (strlen(sread) + 1));
    buf = cks->keys_metadata[1]->key_reader(buf, (void *) &bread);
    buf = cks->keys_metadata[2]->key_reader(buf, (void *) &iread);

    assert(strncmp(swrite, sread, strlen(swrite)) == 0);
    assert(bwrite == bread);
    assert(iwrite == iread);

    /* Clean up */
    buf -= cks->full_key_size;
    free(buf);

    free(cks->keys_metadata);
    free(cks);
}

static void
test_basic_key_handlers(void){
    printf("> Test integer handler\n");
    test_int_handler();

    printf("> Test double handler\n");
    test_double_handler();

    printf("> Test bool handler\n");
    test_bool_handler();

    printf("> Test string handler\n");
    test_string_handler();
}

static void
test_advanced_key_handlers(void){
    printf("> Test keys combination (double & integer)\n");
    test_combinatition_keys_handlers();

    printf("> Test keys combination (string & bool & integer)\n");
    test_combinatition_keys_handlers_v2();
}

int
main(int argc, char **argv){

    printf("> Perform tests for key handling\n");

    test_basic_key_handlers();

    printf("> Perform combination tests of more than two types\n");

    test_advanced_key_handlers();

    return 0;
}
