#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../bpt_key_handler.h"
#include "../b_plus_tree.h"

static void
test_int_handler(void){
    void *key;
    int write = 10, read = -1;

    key = bkh_malloc(INT_SIZE);

    bkh_int_write(key, (void *) &write);
    bkh_int_read(key, (void *) &read);

    printf("key = %d\n", *((int *) key));
    assert(read == write);

    bkh_free(key);
}

static void
test_double_handler(void){
    void *key;
    double write = 100.0, read = 1.0;

    key = bkh_malloc(DOUBLE_SIZE);

    bkh_double_write(key, (void *) &write);
    bkh_double_read(key, (void *) &read);

    printf("Key = %f\n", *((double *) key));

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

    printf("Key = %s\n", *((bool *) key) == true ? "true" : "false");

    assert(write == read);

    bkh_free(key);
}

static void
test_basic_key_handlers(void){
    printf("> Test integer handler\n");
    test_int_handler();

    printf("> Test double handler\n");
    test_double_handler();

    printf("> Test bool handler\n");
    test_bool_handler();
}

int
main(int argc, char **argv){

    printf("> Perform tests for key handling\n");

    test_basic_key_handlers();

    return 0;
}
