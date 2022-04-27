#ifndef L_LIST_H
#define L_LIST_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_LEN 256

/*
 * This node holds a single signed integer for the sake of this exercise.
 * For real-world use it could point to another struct or a void pointer.
 */
typedef struct llist_node llist_node_t;

struct llist_node
{
    void * data;
    llist_node_t * next;
};

/*
 * This "overarching" struct is intended to encapsulate the nodes for both
 * the head and tail of the linked list.
 */
typedef struct llist
{
    llist_node_t * head;
    llist_node_t * tail;
} llist_t;


/* llist_new() creates a new linked list
    Args:
        No arguments
    Return:
        llist * to a newly allocated linked list.
*/
llist_t *
llist_new(void);

/* llist_add() adds to existing linked list
    Args:
        llist * list : the llist* to add the element to
        void * data  : the specific data element to add
    Return:
        bool: true on success, false on failure
*/
bool
llist_add(llist_t *list, void * data);

/* llist_find() searches a linked list for the first occurance of a given value
    Args:
        llist * list : the llist* to find the element in
        void * data  : the specific data element to find
    Return:
        llist_node* of the found node. null if node not found.
*/
llist_node_t *
llist_find(llist_t *list, void * data, int (*comp_f)(const void *, const void *));

/* llist_rm_head() removes only the head of a linked list
    Args:
        llist * list : the llist* to remove the head from
    Return:
        llist_node* of the removed head. null if list is empty.
*/
llist_node_t *
llist_rm_head(llist_t *list);

/* llist_rm_tail() removes only the tail of a linked list
    Args:
        llist * list : the llist* to remove the tail from
    Return:
        llist_node* of the removed tail. null if list is empty.
*/
llist_node_t *
llist_rm_tail(llist_t *list);

/* llist_rm() removes from existing linked list
    Args:
        llist * list : the llist* to remove the element from
         void * data : the specific data element to remove
    Return:
        llist_node* of the removed node. null if node not found.
*/
llist_node_t *
llist_rm(llist_t * list, void * data, int (*comp_f)(const void *, const void *));

/* llist_destroy() destroys a linked list
    Args:
        llist *list : llist * to destroy
    Return:
        bool : true on success, false of failure
*/
bool
llist_destroy(llist_t ** list);


/***
 * Compare two strings of characters for equality
 * @param item_a The first string
 * @param item_b The 2nd string
 * @return returns -1 if either string pointer is NULL or
 * 0 if the two strings match,
 * value > 0 if item_a is larger,
 * value < 0 if item_b is larger
 */
int
ll_comp_str(const void * item_a, const void * item_b);


/***
 * Comapre two ints
 * @param int_a the first int for comparison
 * @param int_b the second int for comparison
 * @return -1 if not equal, 0 if equal
 */
int
ll_comp_int(const void * int_a, const void * int_b);


/***
 * Comapre two uint32_ts
 * @param uint_a the first uint32_t for comparison
 * @param uint_b the second uint32_t for comparison
 * @return -1 if not equal, 0 if equal
 */
int
ll_comp_uint(const void * uint_a, const void * uint_b);


/***
 * Print a visual representation of the linked list showing how many items are
 * in the list
 * @param p_list The linked list to be represented visually
 */
void
print_list(llist_t * p_list);


#endif
