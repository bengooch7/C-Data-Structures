#include "../include/hashtable.h"

#include <stdio.h>

#define LOAD_FACTOR  0.7
#define INITIAL_SIZE 2
#define MAX_LEN      256

static void
check_load (hashtable_t * p_table);

static void
grow_table (hashtable_t * p_table, int new_size);

static llist_t **
setup_buckets(hashtable_t * p_table, int size);


/**
 * String hashing algorithm found at:
 * http://www.cse.yorku.ca/~oz/hash.html
 * @param str
 * @return
 */
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


/***
 * Create and return a hash table
 * @return Pointer to a hash table, if successful, NULL if failed.
 */
hashtable_t *
init_hash_table()
{
    hashtable_t * p_table = calloc(1, sizeof(hashtable_t));

    if (!p_table)
    {
        fprintf(stderr,"\n%s: Failed to allocate memory for table\n", __func__);
        return NULL;
    }

    p_table->buckets = setup_buckets(p_table, INITIAL_SIZE);

    if(!p_table->buckets)
    {
        return NULL;
    }

    p_table->curr_size  = INITIAL_SIZE;
    p_table->item_count = 0;

    return p_table;

}


/**
 * Sets up the link lists required to hold data in the hash table
 * @param p_table The hash table requiring buckets
 * @param size The number of buckets to add to the hash table
 * @return True if successful, False if failed
 */
static llist_t **
setup_buckets (hashtable_t * p_table, int size)
{
    if(!p_table)
    {
        fprintf(stderr, "\n%s: No valid table provided\n", __func__);
        return NULL;
    }

    llist_t ** buckets = calloc(size, sizeof(llist_t *));

    if(!buckets)
    {
        fprintf(stderr, "\n%s: Failed to allocate for buckets\n", __func__);
        return NULL;
    }

    for(int idx = 0; idx < size; idx++)
    {
        buckets[idx] = llist_new();

        if (!buckets[idx])
        {
            fprintf (stderr,
                   "\n%s: Failed to allocate for bucket [%d]\n", __func__, idx);
            goto CLEANUP;
        }

    }

    return buckets;

    CLEANUP:
        for(int idx = 0; idx < size; idx++)
        {
            free(buckets[idx]);
            buckets[idx] = NULL;
        }

        return NULL;

}


/***
 * Place an item into the provided hash table assuming it does not yet exist
 * @param p_table The table for item insertion
 * @param p_item The item to be inserted into the table
 * @return True if successful, false if failed
 */
bool
hash_table_insert(hashtable_t * p_table, void * p_item)
{
    if (!p_table)
    {
        fprintf(stderr, "\n%s: No valid hashtable provided\n", __func__);
    }

    if (!p_item)
    {
        fprintf(stderr, "\n%s: No item provided\n", __func__);
        return false;
    }

    llist_node_t * res = table_find_item(p_table, p_item);
    if (res) //Item already exists
    {
        printf("\n%s: Item already exists in table\n", __func__);
        return false;
    }

    //Check current load factor
    check_load(p_table);

    unsigned long location = hash(p_item);

    location = location % p_table->curr_size;

    bool result = llist_add(p_table->buckets[location], p_item);

    if(result)
    {
        p_table->item_count++;
        return true;
    }
    else
    {
        return false;
    }

}


/***
 * Search for an item in the table
 * @param p_table The hash table to search for the item
 * @param p_item The item to be searched for
 * @return NULL if item is not found, list node containing item if found
 */
llist_node_t *
table_find_item (hashtable_t * p_table, void * p_item)
{
    if((!p_table) || (!p_item))
    {
        fprintf(stderr, "\n%s: No table or no item provided\n", __func__);
        return NULL;
    }

    unsigned long location = hash(p_item);

    location = location % p_table->curr_size;

    if (p_table->buckets[location]->head) // nodes exists here
    {

        llist_node_t * res = llist_find(p_table->buckets[location],
                                       p_item,
                                       ll_comp_str);

        return res;

    }

    return NULL;

}


/***
 * Remove an item from the provided hash table
 * @param p_table The table in which to find the item to be removed
 * @param p_item The item that should be removed, if found
 */
