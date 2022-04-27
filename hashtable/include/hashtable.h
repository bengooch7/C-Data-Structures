#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../include/l_list.h"


typedef struct hashtable
{
    int curr_size;
    int item_count;
    llist_t ** buckets;
} hashtable_t;


/**
 * String hashing algorithm found at:
 * http://www.cse.yorku.ca/~oz/hash.html
 * @param str
 * @return
 */
unsigned long
hash(unsigned char *str);


/***
 * Create and return a hash table
 * @return Pointer to a hash table, if successful, NULL if failed.
 */
hashtable_t *
init_hash_table();


/***
 * Place an item into the provided hash table assuming it does not yet exist
 * @param p_table The table for item insertion
 * @param p_item The item to be inserted into the table
 * @return True if successful, false if failed
 */
bool
hash_table_insert(hashtable_t * p_table, void * p_item);


/***
 * Search for an item in the table
 * @param p_table The hash table to search for the item
 * @param p_item The item to be searched for
 * @return NULL if item is not found, list node containing item if found
 */
llist_node_t *
table_find_item (hashtable_t * p_table, void * p_item);


/***
 * Remove an item from the provided hash table
 * @param p_table The table in which to find the item to be removed
 * @param p_item The item that should be removed, if found
 */
void
table_rm_item (hashtable_t * p_table, void * p_item);


/**
 * Remove all items from the hashtable
 * @param p_table the table to clear
 */
void
hash_table_clear(hashtable_t * p_table);


/**
 * Completely deallocate an existing hash table
 * @param p_table table to be destroyed
 */
void
hash_table_destroy(hashtable_t ** p_table);


/***
 * Print a text representation to the terminal of the current status of the hash
 * table
 * @param p_table The table to print
 */
void
print_table(hashtable_t * p_table);

#endif //HASHTABLE_H
