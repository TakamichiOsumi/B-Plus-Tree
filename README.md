# B+ Tree (Variant)

Making submodule for my other repository. This is a work in progress and written for self-education.

# How to build and test

```
git clone https://github.com/TakamichiOsumi/B-Plus-Tree.git
cd B-Plus-Tree
git submodule init
git submodule update
make
make test
```

## Notes

The difference from the normal B+ Tree is that nodes at the same depth are connected as doubly linked list.
