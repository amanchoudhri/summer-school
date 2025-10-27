b-tree
---

A minimal in-memory B+-tree. Maps 128-bit (`long long`) primary keys to 128-bit row indices. I may extend this to support an on-disk index.

Excuse the code, I wanted an excuse to brush up on C.

Implemented:
 - `insert`
 - `find`