void
table_rm_item (hashtable_t * p_table, void * p_item)
{
    if((!p_table) || (!p_item))
    {
        fprintf(stderr, "\n%s: No table or no item provided\n", __func__);
        return;
    }

    llist_node_t * res = table_find_item(p_table, p_item);
    llist_node_t * target_node = NULL;

    if(res)
    {

        int64_t location = hash(p_item);

        location = location % p_table->curr_size;

        target_node =
                llist_rm(p_table->buckets[location], p_item, ll_comp_int);

        if(target_node)
        {
            p_table->item_count--;
        }

    }
    else
    {
        printf("\n%s: Item does not exist!\n", __func__);
    }

    free(target_node);
    target_node = NULL;

}


/***
 * Double the size of the current table to better distribute the load
 * @param p_table The table to be resized
 * @param new_size The desired size of the current table, usually the current
 * table's size * 2
 */
static void
grow_table(hashtable_t * p_table, int new_size)
{
    if(!p_table)
    {
        fprintf(stderr, "\n%s: No hashtable provided\n", __func__);
        return;
    }

    if(p_table->curr_size > new_size)
    {
        fprintf(stderr, "\n%s: Invalid size proposed for new table", __func__);
        return;
    }

    //Make new, larger bucket array
    llist_t ** new_buckets = setup_buckets(p_table, new_size);

    //Rehash existing nodes to place them in the new hash table
    for(int idx = 0; idx < p_table->curr_size; idx++)
    {
        //If there is a bucket, go through it and rehash the items
        if(p_table->buckets[idx]->head)
        {
            llist_node_t * p_curr_node = NULL;
            llist_node_t * p_prev_node = NULL;

            do
            {
                p_curr_node = llist_rm_head(p_table->buckets[idx]);

                int64_t location = hash(p_curr_node->data) % new_size;

                if (!new_buckets[location]->head) //No chain exists at the new location
                {
                    new_buckets[location] = llist_new();
                    llist_add(new_buckets[location], p_curr_node->data);
                }
                else //Chain exists, just add to chain
                {
                    llist_add(new_buckets[location], p_curr_node->data);
                }

                p_prev_node = p_curr_node;
                p_curr_node = p_curr_node->next;

                free(p_prev_node);
                p_prev_node = NULL;

            } while (p_curr_node);

        }

        llist_destroy(&(p_table->buckets[idx]));

        printf("\n%s: Address of list: %p\n", __func__, p_table->buckets[idx]);


    }

    free(p_table->buckets);
    p_table->buckets = NULL;

    p_table->curr_size = new_size;
    p_table->buckets = new_buckets;

}


/***
 * Check to see if the current load of the table meets or exceeds the load limit
 * @param p_table The table to check load factor
 */
static void
check_load(hashtable_t * p_table)
{
    if (!p_table)
    {
        fprintf(stderr, "\n%s: No valid table provided", __func__);
        return;
    }

    if ((p_table->item_count / (float) p_table->curr_size) >= LOAD_FACTOR)
    {
        printf("\nDEBUG: Table needs to be resized\n");
        grow_table(p_table, p_table->curr_size * 2);
    }

    return;
}

/**
 * Remove all items from the hashtable
 * @param p_table the table to clear
 */
void
hash_table_clear(hashtable_t * p_table)
{
    if(!p_table)
    {
        fprintf(stderr, "\n%s: No hash table provided\n", __func__);
        return;
    }

    for(int idx = 0; idx < p_table->curr_size; idx++)
    {
        while(p_table->buckets[idx]->head)
        {
            llist_node_t * res = llist_rm_head(p_table->buckets[idx]);
            if(!res)
            {
                fprintf(stderr, "\n%s: Something went wrong removing nodes "
                                "from key %d\n", __func__, idx);
                return;
            }

            free(res);
            res = NULL;

            p_table->item_count--;
        }

    }

    if(p_table->item_count > 0)
    {
        fprintf(stderr, "\n%s: Something went wrong, not all items were cleared "
                        "from the hash table\n", __func__);
    }

    return;
}


/**
 * Completely deallocate an existing hash table
 * @param p_table table to be destroyed
 */
