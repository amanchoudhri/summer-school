#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ORDER 5

struct Node {
    int n_keys;
    // invariant is ORDER - 1 keys, we add space for one more
    // for convenience during splits
    long long keys[ORDER];

    bool is_leaf;

    // if internal node
    struct Node *children[ORDER + 1]; 

    // if leaf
    struct Node *next;
    long long row_ids[ORDER];
};

struct Node* make_node(bool is_leaf) {
    struct Node* node = malloc(sizeof(struct Node));
    node->n_keys = 0;
    for (int i = 0; i < ORDER; i++) {
        node->keys[i] = -1;
    }
    node->is_leaf = is_leaf;

    for (int i = 0; i <= ORDER; i++) {
        node->children[i] = NULL;
    }

    node->next = NULL;
    for (int i = 0; i < ORDER; i++) {
        node->row_ids[i] = -1;
    }

    return node;
}

void print_array(long long arr[], int n, char* name){
    printf("%s: [", name);
    for (int i = 0; i < n; i++) {
        if (i > 0) printf(", ");
        printf("%lld", arr[i]);
    }
    printf("]\n");
}

void indent(int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
}

void print_tree(struct Node *node, char* name, int depth){
    indent(depth);
    printf("Node %s\n", name);
    indent(depth);
    printf("is_leaf: %d\n", node->is_leaf);
    indent(depth);
    printf("n_keys: %d\n", node->n_keys);
    indent(depth);
    print_array(node->keys, node->n_keys, "keys");

    if (!node->is_leaf) {
        for (int i = 0; i <= node->n_keys; i++){
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "%s->children[%d]", name, i);
            print_tree(node->children[i], buffer, depth + 1);
        }
    }
    else {
        indent(depth);
        printf("%s: %p\n", name, node);
        indent(depth);
        printf("%s->next: %p\n", name, node->next);
        indent(depth);
        print_array(node->row_ids, node->n_keys, "row_ids");
    }
    indent(depth);
    printf("---\n");
}

void free_tree(struct Node *node){
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n_keys; i++) free_tree(node->children[i]);
    }
    free(node);
}

void long_array_insert(long long arr[], int n, long long value, int position){
    long long tmp;
    for (int i = position; i < n; i++){
        tmp = arr[i];
        arr[i] = value;
        value = tmp;
    }
}
void node_ptr_array_insert(struct Node *arr[], int n, struct Node *value, int position){
    struct Node *tmp;
    for (int i = position; i < n; i++){
        tmp = arr[i];
        arr[i] = value;
        value = tmp;
    }
}

void check_leaf_links(struct Node* root){
    struct Node *cursor = root;
    while (!cursor->is_leaf) {
        cursor = cursor->children[0];
    }
    printf("Checking leaf links -----\n\n");
    while (cursor != NULL) {
        printf("cursor: %p\n", cursor);
        indent(1);
        print_array(cursor->keys, cursor->n_keys, "keys");
        indent(1);
        printf("cursor->next: %p\n", cursor->next);
        cursor = cursor->next;
    }
}

struct Node* split_leaf(struct Node *node) {
    /* 
     * Split `node` into left and right nodes, returning a pointer to the new parent.
     */
    struct Node *root = make_node(false);
    struct Node *right = make_node(true);

    const int n_left = (int) ORDER / 2;

    // move the right half from `node` into `right`
    for (int j = 0; n_left + j < node->n_keys; j++){
        right->keys[j] = node->keys[n_left + j];
        right->row_ids[j] = node->row_ids[n_left + j];
    }

    right->n_keys = node->n_keys - n_left;
    right->is_leaf = true;

    node->n_keys = n_left;

    root->keys[0] = right->keys[0];
    root->n_keys = 1;

    root->children[0] = node;
    root->children[1] = right;

    right->next = node->next;
    node->next = right;

    return root;
}

struct Node* split(struct Node *node) {
    /* 
     * Split `node` into left and right nodes, returning a pointer to the new parent.
     */
    struct Node *root = make_node(false);
    struct Node *right = make_node(false);

    const int n_left = (int) ORDER / 2;

    // right now there's ORDER keys and ORDER + 1 children
    // move [n_left + 1, ..., ORDER] keys into `right`
    for (int j = 0; n_left + j < ORDER - 1; j++){
        right->keys[j] = node->keys[n_left + j + 1];
    }
    // move [n_left + 1, ..., ORDER + 1] children into `right`
    for (int j = 0; n_left + j < ORDER; j++){
        right->children[j] = node->children[n_left + j + 1];
    }
    right->n_keys = node->n_keys - n_left - 1;
    node->n_keys = n_left;

    root->keys[0] = node->keys[n_left];
    root->n_keys = 1;

