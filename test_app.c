#include <stdlib.h>
#include <stdbool.h>

#include "b_plus_tree.h"

/* dummy data */
typedef struct app_data {
    int id;
    char *name;
    char *description;
} app_data;

int
main(int argc, char **argv){
    bpt_tree *tree;
    bpt_key *my_key;
    app_data my_data;

    tree = bpt_init(NULL, NULL, 5);

    my_key = bpt_gen_key(sizeof(int), (void *) 1);
    my_data.id = 1;
    my_data.name = "foo";
    my_data.description = "bar";
    bpt_insert(tree, my_key, (void *) &my_data);

    return 0;
}
