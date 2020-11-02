// SP Exercise 1A
// Fabrizio Catinella
// 2322021C
// This is my own work as defined in the Academic Ethics agreement I have signed.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tldlist.h"
#include "date.h"


// Definitions for each structure
struct tldlist {
    struct date *begin;
    struct date *end;
    struct tldnode *root;
};

struct tldnode {
    struct tldnode *right;
    struct tldnode *left;
    struct tldnode *parent;
    char *tld;
    long count;
    long height;
    long balance;
};

struct tlditerator {
    struct tldnode *pointer;
} ;

// File Specific Prototypes
// Tree Impementation
TLDNode *tldnode_create(char *tld, TLDNode *parent);
void tldlist_inorder_count(TLDNode *node, long *tally);
void tldnode_destory(TLDNode *node);
void tldnode_destory_recursive(TLDNode *node);
// AVL Implementations
TLDNode *right_rotate(TLDNode *grandparent);
TLDNode *left_rotate(TLDNode *grandparent);
TLDNode *right_left_rotate(TLDNode *grandparent);
TLDNode *left_right_rotate(TLDNode *grandparent);
void setbalance(TLDNode *node);
void rebalance(TLDNode *node, TLDList *list);
int height(TLDNode *node);
void reheight(TLDNode *node);
long max(int a, int b);
//  String Based Implementions
char *tldstrip(char *s);

/*
/
/ TLDList Implementation
/
*/

TLDList *tldlist_create(Date *begin, Date *end) {

    // Assign space for new list in the heap
    TLDList *list = (TLDList *) malloc(sizeof(TLDList));
    if (list == NULL) { return NULL; }
    
    TLDNode *root = NULL;
    // If Malloc fails for either of the calls, return null
    if (begin == NULL || end == NULL) { free(list); return NULL; }

    // Ensure the start date is less than the end data
    if (date_compare(begin, end) > 0) { free(list); return NULL; }

    // Assign new pointers as members of list
    list->begin     = begin;
    list->end       = end;
    list->root      = root;
    return list;
}

void tldlist_destroy(TLDList *tld) {
    // Free all nodes in tree from heap
    tldnode_destory_recursive(tld->root);

    // Finally, free the list from memory from heap
    free(tld);
}

void tldnode_destory_recursive(TLDNode *node) {

    // This function recursively frees all linked nodes from a given input node
    // Base case: Node points to Null
    if (node == NULL) { return; }
    // First Case: Node's children points to null
    else if (node->left == NULL && node->right == NULL) { tldnode_destory(node); }
    // Second Case: Node's left child full but right child null
    else if (node->left != NULL && node->right == NULL) {
        tldnode_destory_recursive(node->left);
        tldnode_destory(node);
    }
    // Third Case: Node's right child full but left child null
    else if (node->left == NULL && node->right != NULL) {
        tldnode_destory_recursive(node->right);
        tldnode_destory(node);
    }
    // Fourth Case: Node has both full children
    else if (node->left != NULL && node->right != NULL) {
        tldnode_destory_recursive(node->left);
        tldnode_destory_recursive(node->right);
        tldnode_destory(node);
    }
}

int tldlist_add(TLDList *tld, char *hostname, Date *d) {

    // Local Variable to keep track of successful addition
    short int success = -1;

    // Condition Check: See if given date is within user's time peroid
    if (d == NULL || hostname == NULL || tld == NULL) { 
        return 0; 
    }
    if (date_compare(d,tld->begin) < 0 || date_compare(d,tld->end) > 0) { 
        return 0; 
    }

    // Base Case: The List has no node for the root
    if (tld->root == NULL) {
        tld->root = tldnode_create(hostname, NULL);
        success = (tld->root == NULL) ? 0 : 1;
        return success;         // If unsuccessful, return 0, else 1
    } 
    // If the list does have a root, search the tree for hostname and add it
    TLDNode *node = NULL;
    node = tld->root;
    while (success == -1) {
       
        // So we need the domain's TLD so we'll strip it and compare it to the current node's TLD
        char *temp = NULL;
        temp  = tldstrip(hostname);
        if (temp == NULL) { success=0; break; }
        int tld_diff = strcasecmp(temp, node->tld);  
        free(temp);

        // If current node's TLD is equal to the one we're searching for then add one to count
        if (tld_diff == 0) {
            node->count += 1;
            success = 1;
        }
        // Else we need to see if the tld is of greater or less value to the children of the current node
        else {
            // We'll also need to keep track of parent node just in case we need to add a node in this iteration
            TLDNode *parent = NULL;
            parent = node;
            short goLeft = tld_diff < 0;
            node = (goLeft) ? node->left : node->right;

            // If the leaf we are looking at is null then create a new node
            // If we don't enter this, we continue down the tree
            if (node == NULL) {
                if (goLeft) {
                    parent->left = tldnode_create(hostname, parent);
                    success = (parent->left == NULL ? 0 : 1);
                } else {
                    parent->right = tldnode_create(hostname, parent);
                    success = (parent->right == NULL ? 0 : 1);
                }
                // If we successfully added a node, rebalance the parent just incase the balance is off
                if (success > 0) { rebalance(parent, tld); };
            }
        }
    }
    return success;
}


