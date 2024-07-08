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

    bkh_append_int(key, (void *) &write);
    bkh_int_handler(key, (void *) &read);

    printf("key = %d\n", *((int *) key));

    assert(read == write);
}

int
main(int argc, char **argv){

    printf("Perform tests for key handling\n");

    test_int_handler();

    return 0;
}
