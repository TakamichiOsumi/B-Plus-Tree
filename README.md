# B+ Tree (Variant)

In-memory B+ Tree submodule for my other repository to enhance my self-education.

The difference from the normal B+ Tree is that nodes at the same depth are connected as doubly linked list and every node has pointer to its parent.

## `libbplustree` library functions

| Function | Description |
| ---- | ---- |
| bpt_init | Create a new bpt_tree * object |
| bpt_insert | Insert one pair of key and record into bpt_tree * object  |
| bpt_search | Search a key from bpt_tree * object |
| bpt_delete | Delete a key and record from bpt_tree * object |
| bpt_destory | Destroy all registered keys and records from bpt_tree * object |

See the explicit function prototypes in `b_plus_tree.h`.

## How to build and test

```
% git clone https://github.com/TakamichiOsumi/B-Plus-Tree.git
% cd B-Plus-Tree
% git submodule init
% git submodule update
% make
% make test
```

## Notes

This is written to understand the basic flows of B+ Tree algorithms. In order to focus on their logics, some operations that manipulate keys and children are encapsulated by linked list library.
