#include <stdlib.h>
#include <stdio.h>
#include "tldlist.h"


// Definitions for each structure
typedef struct tldlist {
    struct date *begin;
    struct date *end;
    struct tldnode *root;
    long additions;
} TLDList;

typedef struct tldnode {
    struct tldnode *right;
    struct tldnode *left;
    char *tld;
    long count;
} TLDNode;

typedef struct tlditerator {
    struct stack *history;
} TLDIterator;

typedef struct stack {
    struct node *header;
} Stack;

typedef struct node {
    struct node *next;
    struct tldnode *value;
} Node;

// File Specific Prototypes
int tldlist_add_recurrsive(TLDNode *node, char* hostname);
long tldlist_count(TLDList *tld);
TLDNode *tldnode_create(char *tld);
void tldnode_destory(TLDNode *node);
void tldnode_destory_recursive(TLDNode *node);
char *strduplicate(char *s);
int strcompare(char *s, char *d);
Stack *stack_create();
void stack_destory(Stack *stack);
TLDNode *stack_top(Stack *stack);
int stack_push(Stack *stack, TLDNode *tldnode);
TLDNode *stack_pop(Stack *stack);
Node *node_create(TLDNode *tldnode);
void node_destory(Node *node);



TLDList *tldlist_create(Date *begin, Date *end) {

    // Assign space for new list in the heap
    TLDList *list = (TLDList *) malloc(sizeof(TLDList));
    TLDNode *root = NULL;
    
    // If Malloc fails for either of the calls, return null
    if (list == NULL || begin == NULL || end == NULL) { return NULL; }

    // Ensure the start date is less than the end data
    if (date_compare(begin, end) > 0) { return NULL; }

    // Assign new pointers as members of list
    list->additions = 0;
    list->begin     = begin;
    list->end       = end;
    list->root      = root;
    return list;
}

void tldlist_destroy(TLDList *tld) {

    // Free all dates from list
    date_destroy(tld->begin);
    date_destroy(tld->end);

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
    return;
}

int tldlist_add(TLDList *tld, char *hostname, Date *d) {

    // Local Variable to keep track of successful addition
    short int success;

    // Condition Check: See if given date is within user's time peroid
    if (date_compare(d,tld->begin) < 0 || date_compare(d,tld->end) > 0) { return 0; }

    // Base Case: The List has no node for the root
    if (tld->root == NULL) {
        tld->root = tldnode_create(hostname);
        tld->additions += (success = (tld->root == NULL) ? 0 : 1);
        return success;         // If unsuccessful, return 1, else 0
    } else {
        // If the list does have a root, search the tree for hostname and add it
        tld->additions += (success = tldlist_add_recurrsive(tld->root, hostname));
        return success;
    }
}

int tldlist_add_recurrsive(TLDNode *node, char *hostname) {
    int str_cmp = strcompare(hostname, node->tld);  

    // Compare the hostname to the current node's hostname, if a match, add 1 to count
    if (str_cmp == 0) {
        node->count += 1;
        return 1;
    }
    // If hostname doesn't match the comparison is less than the current tld
    else if (str_cmp < 0) {
        if (node->left == NULL) {
            // If no left child, make one 
            node->left = tldnode_create(hostname);
            return (node->left == NULL ? 0 : 1);       // If can't make node, return 0
        } else {
            // If a left child, exists - make a recursive call and return the result
            return tldlist_add_recurrsive(node->left, hostname);
        }
    } else {
        // Check the right child of node
        if (node->right == NULL) {
            // If no left child, make one 
            node->right = tldnode_create(hostname);
            return (node->right == NULL ? 0 : 1);       // If can't make node, return 0
        } else {
            // If a left child, exists - make a recursive call and return the result
            return tldlist_add_recurrsive(node->right, hostname);
        }
    }
}

TLDIterator *tldlist_iter_create(TLDList *tld) {
    
    // Assign space for new iterator and it's members in the heap
    TLDIterator *iter = (TLDIterator *) malloc(sizeof(TLDIterator));
    Stack *history = stack_create();

    // If malloc fails then return null
    if (iter == NULL || history == NULL) { return NULL; }

    // Assign the root of the tree to first in history list
    stack_push(history, tld->root);

    // Assign members to iterator
    iter->history = history;
    return iter;
}

