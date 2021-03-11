/*
 * binary_tree.h
 *
*/

#ifndef BINARY_TREE_HEADER
#define BINARY_TREE_HEADER

#include <bits/types.h>
#include<arpa/inet.h> 


//typedef __uint32_t uint32_t;

struct bin_tree {
    struct in_addr data;
    uint32_t transactions;
    struct bin_tree * right, * left;
};

typedef struct bin_tree node;

node* search(node **, struct in_addr);
void insert(node **, struct in_addr);
void deltree(node *);
struct node* new_node(struct in_addr);
void insertNode(node ** tree, node * temp);

#endif