long tldlist_count(TLDList *tld) {
    // Ensure passed tld is not null before turning a the number of sucessful additions
    long tally = 0;
    tldlist_inorder_count(tld->root, &tally);
    return tally;
}

void tldlist_inorder_count(TLDNode *node, long *tally) {
    if (node == NULL) {
        return ;
    }
    tldlist_inorder_count(node->left, tally);
    *tally += tldnode_count(node);
    tldlist_inorder_count(node->right, tally);
}

/*
/
/ AVL Implementation
/
*/

long max(int a, int b) {
    return (a > b) ? a : b;
}

void reheight(TLDNode *node) {
    if (node != NULL) {
        node->height = 1 + max(height(node->left), height(node->right));
    }
} 

int height(TLDNode *node) {
    if (node == NULL) {
        return -1; 
    } else {
        return node->height;
    }
}

void setbalance(TLDNode *node) {
    // Just recalculate the height and set the new balance
    reheight(node);
    node->balance = height(node->right) - height(node->left);
}

void rebalance(TLDNode *node, TLDList *list) {
    // Because we've just added a new node to this parent or moved it around, we'll need to check to see if it's in balance
    setbalance(node);

    // If it's not in balance, ie the left or right subtrees do not maintain the AVL property of a height difference of 1,
    // Then we'll need to rotate it's children to balance the tree
    if (node->balance == -2) {
        if (height(node->left->left) >= height(node->left->right)) {
            node = right_rotate(node);
        } else {
            node = left_right_rotate(node);
        }
    } else if (node->balance == 2) {
        if (height(node->right->right) >= height(node->right->left)) {
            node = left_rotate(node);
        } else {
            node = right_left_rotate(node);
        }
    }

    // If this returned node is the root, make it so, otherwise continue rebalancing the tree upwards
    if (node->parent != NULL) {
        rebalance(node->parent, list);
    } else {
        list->root = node;
    }
}

TLDNode *left_rotate(TLDNode *grandparent) {
    // If we do a left rotate, the parent of the new node, is the granparent's right node
    TLDNode *parent = grandparent->right;

    // Since the parent will be on top of the grandparent, it's parent will be the grandparent's
    parent->parent = grandparent->parent;
    grandparent->right = parent->left;

    // If the parent has a sub tree, make sure the subtree now points to the grandparent for it's parent
    if (grandparent->right != NULL) {
        grandparent->right->parent = grandparent;
    }
    
    // Flipping the parent and grandparent around
    parent->left = grandparent;
    grandparent->parent = parent;

    // Check if new node isn't the root and if it isn't, update his parent's pointers to point to him
    if (parent->parent != NULL) {
        if (parent->parent->right == grandparent) {
            parent->parent->right = parent;
        } else {
            parent->parent->left = parent;
        }
    }

    // Because we have shifted nodes around, we'll need to check their heights and balance
    setbalance(grandparent);
    setbalance(parent);

    // Return the parent as a node, we'll need this to check if it's a root or not
    return parent;
}

TLDNode *right_rotate(TLDNode *grandparent) {
    // If we do a right rotate, the parent of the new node, is the granparent's left node
    TLDNode *parent = grandparent->left;

    // Since the parent will be on top of the grandparent, it's parent will be the grandparent's
    parent->parent = grandparent->parent;
    
    // If the parent has a sub tree, make sure the subtree now points to the grandparent for it's parent
    grandparent->left = parent->right;
    if (grandparent->left != NULL) {
        grandparent->left->parent = grandparent;
    }
    
    // Flipping the parent and grandparent around
    parent->right = grandparent;
    grandparent->parent = parent;

    // Check if new node isn't the root and if it isn't, update his parent's pointers to point to him
    if (parent->parent != NULL) {
        if (parent->parent->right == grandparent) {
            parent->parent->right = parent;
        } else {
            parent->parent->left = parent;
        }
    }

    // Because we have shifted nodes around, we'll need to check their heights and balance
    setbalance(grandparent);
    setbalance(parent);

    // Return the parent as a node, we'll need this to check if it's a root or not
    return parent;
}

