# B+ Tree (Variant)

Making a submodule for my other repository for self-education.

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

This is written to understand the basic flow of B+ Tree algorithms. In order to focus on the logic of the algorithms, some operations that manipulate keys and children are encapsulated by linked list library.

The difference from the normal B+ Tree is that nodes at the same depth are connected as doubly linked list.
