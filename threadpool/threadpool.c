#include "threadpool.h"

#define MAX_WORKERS   100

/* Function declarations */
static bool
setup_mutex_and_conds(threadpool_t * p_tpool);


/**
 * @brief Creates a new threadpool
 *
 * On error set err_code = 1
 *
 * @param[in] thread_count Max number of threads for this pool
 * @return threadpool_t* A new threadpool context or NULL if error
 */
threadpool_t * threadpool_new(size_t thread_count)
{
    if (0 >= thread_count)
    {
        fprintf(stderr, "\nProposed thread count was 0 or less");
        return NULL;
    }

    //Make pool
    threadpool_t * p_tpool = calloc(1, sizeof(threadpool_t));
    if (NULL == p_tpool)
    {
        fprintf(stderr, "\nCalloc failed for new threadpool");
        return NULL;
    }

    //Make queue struct to hold all the queues
    p_tpool->que = calloc(1, sizeof(que_t));
    if (NULL == p_tpool->que)
    {
        fprintf(stderr, "\nCalloc failed for new threadpool que holder");
        goto CLEANUP_FROM_POOL;
    }

    //Initialization
    bool m_and_c_res = setup_mutex_and_conds(p_tpool);
    if(!m_and_c_res)
    {
        fprintf(stderr, "\nFailed to initialize mutex/conds\n");
        goto CLEANUP_FROM_POOL;
    }

    p_tpool->thread_limit         = thread_count;
    p_tpool->stop                 = false;
    //Create a queue for each required queue in the threadpool
    p_tpool->que->task_q          = create_queue();
    p_tpool->que->complete_task_q = create_queue();
    p_tpool->que->incoming        = create_queue();
    p_tpool->que->conn_queue      = create_queue();

    if((!p_tpool->que->task_q) ||
       (!p_tpool->que->incoming) ||
       (!p_tpool->que->complete_task_q) ||
       (!p_tpool->que->conn_queue))
    {
        goto CLEANUP_FROM_QUEUES;
    }

    p_tpool->thr_id = calloc(thread_count, sizeof(pthread_t));
    if (NULL == p_tpool->thr_id)
    {
        fprintf(stderr, "\nFailed to calloc for thread IDs");
        goto CLEANUP_FROM_THREADS;
    }

    p_tpool->que->pfds = calloc(MAX_WORKERS, sizeof(struct pollfd));
    if(!p_tpool->que->pfds)
    {
        fprintf(stderr, "\n%s: Failed to calloc for pfds\n", __func__);
        goto CLEANUP_FROM_THREADS;
    }

    p_tpool->que->nfds      = 0;
    p_tpool->working_cnt    = 0;




    return p_tpool;

    CLEANUP_FROM_THREADS:
    free(p_tpool->thr_id);
    p_tpool->thr_id = NULL;

    CLEANUP_FROM_QUEUES:
    free(p_tpool->que->task_q);
    p_tpool->que->task_q = NULL;
    free(p_tpool->que->incoming);
    p_tpool->que->incoming = NULL;
    free(p_tpool->que->conn_queue);
    p_tpool->que->conn_queue = NULL;
    free(p_tpool->que->complete_task_q);
    p_tpool->que->complete_task_q = NULL;

    CLEANUP_FROM_POOL:
    free(p_tpool->que);
    p_tpool->que = NULL;
    free(p_tpool);
    p_tpool = NULL;

    return NULL;

}


/**
 * @brief Spins up the threads that run the server and handler code
 *
 * @param p_tpool the threadpool context in which these threads should exist
 * @param p_starting_func The function that this thread will run when it starts
 * @return true   Successfully created threads
 * @return false  Failed to create one or both threads
 */
bool
create_main_thread (threadpool_t * p_tpool, void * p_starting_func)
{
    if((!p_tpool) ||(!p_starting_func))
    {
        fprintf(stderr, "\n%s: No threadpool passed into function\n", __func__);
        return false;
    }

    int res = pthread_create(
            &p_tpool->thr_id[p_tpool->working_cnt],
            NULL,
            p_starting_func,
            p_tpool);

    if(0 != res)
    {
        return false;
    }
    else
    {
        p_tpool->working_cnt++;
    }

    return true;

}


/**
 * @brief Create a worker threads
 *
 * @param p_tpool threadpool context to hold the threads
 * @return true   successfully created thread
 * @return false  failed to create thread
 */
bool
create_worker_thread (threadpool_t * p_tpool, void * starting_func)
{
    if((!p_tpool) || (!starting_func))
    {
        fprintf(stderr,
            "\n%s: No threadpool or starting function passed into function\n",
            __func__);

        return false;
    }

    // Double thread_limit if more space is required for new threads
    if(p_tpool->working_cnt == p_tpool->thread_limit)
    {
        p_tpool->thread_limit = p_tpool->thread_limit * 2;

        p_tpool->thr_id =
                realloc(p_tpool->thr_id, p_tpool->thread_limit);
        if(!p_tpool->thr_id)
        {
            fprintf(stderr, "\n%s: Realloc failed for threadpool\n", __func__);
            return false;
        }

    }

    bool res = pthread_create(
            &p_tpool->thr_id[p_tpool->working_cnt],
            NULL,
            starting_func,
            p_tpool);
    if(0 != res)
    {
        return false;
    }
    else
    {
        p_tpool->working_cnt++;
    }

    return true;
}


