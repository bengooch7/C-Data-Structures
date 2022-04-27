#include "../include/l_list.h"


/* llist_new() creates a new linked list
    Args:
        No arguments
    Return:
        llist_t * to a newly allocated linked list.
*/
llist_t *
llist_new(void)
{
    llist_t * p_new_llist = calloc(1, sizeof(llist_t));

    p_new_llist->head = NULL;
    p_new_llist->tail = NULL;

    return p_new_llist;
}

/* llist_add() adds to existing linked list
    Args:
        llist_t * list : the llist* to add the element to
        int data     : the specific data element to add
    Return:
        bool: true on success, false on failure
*/
bool
llist_add(llist_t * p_list, void * data)
{

    if (!p_list)
    {
        fprintf(stderr, "\n%s: No list provided\n", __func__);
        return false;
    }

    llist_node_t * p_new_node = calloc(1, sizeof(llist_node_t));
    if(!p_new_node)
    {
        fprintf(stderr, "\n%s: Failed to allocate for new node", __func__);
        return false;
    }

    p_new_node->data = data;

    if (!p_list->head)
    {
        p_list->head = p_new_node;
        p_list->tail = p_new_node;

    }
    else
    {
        p_new_node->next = p_list->head;
        p_list->head = p_new_node;

    }

    return true;
}

/* llist_find() searches a linked list for the first occurrence of a given value
    Args:
        llist_t * list : the llist* to find the element in
        int data     : the specific data element to find
        comp_f       : the comparison function you want to use
                        Valid options: ll_comp_str, ll_comp_int, ll_comp_uint
    Return:
        llist_node_t* of the found node. null if node not found.
*/
llist_node_t *
llist_find(llist_t * list, void * data, int (*comp_f)(const void *, const void *))
{
    if (!list)
    {
        fprintf (stderr, "\n%s: No list provided\n", __func__);
        return NULL;
    }

    if (!comp_f)
    {
        fprintf (stderr, "\n%s: No function pointer provided\n", __func__);
        return NULL;
    }

    llist_node_t * p_curr_node = list->head;

    do
    {
        int res = comp_f(p_curr_node->data, data);

        if (0 == res) //Good match
        {
            return p_curr_node;
        }

        p_curr_node = p_curr_node->next;

    }
    while (p_curr_node);

    return NULL;
}

/* llist_rm_head() removes only the head of a linked list
    Args:
        llist_t * list : the llist * to remove the head from
    Return:
        llist_node_t* of the removed head. null if list is empty.
*/
llist_node_t *
llist_rm_head(llist_t * list)
{
    if (!list) //No list
    {
        fprintf(stderr, "\n%s: No list provided\n", __func__);
        return NULL;
    }

    if (!list->head) //Empty list
    {
        return NULL;
    }

    llist_node_t * old_head = list->head;

    if (!list->head->next) //Single node list
    {
        list->head = NULL;
        list->tail = NULL;
        return old_head;
    }

    list->head = list->head->next;

    return old_head;
}

/* llist_rm_tail() removes only the tail of a linked list
    Args:
        llist_t * list : the llist* to remove the tail from
    Return:
        llist_node_t* of the removed tail. null if list is empty.
*/
llist_node_t *
llist_rm_tail(llist_t * list)
{
    if (!list) //No list
    {
        fprintf(stderr, "/n%s: No list provided\n", __func__);
        return NULL;
    }

    if (!list->head) //Empty list
    {
        return NULL;
    }

    llist_node_t * old_tail = list->tail;
    llist_node_t * p_curr_node = list->head;

    while (p_curr_node->next != list->tail)
    {
        p_curr_node = p_curr_node->next; //increment to next node
    }

    list->tail = p_curr_node;

    return old_tail;
}

