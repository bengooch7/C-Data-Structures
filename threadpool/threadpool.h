#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../queues/queues.h"

//NOTE: This needs to be reworked to fit whatever the project requires as far as queues go.
//NOTE: The #include "queues.h" will also need to be added back in for this to access queue stuff
typedef struct que
{
    //Work queue
    queue_t        * task_q;            //Pointer to queue of incomplete tasks
    pthread_mutex_t  task_lock;         //Mutex for thread access to work queue
    pthread_cond_t   task_wake_up;      //A new task is in queue

    //Completed task queue
    queue_t        * complete_task_q;       //Pointer to queue of finished jobs
    pthread_mutex_t  complete_task_lock;    //Mutex for thread access to work q
    pthread_cond_t   complete_task_wake_up; //A completed job is in queue

    //Message buffer
    queue_t         * incoming;          //Pointer to queue of jobs
    pthread_mutex_t   inco_lock;         //Mutex for thread access to work queue
    pthread_cond_t    inco_wake_up;      //A new job is in queue

    //Connection queue
    queue_t        * conn_queue;        //Queue of connections to be grabbed
    pthread_mutex_t  conn_lock;         //Mutex for thread access to connections
    pthread_cond_t   conn_wake_up;      //A new connection is in the array
    struct pollfd  * pfds;              //Checks for signal on file descriptors
    int              nfds;              //The number of file descriptors
}que_t;

typedef struct threadpool
{
    pthread_t     * thr_id;            //Array of thread IDs
    que_t         * que;               //All queue interaction
    size_t          working_cnt;       //Num of threads working/waiting for work
    size_t          thread_limit;      //Number of total allowed threads in pool
    _Atomic bool    stop;              //Signal to shut down server

}threadpool_t;







void *
worker_gonna_work(void * p_pool);    //worker_threads.c

threadpool_t *
threadpool_new(size_t thread_count);

bool
threadpool_delete(threadpool_t ** pp_tpool);

bool
create_worker_thread(threadpool_t * p_tpool, void * starting_func);

bool
create_main_thread(threadpool_t * p_tpool, void * starting_func);


#endif /* THREADPOOL_H */

/**END OF FILE**/
