#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "b_plus_tree.h"

static void*
bpt_malloc(size_t size){
    void *p;

    if ((p = malloc(size)) == NULL){
	perror("malloc");
	exit(-1);
    }

    return p;
}

static bool
bpt_found_insert_node(bpt_tree *bpt, bpt_node *n, bpt_key *new_key){
    bpt_key *existing_key;

    assert(bpt->root != NULL);

    /* There is no data in the tree */
    if (ll_get_length(bpt->left_most->keys) == 0){
	return true;
    }

    /*
     * Search the first key value where the new key value can exceed.
     * That is the minimum value, larger than the key value in the
     * list.
     */
    ll_begin_iter(n->keys);
    while((existing_key = (bpt_key *) ll_get_iter_node(n->keys)) != NULL){
	if (bpt->key_compare(existing_key, new_key) == 1){
	    return true;
	}
    }
    ll_end_iter(n->keys);

    return false;
}

static bool
bpt_node_available(bpt_tree *tree, bpt_node *node){
    int max_key_num = tree->m - 1;

    if (node->n_keys < max_key_num)
	return true;
    else
	return false;
}

bpt_key*
bpt_gen_key(uint16_t key_size, void *key){
    bpt_key *new_key;

    new_key = (bpt_key *) bpt_malloc(sizeof(bpt_key));
    new_key->key_size = key_size;
    new_key->key = key;

    return new_key;
}

static void *
bpt_access_key_in_bpt_key(void *my_bpt_key){
    bpt_key *k = (bpt_key *) my_bpt_key;
    return k->key;
}

static void *
bpt_access_key_in_children(void *child){
    return NULL;
}

static bpt_node *
bpt_gen_node(uint16_t m,
	     void *(*key_access_cb)(void *data),
	     int (*key_compare_cb)(void *key1,
				   void *key2),
	     void (*free_cb)(void *data)){
    bpt_node *node;

    node = (bpt_node *) bpt_malloc(sizeof(bpt_node));
    node->is_root = node->is_leaf = NULL;
    node->n_keys = 0;
    node->keys = ll_init(NULL, NULL, NULL);
    node->children = ll_init(NULL, NULL, NULL);
    node->parent = node->next = NULL;

    return node;
}

bpt_tree *
bpt_init(bpt_key_access_cb key_access,
	 bpt_key_compare_cb key_compare,
	 bpt_free_cb free, uint16_t m){
    bpt_tree *tree;

    assert(m >= 3);

    tree = (bpt_tree *) bpt_malloc(sizeof(bpt_tree));

    /* Set up the initial empty node */
    tree->root = tree->left_most = bpt_gen_node(m,
						key_access,
						key_compare,
						free);
    tree->root->is_root = tree->root->is_leaf = true;

    tree->key_access = key_access;
    tree->key_compare = key_compare;
    tree->free = free;
    tree->m = m;

    return tree;
}

static void
bpt_insert_to_node(bpt_tree *t, bpt_node *n,
		   bpt_key *new_key, void *new_data){

    if (bpt_node_available(t, n)){
	if (n->is_leaf){
	    printf("insert the node to this node\n");
	    ll_asc_insert(n->keys, new_key);
	    ll_asc_insert(n->children, new_data);
	}else{
	    ;
	}
    }else{
	printf("this node is full. split the list\n");
    }
}

void
bpt_insert(bpt_tree *bpt, bpt_key *new_key, void *new_data){
    bpt_node *prev, *n = bpt->left_most;
    bool found_node = false;

    prev = n;
    do {
	if ((bpt_found_insert_node(bpt, n, new_key))){
	    found_node = true;
	    break;
	}
	prev = n;
    } while((n = n->next) != NULL);

    if (found_node){
	bpt_insert_to_node(bpt,
			   n, new_key, new_data);
    }else if (n == NULL){
	/*
	 * Couldn't find a larger key than new key,
	 */
	if (bpt_node_available(bpt, prev)){
	    /* But, we have some space in the last node */
	    ll_tail_insert(prev->keys, new_key);
	    ll_tail_insert(prev->children, new_data);
	    printf("inserted to the current node\n");
	}else{
	    /*
	     * In the last node, there is no space. Create
	     * another node and insert there.
	     */
	    printf("created a new node and inserted it there\n");
	    ll_tail_insert(prev->keys, new_key);
	    ll_tail_insert(prev->children, new_data);
	}
    }
}

void
bpt_search(bpt_tree *bpt, bpt_key *key){}

void
bpt_delete(bpt_tree *bpt, bpt_key *key){}

void
bpt_destroy(bpt_tree *bpt){}