/* llist_rm() removes from existing linked list
    Args:
        llist_t * list : the llist* to remove the element from
        int data     : the specific data element to remove
    Return:
        llist_node_t* of the removed node. null if node not found.
*/
llist_node_t *
llist_rm (llist_t * p_list, void * data,
         int (*comp_f)(const void *, const void *))
{
    if (!p_list)
    {
        fprintf(stderr, "/n%s: No list provided\n", __func__);
        return NULL;
    }

    if (!p_list->head)
    {
        return NULL;
    }

    llist_node_t * p_curr_node = p_list->head;
    llist_node_t * p_prev_node = NULL;

    do
    {
        int res = comp_f(p_curr_node->data, data);

        if (0 == res) //Match found
        {
            //Single item in list
            if ((p_list->head == p_curr_node) && (p_list->tail == p_curr_node))
            {
                p_list->head = NULL;
                p_list->tail = NULL;
                return p_curr_node;
            }

            //Item is the head of the list
            if (p_list->head == p_curr_node)
            {
                p_list->head = p_curr_node->next;
                return p_curr_node;
            }

            p_prev_node->next = p_curr_node->next;

            //Last item on the list (tail)
            if (!p_prev_node->next)
            {
                p_list->tail = p_prev_node;
            }

            return p_curr_node;
        }

        //Increment to next nodes
        p_prev_node = p_curr_node;
        p_curr_node = p_curr_node->next;

    }
    while (p_curr_node); //No match and end of list has not been reached

    return NULL;

}


/* llist_destroy() destroys a linked list
    Args:
        llist_t * list : llist * to destroy
    Return:
        bool : true on success, false of failure
*/
bool
llist_destroy(llist_t ** p_list)
{
    if ((!p_list) || (!*p_list)) //No list
    {
        fprintf(stderr, "\n%s: No list provided\n", __func__);
        return false;
    }

    if (!(*p_list)->head) //Empty list
    {
        free((*p_list));
        *p_list = NULL;
        return true;
    }

    while ((*p_list)->head)
    {
        llist_node_t * p_old_head = llist_rm_head((*p_list));
        free(p_old_head);
        p_old_head = NULL;
    }


    free(*p_list);
    *p_list = NULL;

    return true;
}


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
ll_comp_str(const void * item_a, const void * item_b)
{
    if ((!item_a) || (!item_b))
    {
        fprintf(stderr,
                "/n%s: No string(s) provided for comparison\n", __func__);
        return -1;
    }

    //Get length of strings
    size_t a_len = strnlen((char *) item_a, MAX_LEN);
    size_t b_len = strnlen((char *) item_b, MAX_LEN);

    int longer = a_len > b_len ? a_len : b_len;

    return strncmp((char *) item_a, (char *) item_b, longer);

}

/***
 * Comapre two ints
 * @param int_a the first int for comparison
 * @param int_b the second int for comparison
 * @return -1 if not equal, 0 if equal
 */
int
ll_comp_int(const void * int_a, const void * int_b)
{
    if ((int *) int_a != (int *) int_b)
    {
        return -1;
    }

    return 0;

}

/***
 * Comapre two uint32_ts
 * @param uint_a the first uint32_t for comparison
 * @param uint_b the second uint32_t for comparison
 * @return -1 if not equal, 0 if equal
 */
int
ll_comp_uint(const void * uint_a, const void * uint_b)
{
    if ((uint32_t *) uint_a != (uint32_t *) uint_b)
    {
        return -1;
    }

    return 0;

}

/***
 * Print a visual representation of the linked list showing how many items are
 * in the list
 * @param p_list The linked list to be represented visually
 */
void
print_list(llist_t * p_list)
{
    if (!p_list)
    {
        fprintf(stderr, "\n%s: There was no list provided\n", __func__);
        return;
    }

    printf("|--[");

    if (!p_list->head)
    {
        printf(" Empty list ");
    }

    llist_node_t * p_curr_node = p_list->head;

    while (p_curr_node)
    {
        printf("%s",(char *)p_curr_node->data);
        if (p_curr_node->next)
        {
            printf("]--[");
        }
        p_curr_node = p_curr_node->next;
    }

    printf("]--|\n");
}


/**END OF FILE**/