/**
 * @brief Set the up mutex and conds objects
 *
 * @param p_tpool the threadpool context to maintain the mutex/conds
 * @return true   Everything setup properly
 * @return false  Something went wrong with the setup
 */
static bool
setup_mutex_and_conds(threadpool_t * p_tpool)
{
    //static function, no chance of p_tpool not being passed in

    if (pthread_mutex_init(&p_tpool->que->task_lock, NULL))
    {
        fprintf(stderr, "\nFailed to initialize task_lock");
        return false;
    }

    if (pthread_cond_init(&p_tpool->que->task_wake_up, NULL))
    {
        fprintf(stderr, "\nFailed to initialize task_wake_up");
        return false;
    }

    if (pthread_mutex_init(&p_tpool->que->complete_task_lock, NULL))
    {
        fprintf(stderr, "\nFailed to initialize task_lock");
        return false;
    }

    if (pthread_cond_init(&p_tpool->que->complete_task_wake_up, NULL))
    {
        fprintf(stderr, "\nFailed to initialize task_wake_up");
        return false;
    }

    if (pthread_mutex_init(&p_tpool->que->conn_lock, NULL))
    {
        fprintf(stderr, "\nFailed to initialize conn_lock");
        return false;
    }

    if (pthread_cond_init(&p_tpool->que->conn_wake_up, NULL))
    {

        fprintf(stderr, "\nFailed to initialize conn_wake_up");
        return false;
    }

    if (pthread_mutex_init(&p_tpool->que->inco_lock, NULL))
    {
        fprintf(stderr, "\nFailed to initialize inco_lock");
        return false;
    }

    if (pthread_cond_init(&p_tpool->que->inco_wake_up, NULL))
    {

        fprintf(stderr, "\nFailed to initialize inco_wake_up");
        return false;
    }

    return true;

}



/**
 * @brief Sets the shutdown flag and processes the remainder of the queues. This
 * is so the worker threads that are responsible for cleaning up resources are
 * afforded the opportunity.
 *
 * @param[in] pool A threadpool context
 * @return int 0 on success
 */
static int
threadpool_shutdown(threadpool_t* p_tpool)
{
    if (!p_tpool)
    {
        fprintf(stderr, "\n%s: No threadpool passed into function\n", __func__);
        return -1;
    }

    p_tpool->stop = true;
    pthread_cond_broadcast(&p_tpool->que->task_wake_up);
    pthread_cond_broadcast(&p_tpool->que->complete_task_wake_up);
    pthread_cond_broadcast(&p_tpool->que->conn_wake_up);
    pthread_cond_broadcast(&p_tpool->que->inco_wake_up);

    for (size_t idx = 0; idx < p_tpool->thread_limit; idx++)
    {
        //Wait for all threads in the pool
        pthread_join(p_tpool->thr_id[idx], NULL);

        p_tpool->working_cnt--;


        printf("\nDEBUG: waiting on %lu threads to return\n",
               p_tpool->working_cnt);
    }

    return 0;
}



/**
 * @brief Waits for all queued jobs to finish and then kills the threadpool.
 *
 * @param pp_tpool A pointer to threadpool context.
 * @return bool True if successful, false if not.
 */
bool
threadpool_delete(threadpool_t ** pp_tpool)
{
    if ((!pp_tpool) || (!*pp_tpool))
    {
        fprintf(stderr, "\nNo threadpool passed to threadpool_delete.\n");
        return false;
    }
    if (!(*pp_tpool)->stop)
    {
        threadpool_shutdown(*pp_tpool);
    }

    free((*pp_tpool)->thr_id);
    (*pp_tpool)->thr_id = NULL;

    free((*pp_tpool)->que->task_q);
    (*pp_tpool)->que->task_q = NULL;

    free((*pp_tpool)->que->complete_task_q);
    (*pp_tpool)->que->complete_task_q = NULL;

    free((*pp_tpool)->que->incoming);
    (*pp_tpool)->que->incoming = NULL;

    free((*pp_tpool)->que->conn_queue);
    (*pp_tpool)->que->conn_queue = NULL;

    free((*pp_tpool)->que->pfds);
    (*pp_tpool)->que->pfds = NULL;


    pthread_mutex_destroy(&(*pp_tpool)->que->task_lock);
    pthread_cond_destroy(&(*pp_tpool)->que->task_wake_up);
    pthread_mutex_destroy(&(*pp_tpool)->que->complete_task_lock);
    pthread_cond_destroy(&(*pp_tpool)->que->complete_task_wake_up);
    pthread_mutex_destroy(&(*pp_tpool)->que->conn_lock);
    pthread_cond_destroy(&(*pp_tpool)->que->conn_wake_up);
    pthread_mutex_destroy(&(*pp_tpool)->que->inco_lock);
    pthread_cond_destroy(&(*pp_tpool)->que->inco_wake_up);


    free((*pp_tpool)->que);
    (*pp_tpool)->que = NULL;

    free((*pp_tpool));
    (*pp_tpool) = NULL;

    return true;
}