TLDNode *tldlist_iter_next(TLDIterator *iter) {

    // Iterate in depth-search fashion and in order.

    // So let's take it step at a time
    // So the next node will be at the top of the history as the root is already there.
    Stack *history = iter->history;
    TLDNode *current_node = stack_top(history);

    // Base case: If no more items return null
    if (current_node == NULL) {
        return current_node;
    }
    // First Case:If current node has a left child, continue down the tree
    else if (current_node->left != NULL) {
        stack_push(history, current_node->left);
    } 
    // Second Case: If current node doesn't have a left child but has a right child
    else if (current_node->left == NULL && current_node->right != NULL) {
        stack_push(history, current_node->right);
    }
    // Third Case: If current node has no children, start to peel back history
    else if (current_node->left == NULL && current_node->right == NULL) {
        TLDNode *prev_node = stack_pop(history);
        while (stack_top(history) != NULL && (stack_top(history)->right == prev_node || stack_top(history)->right == NULL)) {
            prev_node = stack_pop(history);
        }
        if (stack_top(history) == NULL) {
            return current_node;
        }
        else if (stack_top(history)->right != NULL) {
            stack_push(history, stack_top(history)->right);
        }
    } 

    return current_node;
}

void tldlist_iter_destroy(TLDIterator *iter) {

    // Make sure to remove iterator and it's dynamically allocated members off of the heap
    stack_destory(iter->history);
    free(iter);
}

long tldlist_count(TLDList *tld) {
    // Ensure passed tld is not null before turning a the number of sucessful additions
    return (tld == NULL) ? 0 : tld->additions;
}

TLDNode *tldnode_create(char *tld) {
    
    // Assign space for new node in the heap
    TLDNode *node = (TLDNode *) malloc(sizeof(TLDNode));
    char *domain = strduplicate(tld);

    // If Malloc fails for either of the calls, return null
    if (domain == NULL || node == NULL) { return NULL; }

    // Assign members to the new node
    node->left  = NULL;
    node->right = NULL;
    node->count = 1;
    node->tld   = domain;
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

// Compares the values of two Strings
// <0  : If first different character in s has a lesser value than d
// ==0 : Both strings are equal
// >0  : If first different character in s has a greater value than d
int strcompare(char *s, char *d) {
    int compar;
    // Record the difference in two strings 
    while (((compar = (*(s++)-*(d++))) == 0) && *(s-1) != '\0' && *(d-1) != '\0') {}
    return compar;
}

// Duplicate String and save it to heap
char *strduplicate(char *str) {
    // Get string length (p+1)-str
    char *p = str;
    while (*p != '\0') { p++; }
    // Get space for pointer
    p = (char *) malloc(sizeof(char) * ((++p)-str));
    // Copy contents of str into p
    if (p != NULL) {
        for (int i=0; (p[i] = str[i]) != '\0'; i++) {}
    }
    return p;
}

// Simple Stack implementation for iterator
Stack *stack_create() {

    // Allocate Space for new stack
    Stack *stack = (Stack *) malloc(sizeof(stack));
    
    // If ran out of memory return null
    if (stack == NULL) {return NULL;}

    // Assign members to new stack
    stack->header = NULL;
    return stack;

}

int stack_push(Stack *stack, TLDNode *tldnode) {

    // Create a node new containing the node of the tldlist
    Node* node = node_create(tldnode);

    // If ran out of memory return 0
    if (node == NULL) { return 0; }

    // Change the header to the new node
    node->next = stack->header;
    stack->header = node;

    return 1;
}

TLDNode *stack_pop(Stack *stack) {

    // Make sure stack is not null
    if (stack == NULL) { return NULL; }

    // Remove node from top of stack
    if (stack->header != NULL) {
        TLDNode *tld = stack->header->value;
        Node *p = stack->header;
        stack->header = stack->header->next;
        node_destory(p);
        return tld;
    } else {
        return NULL;
    }
}

TLDNode *stack_top(Stack *stack) {

    // Make sure stack is not null
    if (stack == NULL) { return NULL; }

    // Get the value at the top of the stack if it isn't null
    return (stack->header == NULL) ? NULL : stack->header->value;
}

void stack_destory(Stack *stack) {
    // Before freeing stack, all nodes must be freed
    while (stack->header != NULL) {
        Node *p = stack->header;
        stack->header = stack->header->next;
        node_destory(p);
    }
    free(stack);
}

void stack_print(Stack *stack) {
    Node *p;
    p = stack->header;
    while (p != NULL) {
        printf("%s|", p->value->tld);
        p = p->next;
    }
    printf("\n");
}

Node *node_create(TLDNode *tldnode) {
    
    // Allocate Space for new node
    Node *newnode = (Node *) malloc(sizeof(Node));
    
    // If ran out of memory return null
    if (newnode == NULL) {return NULL;}

    // Assign members to new node
    newnode->value = tldnode;
    newnode->next  = NULL;
    return newnode;
}

void node_destory(Node *node) {
    // Remove node from heap
    free(node);
}