    root->children[0] = node;
    root->children[1] = right;

    return root;
}

struct Node* insert(struct Node *root, long long key, long long row_id){
    /*
     * Insert (key, row_id) into the B-tree with root `root`. 
     *
     * Insertion maintains:
     *  1. every leaf is at the same height.
     *  2. every key in `root->children[i]` is >= `keys[i - 1]` and < `keys[i]`.
     */

    // identify where we should insert
    int i = 0;
    for (i = 0; i < root->n_keys; i++){
        if (key < root->keys[i]) break;
    }

    if (!root->is_leaf) {
        // identify which is the next child we should try inserting into
        struct Node* node = insert(root->children[i], key, row_id);

        // if _insert returns us a DIFFERENT node than root->children[i]
        // we need to add its separator and the right child into root.
        if (node != root->children[i]) {
            struct Node *right = node->children[1];
            key = node->keys[0];

            free(node);

            // `key` is by definition larger than everything now in
            // `root->children[i]`, so we should put it immediately after, at `keys[i]`
            long_array_insert(root->keys, ORDER, key, i);
            // and `right` is >= `key` but <= keys[i+1], so we should put it
            // at child position i+1
            node_ptr_array_insert(root->children, ORDER + 1, right, i + 1);
            root->n_keys++;
            if (root->n_keys <= ORDER - 1) {
                return root;
            }
            else {
                struct Node* new_root = split(root);
                return new_root;
            }
        }
        return root;
    }
    // if `root` IS a leaf, we split if it's full or directly insert if there's room

    // implementation-wise, we have one extra space in `root->keys`, so we
    // always insert, then split if necessary
    long_array_insert(root->keys, ORDER, key, i);
    long_array_insert(root->row_ids, ORDER, row_id, i);
    root->n_keys++;

    if (root->n_keys <= ORDER - 1) {
        return root;
    }
    else {
        struct Node* new_root = split_leaf(root);
        return new_root;
    }
};

long long find(struct Node *root, long long key){
    /*
     * Given a primary key, return the corresponding row_id (-1 if key not found).
     */
    if (root->is_leaf) {
        for (int i = 0; i < root->n_keys; i++){
            if (key == root->keys[i]) return root->row_ids[i];
        }
        return -1;
    }

    int i;
    for (i = 0; i < root->n_keys; i++){
        if (key < root->keys[i]) break;
    }
    return find(root->children[i], key);
}

int main() {
    struct Node *root = make_node(true);

    printf("Inserting 2, 3, 5, 4.\n");
    root = insert(root, 2, 0);
    root = insert(root, 3, 1);
    root = insert(root, 5, 2);
    root = insert(root, 4, 3);

    print_tree(root, "root", 0);

    printf("Inserting 7.\n\n");
    root = insert(root, 7, 4);

    print_tree(root, "root", 0);

    printf("Inserting 1.\n\n");
    root = insert(root, 1, 5);
    print_tree(root, "root", 0);

    printf("Inserting 6.\n\n");
    root = insert(root, 6, 6);
    print_tree(root, "root", 0);

    printf("Inserting 8.\n\n");
    root = insert(root, 8, 9);
    print_tree(root, "root", 0);

    printf("Inserting 1000.\n\n");
    root = insert(root, 1000, 10);
    print_tree(root, "root", 0);

    printf("Inserting 20.\n\n");
    root = insert(root, 20, 11);
    print_tree(root, "root", 0);

    printf("Inserting 25, 30.\n\n");
    root = insert(root, 25, 12);
    root = insert(root, 30, 13);
    print_tree(root, "root", 0);

    printf("Inserting 21, 22.\n\n");
    root = insert(root, 21, 14);
    root = insert(root, 22, 15);
    print_tree(root, "root", 0);

    printf("Inserting 23.\n\n");
    root = insert(root, 23, 16);
    print_tree(root, "root", 0);

    printf("Inserting 9.\n\n");
    root = insert(root, 9, 17);
    print_tree(root, "root", 0);

    printf("Inserting 31, 32.\n\n");
    root = insert(root, 31, 18);
    root = insert(root, 32, 19);
    print_tree(root, "root", 0);

    printf("Inserting 33, 34.\n\n");
    root = insert(root, 33, 20);
    root = insert(root, 34, 21);
    print_tree(root, "root", 0);

    printf("Inserting 35, 36.\n\n");
    root = insert(root, 35, 22);
    root = insert(root, 36, 23);
    print_tree(root, "root", 0);

    check_leaf_links(root);

    printf("find(9): %lld. expected: 17\n", find(root, 9));
    printf("find(770): %lld. expected: -1\n", find(root, 770));

    free_tree(root);

    return 0;
}
