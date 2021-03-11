/*
 * binary_tree.c
 *
*/


#include<stdlib.h>
#include<stdio.h>
#include"binary_tree.h"


void insert(node ** tree, struct in_addr val)
{
    node *temp = NULL;
    if(!(*tree))
    {
        temp = (node *)malloc(sizeof(node));
		temp->transactions = 1;
        temp->left = temp->right = NULL;
        temp->data = val;
        *tree = temp;
        return;
    }

    if(ntohl(val.s_addr) < ntohl((*tree)->data.s_addr))
    {
        insert(&(*tree)->left, val);
    }
    else if(ntohl(val.s_addr) > ntohl((*tree)->data.s_addr))
    {
        insert(&(*tree)->right, val);
    }
	
}
void insertNode(node ** tree, node * temp)
{
    node *tmp = temp;
    if(!(*tree))
    {
        (*tree) = tmp;
    }
    if(ntohl((*tmp).data.s_addr) < ntohl((*tree)->data.s_addr))
    {
        insertNode(&(*tree)->left, tmp);
    }
    else if(ntohl((*tmp).data.s_addr) < ntohl((*tree)->data.s_addr))
    {
        insertNode(&(*tree)->right, tmp);
    }
	
}
void deltree(node * tree)
{
    if (tree)
    {
        deltree(tree->left);
        deltree(tree->right);
        free(tree);
    }
}

node* search(node ** tree, struct in_addr val)
{
    if(!(*tree))
    {
        return NULL;
    }

    if(ntohl(val.s_addr) < ntohl((*tree)->data.s_addr))
    {
        search(&((*tree)->left), val);
    }
    else if(ntohl(val.s_addr) > ntohl((*tree)->data.s_addr))
    {
        search(&((*tree)->right), val);
    }
    else if(ntohl(val.s_addr) == ntohl((*tree)->data.s_addr))
    {
        return *tree;
    }
}