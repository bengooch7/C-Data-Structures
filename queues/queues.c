#include "queues.h"


static void
clear_queue (queue_t * p_queue);



/**
 * @brief Create a queue object
 *
 * @param type The type of queue to be created (MSG_Q, CON_Q, WRK_Q)
 * @return void* Returns calloc'd queue with NULL head and tail pointers
 */
void *
create_queue (void)
{
	queue_t * new_q = calloc(1, sizeof(queue_t));
	if (NULL == new_q)
	{
		fprintf(stderr, "\nFailed allocate memory for new queue.\n");
		return NULL;
	}

	new_q->head = NULL;
	new_q->tail = NULL;

	return new_q;

}



/**
 * @brief Adds p_item to specified queue
 *
 * @param p_queue The queue you want to add an p_item to
 * @param p_item    The p_item you want to add to the queue
 * @param type    Type of p_item needing to be placed in the appropriate queue
 */
void
enqueue (queue_t * p_queue, void * p_item)
{
	if ((!p_queue) || (!p_item))
	{
		fprintf(stderr, "\n%s: No queue or job p_item passed in\n", __func__);
		return;
	}


	queue_node_t * p_new_node = calloc(1, sizeof(queue_node_t));
	if(!p_new_node)
	{
		fprintf(stderr, "\n%s: Failed to calloc new node\n", __func__);
		return ;
	}

	//set head of queue to new node if queue is empty
	if (!p_queue->head)
	{
		p_queue->head = p_new_node;
	}

	p_new_node->item = p_item;
	p_new_node->next = NULL;


	if (!p_queue->tail)
	{
		p_queue->tail = p_new_node;
	}
	else
	{
		//redirect tail next pointer to new node
		p_queue->tail->next = p_new_node;

		//set tail to new node
		p_queue->tail = p_new_node;
	}

}




/**
 * @brief Return the first item in the queue
 *
 * @param p_queue the queue of values
 * @return void* the first item in the queue to be returned
 */
void *
dequeue (queue_t * p_queue)
{
	if (!p_queue) //No queue
	{
		fprintf(stderr, "\n%s: No queue passed into function\n", __func__);
		return NULL;
	}

	if ((!p_queue->head) || (!p_queue->head->item)) //Empty queue
	{
		fprintf(stderr, "\n%s: Queue is empty, Nothing to dequeue\n", __func__);
		return NULL;
	}

	void * p_item = p_queue->head->item;

	if (p_queue->head->next)
	{
		queue_node_t * p_old_node = p_queue->head;
		p_queue->head = p_queue->head->next;
		free(p_old_node);
		p_old_node = NULL;
	}
	else
	{
		free(p_queue->head);
		p_queue->head = NULL;
		p_queue->tail = NULL;
	}

	return p_item;
}


/**
 * @brief Empty out the queue of all values
 *          Dequeues and frees all values
 *
 * @param p_queue Queue needing to be emptied
 */
static void
clear_queue (queue_t * p_queue)
{
	if (!p_queue) //No queue
	{
		fprintf(stderr, "\n%s: No queue passed in\n", __func__);
		return;
	}

	if (!p_queue->head) //Empty list
	{
		return;
	}

	while (p_queue->head != NULL)
	{
		void * p_item = dequeue(p_queue);
		free(p_item);
		p_item = NULL;
	}

	p_queue->head = NULL;
	p_queue->tail = NULL;

}



/**
 * @brief If queue is not empty, calls clear_queue() to empty it,
 * then frees the queue
 *
 * @param p_queue The queue to be destroyed
 */
void
destroy_queue (queue_t ** p_queue)
{
	if ((!p_queue) || (!*p_queue)) //No queue
	{
		fprintf(stderr, "\n%s: No Queue to destroy...\n", __func__);
		return;
	}

	if ((*p_queue)->head)
	{
		clear_queue(*p_queue);
	}

	free(*p_queue);
	*p_queue = NULL;
	free(p_queue);
	p_queue = NULL;

}

/**END OF FILE**/