TLDNode *left_right_rotate(TLDNode *grandparent) {
    // Rotate parent so we can do an easier rotation
    grandparent->left = left_rotate(grandparent->left);
    // Rotate grandparent
    return right_rotate(grandparent);
}

TLDNode *right_left_rotate(TLDNode *grandparent) {
    // Rotate parent so we can do an easier rotation
    grandparent->right = right_rotate(grandparent->right);
    // Rotate grandparent
    return left_rotate(grandparent);
}
/*
/
/ TLDIterator Implementation
/
*/

TLDIterator *tldlist_iter_create(TLDList *tld) {
    
    // Assign space for new iterator and it's members in the heap
    TLDIterator *iter = (TLDIterator *) malloc(sizeof(TLDIterator));

    // If malloc fails then return null
    if (iter == NULL) { return NULL; }

    // Get root
    TLDNode *root = tld->root;

    // Assign members to iterator
    iter->pointer = root;

    return iter;
}

TLDNode *tldlist_iter_next(TLDIterator *iter) {
    // Time Complexity: O(log(n))
    // We need to return the pointer in the iterator
    TLDNode *old_ptr = iter->pointer;

    // Ok, so we'll need to cover our bases
    if (iter->pointer == NULL) { return NULL; }

    // If we have the root of the tree and no children, return root, now point to null
    if (iter->pointer->left == NULL && iter->pointer->right == NULL && iter->pointer->parent == NULL) {
        iter->pointer = NULL;
        return old_ptr;
    }

    // Traverse down the tree left first
    if (iter->pointer->left != NULL) {
        iter->pointer = iter->pointer->left;
    } 
    // If we can't go left but we can go right, go right
    else if (iter->pointer->left == NULL && iter->pointer->right != NULL) {
        iter->pointer = iter->pointer->right;
    }
    // The node we're on has no children so 3 things can happen
    // 1) The current node's parent's right child is not null so we point to that
    // 2) We are the right child of the parent so we float up the tree untill we are not
    // 3) We are the left child of the parent but the parent has no right child so we float up the tree until it does
    else {
        while ((iter->pointer == iter->pointer->parent->right) || ((iter->pointer == iter->pointer->parent->left) && iter->pointer->parent->right == NULL)) {
            iter->pointer = iter->pointer->parent;
            if (iter->pointer->parent == NULL) {
                iter->pointer = NULL;
                return old_ptr;
            }
        }
        iter->pointer = iter->pointer->parent->right;
    }
    return old_ptr;
}

void tldlist_iter_destroy(TLDIterator *iter) {
    free(iter);
}

/*
/
/ TLDNode Implementation
/
*/

TLDNode *tldnode_create(char *tld, TLDNode *parent) {
    
    // Assign space for new node in the heap
    TLDNode *node = (TLDNode *) malloc(sizeof(TLDNode));
    if (node == NULL) { 
        return NULL; 
    }
    char *domain = NULL;
    domain = tldstrip(tld);
    if (domain == NULL) { 
        free(node); return NULL;
    }


    // Assign members to the new node
    node->parent  = parent;
    node->left    = NULL;
    node->right   = NULL;
    node->count   = 1;
    node->tld     = domain;
    node->height  = 0;
    node->balance = 0;
    return node;
}

char *tldnode_tldname(TLDNode *node) {
    return node->tld;
}


long tldnode_count(TLDNode *node) {
    return node->count;
}


void tldnode_destory(TLDNode *node) {
    // Don't forget to free duplicated string and node 
    // The children will be free in a parent call of tld_destory_recurrisive
    // Do not use this function by itself, make sure to call ^
    free(node->tld);
    free(node);
}

/*
/
/ String Implementations
/
*/

// Given an input domain, stip it down and return the TLD
char *tldstrip(char *str) {
    if (str == NULL) { return NULL; }
    char *p = strdup(str);
    if (p == NULL) { return NULL; }
    char *p_start = p;
    char *tld = p;
    while(*p != '\0') {
        if (*p == '.') {
            tld = ++p;
        }
        p++;
    }
    char *stripped = (char *) malloc(sizeof(char) * ((p+1)-tld));
    if (stripped != NULL) {
        for (int i=0; (stripped[i] = tld[i]) != '\0'; i++) {}
    }
    free(p_start);
    return stripped;
}
