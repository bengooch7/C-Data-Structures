#ifndef QUEUES_H
#define QUEUES_H

#include <stdio.h>
#include <stdlib.h>

//NODE
typedef struct queue_node
{
	void * item;
	void * next;
} queue_node_t;

//QUEUE
typedef struct queue
{
	queue_node_t * head;
	queue_node_t * tail;
} queue_t;



//Functions

/**
 * @brief Create a queue object
 *
 * @param type The type of queue to be created (MSG_Q, CON_Q, WRK_Q)
 * @return void* Returns calloc'd queue with NULL head and tail pointers
 */
void *
create_queue (void);


/**
 * @brief Adds p_item to specified queue
 *
 * @param p_queue The queue you want to add an p_item to
 * @param p_item    The p_item you want to add to the queue
 * @param type    Type of p_item needing to be placed in the appropriate queue
 */
void
enqueue (queue_t * p_queue, void * p_item);



/**
 * @brief Return the first item in the queue
 *
 * @param p_queue the queue of values
 * @param type    Type of item needing to be removed from the appropriate queue
 * @return void* the first item in the queue to be returned
 */
void *
dequeue (queue_t * p_queue);



/**
 * @brief If queue is not empty, calls clear_queue() to empty it,
 * then frees the list
 *
 * @param p_queue The queue to be destroyed
 */
void
destroy_queue (queue_t ** p_queue);



#endif /* QUEUES_H */

/**END OF FILE**/