void
hash_table_destroy(hashtable_t ** p_table)
{
    if((!p_table) || (!*p_table))
    {
        fprintf(stderr, "\n%s: No hash table provided\n", __func__);
        return;
    }

    hash_table_clear(*p_table);

    for(int idx = 0; idx < (*p_table)->curr_size; idx++)
    {
        bool res = llist_destroy(&(*p_table)->buckets[idx]);
        if(!res)
        {
            fprintf(stderr, "\n%s: Failed to destroy a bucket\n", __func__);
            return;
        }

    }

    free(*p_table);
    *p_table = NULL;

    return;
}



/***
 * Print a text representation to the terminal of the current status of the hash
 * table
 * @param p_table The table to print
 */
void
print_table(hashtable_t * p_table)
{
    if (!p_table)
    {
        fprintf(stderr, "\n%s: No valid table provided\n", __func__);
        return;
    }

    if(0 == p_table->item_count)
    {
        printf("\nThe table is empty!");
    }
    else
    {
        for (int idx = 0; idx < p_table->curr_size; idx++)
        {
            if(!p_table->buckets[idx]->head)
            {
                printf("\t%i\t|-[]-|\n", idx);
            }
            else
            {
                printf("\t%i\t", idx);
                print_list(p_table->buckets[idx]);
            }

        }

    }

}


/***
 * For testing purposes only, should be removed before production
 * @return Ignored
 */
int main ()
{
    hashtable_t * my_table = init_hash_table();

    if(!my_table)
    {
        fprintf(stderr, "\n%s: Failed to create a hashtable\n", __func__);
        return -1;
    }

    printf("\nPrinting table before anything is inserted:");
    print_table(my_table);

    char * the_string = "bartholomew";
    char * the_string1 = "gregory";
    char * the_string2 = "Jeffory";
    char * the_string3 = "Mackus";
    char * the_string4 = "gregory";                         //SAME AS String 1
    char * the_string5 = "Maximilian";
    char * the_string6 = "John";
    char * the_string7 = "Melissa";
    char * the_string8 = "Alice";

    printf("\n\nInserting %s\n", the_string);
    hash_table_insert(my_table, the_string);

    printf("\nInserting %s\n", the_string1);
    hash_table_insert(my_table, the_string1);
    printf("\nInserting %s\n", the_string2);
    hash_table_insert(my_table, the_string2);
    printf("\nInserting %s\n", the_string3);
    hash_table_insert(my_table, the_string3);
    printf("\nInserting %s\n", the_string4);
    hash_table_insert(my_table, the_string4);
    printf("\nInserting %s\n", the_string5);
    hash_table_insert(my_table, the_string5);
    printf("\nInserting %s\n", the_string6);
    hash_table_insert(my_table, the_string6);
    printf("\nInserting %s\n", the_string7);
    hash_table_insert(my_table, the_string7);
    printf("\nInserting %s\n", the_string8);
    hash_table_insert(my_table, the_string8);

    print_table(my_table);

    //test removal
    printf("\nRemoving %s\n", the_string2);
    table_rm_item(my_table, the_string2);
    printf("\nRemoving %s\n", the_string5);
    table_rm_item(my_table, the_string5);
    printf("\nRemoving %s\n", the_string7);
    table_rm_item(my_table, the_string7);
    printf("\nRemoving %s\n", the_string1);
    table_rm_item(my_table, the_string1);
    printf("\nRemoving %s\n", the_string2);
    table_rm_item(my_table, the_string2);

    print_table(my_table);

    //Find item
    printf("\nsearching for %s", the_string3);
    bool res3 = table_find_item(my_table, the_string3);
    if(res3)
    {
        printf("\nGood hit!\n");
    }
    else
    {
        printf("\nDoes not exist!\n");
    }

    printf("\nsearching for %s", the_string8);
    bool res8 = table_find_item(my_table, the_string8);
    if(res8)
    {
        printf("\nGood hit!\n");
    }
    else
    {
        printf("\nDoes not exist!\n");
    }

    printf("\nsearching for %s", the_string5);
    bool res5 = table_find_item(my_table, the_string5);
    if(res5)
    {
        printf("\nGood hit!\n");
    }
    else
    {
        printf("\nDoes not exist!\n");
    }

    print_table(my_table);

//    hash_table_clear(my_table);
    hash_table_destroy(&my_table);

    print_table(my_table);

    free(my_table);
    my_table = NULL;
    printf("\nEnding Program\n");
}

//END OF FILE
