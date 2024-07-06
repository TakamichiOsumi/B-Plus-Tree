# B+ Tree (Variant)

Submodule for my other repository to enhance my self-education.

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

The difference from the normal B+ Tree is that nodes at the same depth are connected as doubly linked list.
