#include <stdlib.h>
#include <stdio.h>
#include "tldlist.h"
//#include "date.h"
#include "date.h"
#include <strings.h>


// Definitions for each structure
typedef struct tldlist {
    struct date *begin;
    struct date *end;
    struct tldnode *root;
} TLDList;

typedef struct tldnode {
    struct tldnode *right;
    struct tldnode *left;
    struct tldnode *parent;
    char *tld;
    long count;
    long height;
    long balance;
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

// Tree Impementation
int tldlist_add_recurrsive(TLDNode *node, char* hostname, TLDList *list);
long tldlist_count(TLDList *tld);
void tldlist_inorder_count(TLDNode *node, long *tally);
TLDNode *tldnode_create(char *tld, TLDNode *parent);
void tldnode_destory(TLDNode *node);
void tldnode_destory_recursive(TLDNode *node);
void iter_check(TLDList *tld);
void tldlist_iter_test(TLDNode *root);
// AVL Implementations
TLDNode *right_rotate(TLDNode *node);
TLDNode *left_rotate(TLDNode *node);
TLDNode *right_left_rotate(TLDNode *node);
TLDNode *left_right_rotate(TLDNode *node);
void setbalance(TLDNode *node);
void rebalance(TLDNode *node, TLDList *list);
int height(TLDNode *node);
void reheight(TLDNode *node);
long max(int a, int b);
// Stack Implementation
Stack *stack_create();
void stack_destory(Stack *stack);
TLDNode *stack_top(Stack *stack);
int stack_push(Stack *stack, TLDNode *tldnode);
TLDNode *stack_pop(Stack *stack);
Node *node_create(TLDNode *tldnode);
void node_destory(Node *node);
//  String Based Implementions
char *strduplicate(char *s);
int strcompared(char *s, char *d);
char *tldstrip(char *s);


/*
/
/ TLDList Implementation
/
*/

TLDList *tldlist_create(Date *begin, Date *end) {

    // Assign space for new list in the heap
    TLDList *list = (TLDList *) malloc(sizeof(TLDList));
    TLDNode *root = NULL;
    
    // If Malloc fails for either of the calls, return null
    if (list == NULL || begin == NULL || end == NULL) { return NULL; }

    // Ensure the start date is less than the end data
    if (date_compare(begin, end) > 0) { return NULL; }

    // Assign new pointers as members of list
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
    TLDNode *node = tld->root;
    while (success == -1) {
        // So we need the domain's TLD so we'll strip it and compare it to the current node's TLD
        char *temp  = tldstrip(hostname);
        int tld_diff = strcompared(temp, node->tld);  
        free(temp);

        // If current node's TLD is equal to the one we're searching for then add one to count
        if (tld_diff == 0) {
            node->count += 1;
            success = 1;
        }
        // Else we need to see if the tld is of greater or less value to the children of the current node
        else {
            // We'll also need to keep track of parent node just in case we need to add a node in this iteration
            TLDNode *parent = node;
            short goLeft = tld_diff < 0;
            node = (goLeft) ? node->left : node->right;

            // If the leaf we are look at is null then create a new node
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
        // Watch out, this is a recurisive function this could be a memory hog in a big tree - maybe a do while could solve this
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
    TLDNode *current_node = stack_pop(history);

    // Push current node's children to stack
    if (current_node != NULL) {
        stack_push(history, current_node->right);
        stack_push(history, current_node->left);
    }
    return current_node;
}

void tldlist_iter_destroy(TLDIterator *iter) {

    // Make sure to remove iterator and it's dynamically allocated members off of the heap
    stack_destory(iter->history);
    free(iter);
}

void iter_check(TLDList *tld) {
    tldlist_iter_test(tld->root);
}

void tldlist_iter_test(TLDNode *root) {
    if (root == NULL) { return; }
    tldlist_iter_test(root->left);
    printf("%s ", root->tld);
    tldlist_iter_test(root->right);
}

/*
/
/ TLDNode Implementation
/
*/

TLDNode *tldnode_create(char *tld, TLDNode *parent) {
    
    // Assign space for new node in the heap
    TLDNode *node = (TLDNode *) malloc(sizeof(TLDNode));
    char *domain = tldstrip(tld);

    // If Malloc fails for either of the calls, return null
    if (domain == NULL || node == NULL) { return NULL; }

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

// Compares the values of two Strings
// <0  : If first different character in s has a lesser value than d
// ==0 : Both strings are equal
// >0  : If first different character in s has a greater value than d
int strcompared(char *s, char *d) {
    int compar;
    // Record the difference in two strings 
    //while (((compar = (*(s++)-*(d++))) == 0) && *(s-1) != '\0' && *(d-1) != '\0') {}
    //return compar;
    return strcasecmp(s,d);
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

char *tldstrip(char *str) {
    char *p = strduplicate(str);
    char *tld = p;
    while(*(p++) != '\0') {
        if (*p == '.') {
            tld = ++p;
        }
    }
    char *stripped = (char *) malloc(sizeof(char) * (p-tld));
    if (stripped != NULL) {
        for (int i=0; (stripped[i] = tld[i]) != '\0'; i++) {}
    }
    free(p);
    return stripped;
}

/*
/
/ Stack Implementation
/
*/

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

    // If ran out of memory return 0 or node given doesn't exist
    if (node == NULL || tldnode == NULL) { return 0; }

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
        TLDNode *node = stack->header->value;
        Node *p = stack->header;
        stack->header = stack->header->next;
        node_destory(p);
        return node;
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


/*
    Testing Grounds
*/
/*
void print_results(TLDList *list) {
    // Print in order
    iter_check(list);
    printf("\n");
    // Count number of additions
    printf("%d",tldlist_count(list));
    printf("\n");
    // Reveal nodes in tree
    TLDIterator *iter = tldlist_iter_create(list);
    TLDNode *p = tldlist_iter_next(iter);
    while (p != NULL) {
        printf("Node: %s Count: %ld \n", p->tld, p->count);
        p = tldlist_iter_next(iter);
    }
    tldlist_iter_destroy(iter);
}

void print_hostname(char *s) {
    char *temp = tldstrip(s);
    printf("%s \n", temp);
    free(temp);
}

void compare_hostname(char *a, char *b) {

    char *atemp = tldstrip(a);
    char *btemp = tldstrip(b);

    int cmp = strcompared(atemp, btemp);

    if (cmp > 0) {
        printf("%s is greater than %s\n", atemp, btemp);
    } else if (cmp == 0) {
        printf("%s is equal to %s\n", atemp, btemp);
    } else {
        printf("%s is less than %s\n", atemp, btemp);
    }
    free(atemp);
    free(btemp);
}

void compare_dates(Date *d1, Date *d2) {
    int cmp = date_compare(d1,d2);

    if (cmp < 0) {
        printf("%d/%d/%d is before %d/%d/%d", d1->day,d1->month,d1->year,d2->day,d2->month,d2->year);
    } else if (cmp == 0) {
        printf("%d/%d/%d are the same before %d/%d/%d", d1->day,d1->month,d1->year,d2->day,d2->month,d2->year);
    } else {
        printf("%d/%d/%d is after %d/%d/%d", d1->day,d1->month,d1->year,d2->day,d2->month,d2->year);
    }

}

void compare_dates_range(Date *begin, Date *end, Date *d) {
    int begincmp = date_compare(d,begin);
    int endcmp = date_compare(d,end);

    if (begincmp > 0 && endcmp < 0) {
        printf("\nDate is in range!\n");
    } else if (begincmp == 0 && endcmp == 0) {
        printf("\nBegin, end and date are the same\n");
    } else if (begincmp < 0 || endcmp > 0) {
        printf("\nDate is out of bounds\n");
    } else if (begincmp == 0) {
        printf("\nDate is at the beginning\n");
    } else if (endcmp == 0) {
        printf("\nDate is at end\n");
    }
}

int main() {

    Date *begin = date_create("12/03/1999");
    Date *end = date_create("12/03/2020");
    Date *in = date_create("12/01/2013");
    Date *under = date_create("11/03/1999");
    Date *over = date_create("13/03/2020");
    TLDList *a = tldlist_create(begin,end);
    TLDList *a2 = tldlist_create(begin,end);
    TLDList *a3 = tldlist_create(begin,end);
    TLDList *b = tldlist_create(begin,end);
    TLDList *b2 = tldlist_create(begin,end);
    TLDList *b3 = tldlist_create(begin,end);
    TLDList *c1 = tldlist_create(begin,end);
    
    // Test case a
    printf("\n\nTest A1 \n");
    tldlist_add(a, "20", in);
    tldlist_add(a, "04", in);
    tldlist_add(a, "15", in);
    print_results(a);
    
    // Test for left right rotate
    printf("\n\nTest A2 \n");
    tldlist_add(a2, "20", in);
    tldlist_add(a2, "04", in);
    tldlist_add(a2, "26", in);
    tldlist_add(a2, "03", in);
    tldlist_add(a2, "09", in);
    tldlist_add(a2, "15", in);
    print_results(a2);

    // Test for double left right rotate
    printf("\n\nTest A3 \n");
    tldlist_add(a3, "20", in);
    tldlist_add(a3, "04", in);
    tldlist_add(a3, "26", in);
    tldlist_add(a3, "03", in);
    tldlist_add(a3, "09", in);
    tldlist_add(a3, "21", in);
    tldlist_add(a3, "30", in);
    tldlist_add(a3, "02", in);
    tldlist_add(a3, "07", in);
    tldlist_add(a3, "11", in);
    tldlist_add(a3, "15", in);
    print_results(a3); 

    // Test for b
    printf("\n\nTest B1 \n");
    tldlist_add(b, "20", in);
    tldlist_add(b, "04", in);
    tldlist_add(b, "08", in);
    print_results(b);

    //
    printf("\n\nTest B2 \n");
    tldlist_add(b2, "20", in);
    tldlist_add(b2, "04", in);
    tldlist_add(b2, "26", in);
    tldlist_add(b2, "03", in);
    tldlist_add(b2, "09", in);
    tldlist_add(b2, "08", in);
    print_results(b2);

    //
    printf("\n\nTest B3 \n");
    tldlist_add(b3, "20", in);
    tldlist_add(b3, "04", in);
    tldlist_add(b3, "26", in);
    tldlist_add(b3, "03", in);
    tldlist_add(b3, "09", in);
    tldlist_add(b3, "21", in);
    tldlist_add(b3, "30", in);
    tldlist_add(b3, "02", in);
    tldlist_add(b3, "07", in);
    tldlist_add(b3, "11", in);
    tldlist_add(b3, "08", in);
    print_results(b3);

    // Test for additions
    printf("\n\nTest C1 \n");
    tldlist_add(c1, "20", in);
    tldlist_add(c1, "04", in);
    tldlist_add(c1, "26", in);
    tldlist_add(c1, "03", in);
    tldlist_add(c1, "09", in);
    tldlist_add(c1, "21", in);
    tldlist_add(c1, "30", in);
    tldlist_add(c1, "02", in);
    tldlist_add(c1, "07", in);
    tldlist_add(c1, "11", in);
    tldlist_add(c1, "08", in);
    tldlist_add(c1, "08", over);
    tldlist_add(c1, "08", under);
    tldlist_add(c1, "08", begin);
    tldlist_add(c1, "08", end);
    tldlist_add(c1, "08", in);
    tldlist_add(c1, "02", over);
    tldlist_add(c1, "02", under);
    tldlist_add(c1, "02", begin);
    tldlist_add(c1, "02", end);
    tldlist_add(c1, "02", in);
    print_results(c1);

    // Test for input strings
    printf("\n\nDomain Testing \n");
    print_hostname("hello.co.uk");
    print_hostname("msnbot.msn.com");
    print_hostname("msnbot.msn.cc");
    print_hostname("groupe02.ac-lille.fr");
    print_hostname("cs168130.pp.htv.fi");

    // Compare two closely related strings
    // Should be greater
    compare_hostname("msnbot.msn.com","msnbot.msn.cc");
    // Should be lesser
    compare_hostname("groupe02.ac-lille.fi","cs168130.pp.htv.fr");
    // Should be equal
    compare_hostname("groupe02.ac-lille.com","cs168130.pp.htv.com");
    compare_hostname("groupe02.ac-lille.cccom","cs168130.pp.htv.cccom");
    compare_hostname("mdk.rafael.co.il","gateway.iiap.res.in");

    // Test for comparing dates
    Date *ends = date_create("01/09/2020");
    Date *begins = date_create("01/01/2017");
    compare_dates(begins, ends);
    
    // Date for dates in range 
    Date *onlarger = date_create("01/09/2020");
    Date *onsmaller = date_create("01/01/2017");
    Date *inrange1 = date_create("01/02/2017");
    Date *inrange2 = date_create("31/08/2020");
    Date *outbounds = date_create("31/12/2016");
    compare_dates_range(begins, ends, onlarger);
    compare_dates_range(begins, ends, onsmaller);
    compare_dates_range(begins, ends, inrange1);
    compare_dates_range(begins, ends, inrange2);
    compare_dates_range(begins, ends, outbounds);

    
    tldlist_destroy(a);
    tldlist_destroy(a2);
    tldlist_destroy(a3);
    date_destroy(begin);
    date_destroy(end);
}  */
