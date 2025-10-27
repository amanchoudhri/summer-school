b-tree
---

A minimal in-memory B+-tree. Maps 128-bit (`long long`) primary keys to 128-bit
row indices. I may extend this to support an on-disk index.

Excuse the code, I wanted an excuse to brush up on C.

Implemented:
 - `insert`
 - `find`

In brief: A B+-tree of order `m` is a tree data structure where each node may
have up to `m` children. It is height-balanced, meaning all leaf nodes are at
the same depth. This height balance condition is important because B+-trees are
usually stored on-disk—keeping nodes wide and the tree shallow, then, means
fewer disk I/O operations and better performance.
