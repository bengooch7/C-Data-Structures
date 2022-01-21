#include "server.h"

#define SA          struct sockaddr
#define MAX_BUFF    4096

static void
cleanup_worker_connections(threadpool_t * p_tpool);

static int
setup_tcp(void);

static void
listen_for_traffic(threadpool_t * p_tpool, int sockfd);

static void
check_tcp_listener(threadpool_t * p_tpool, int sockfd);

static void
check_worker_fds(threadpool_t * p_tpool);

static void
handle_new_message(threadpool_t * p_tpool, char * incoming_msg,
                   ssize_t length, int sender);

static void
get_initial_message(threadpool_t * p_tpool, int connfd);





/**
 * @brief Start the server for all communications
 *
 * @param p_main_pool Threadpool hosting threads
 */
void *
start_server(void * p_main_pool)
{
    if (!p_main_pool)
    {
        fprintf(stderr, "\nNothing passed to %s\n", __func__);
        return NULL;
    }

    threadpool_t * p_tpool = p_main_pool;

//Build socket for TCP listener
    int sockfd = setup_tcp();
    if(sockfd < 0)
    {
        fprintf(stderr, "\n[\033[0;31m FAIL\033[0m ] Start Server - TCP");
        goto CLEANUP_TCP;
    }

    printf("\n[\033[0;32m  OK \033[0m ] Ready Server - TCP");

    //Set up polling of connected workers/submitter
    p_tpool->que->pfds[p_tpool->que->nfds].fd     = sockfd;
    p_tpool->que->pfds[p_tpool->que->nfds].events = POLLIN;
    p_tpool->que->nfds++;

    if (!p_tpool->stop)
    {
        printf("\n[\033[0;32m  OK \033[0m ] Server running\n");
        listen_for_traffic(p_tpool, sockfd);
    }


    fprintf(stderr, "\nDEBUG: Exited listen_for_traffic()\n");


    cleanup_worker_connections(p_tpool);

    CLEANUP_TCP:
    close(sockfd);
    p_tpool->stop = true;
    return NULL;
}

/**
 * @brief Set up TCP socket for listening
 *
 * @return int Returns file descriptor for connection or -1 if error
 */
static int
setup_tcp(void)
{
    printf("\n[\033[0;33m WAIT\033[0m ] Starting Server - TCP\n");
    struct sockaddr_in servaddr;

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > sockfd)
    {
        perror("\nFailed to create socket!\n");
        return -1;
    }
    else
    {
        printf("[\033[0;32m  OK \033[0m ]\t Create socket\n");
    }

    memset(&servaddr, -1, sizeof(servaddr));

    //assign IP and port
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(DEFAULT_TCP_PORT);
    int enable               = 1;


    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                   &enable, sizeof(enable)))
    {
        perror("setsockopt: ");
        return -1;
    }

    //Binding newly created socket to given IP and verification
    if (bind(sockfd, (SA*)&servaddr, sizeof(servaddr)))
    {
        fprintf(stderr,"[\033[1;31m FAILED\033[0m ]\t Bind TCP Socket\n");
        return -1;
    }
    else
    {
        uint16_t prt_num = ntohs(servaddr.sin_port);
        printf("[\033[0;32m  OK \033[0m ]\t Bind TCP socket - ");
        printf("Listening on port %"PRIu16"\n", prt_num);
    }

    // Now server is ready to listen
    if ((listen(sockfd, 10)) != 0) {
        printf("[\033[1;31m FAILED\033[0m ]\t Start server listening");
        return -1;
    }
    else
    {
        printf("[\033[0;32m  OK \033[0m ]\t Start server listening");
    }

    return sockfd;

}


static void
listen_for_traffic(threadpool_t * p_tpool, int sockfd)
{
    if (!p_tpool)
    {
        fprintf(stderr, "\n%s: No threadpool passed to function\n", __func__);
        return;
    }

    if (sockfd < 0)
    {
        fprintf(stderr, "\n%s: Invalid FD passed to function\n", __func__);
        return;
    }

    while (!p_tpool->stop) //Gather new client and add them to conn array
    {
        int num_events = poll(p_tpool->que->pfds, p_tpool->que->nfds,
                              TIMEOUT);

        if (num_events == 0) //No traffic on server
        {
            continue;
        }
        else   //One of the sockets received some traffic, let's see which one
        {
            //TCP listener polled, new client attempting to connect
            int tcp_pollin_happened =
                    p_tpool->que->pfds[0].revents & POLLIN;

            if(0 < tcp_pollin_happened)
            {
                check_tcp_listener(p_tpool, sockfd);
            }

            check_worker_fds(p_tpool);
        }//END poll() has results

    } //END Gather client connections and add them to connection array

}//END listen_for_traffic()


/**
 * @brief Check main TCP listener socket for traffic
 * @param p_tpool Threadpool context containing socket for listener
 * @param sockfd File descriptor for this server's TCP listener socket
 */
static void
check_tcp_listener(threadpool_t * p_tpool, int sockfd)
{
    if (!p_tpool)
    {
        fprintf(stderr, "\n%s: No threadpool passed to function\n", __func__);
        return;
    }

    if (sockfd < 0)
    {
        fprintf(stderr, "\n%s: Invalid FD passed to function\n", __func__);
        return;
    }

    socklen_t          len    = 0;
    struct sockaddr_in cli    = {0};

    len = sizeof(cli);

    // Accept connection from client
    int connfd = accept(sockfd, (SA*)&cli, &len);

    get_initial_message(p_tpool, connfd);
}


static void
check_worker_fds(threadpool_t * p_tpool)
{
    if(!p_tpool->que->conn_queue)
    {
        fprintf(stderr, "\n%s: No threadpool passed to function\n", __func__);
        return;
    }
    for(int idx = 2; idx < p_tpool->que->nfds; idx++)
    {
        int pollin_happened = p_tpool->que->pfds[idx].revents & POLLIN;

        if (0 < pollin_happened)
        {
            //Read to see what kind of connection it is
            char incoming_msg [MAX_BUFF + 1] = {0};

            ssize_t num_bytes_read = recv(p_tpool->que->pfds[idx].fd,
                                          &incoming_msg, MAX_BUFF, 0);

            if(0 > num_bytes_read)
            {
                perror("\nFailed to recv: ");
                break;
            }

            if(0 == num_bytes_read)
            {
                continue;
            }

            int sender = p_tpool->que->pfds[idx].fd;

            handle_new_message(p_tpool, incoming_msg, num_bytes_read, sender);

        }//end listener_pollin_happened

    }//end loop through FDs for polls

}//END check_worker_fds()


static void
handle_new_message(threadpool_t * p_tpool, char * incoming_msg,
                   ssize_t length, int sender)
{
    if (!p_tpool)
    {
        fprintf(stderr, "\n%s: No threadpool passed to function\n", __func__);
        return;
    }

    printf("\nA MESSAGE WAS RECEIVED BY THE SERVER\n");

        //NOTE: This will likely all need to be changed depending on the requirements of the program.---------------------------

}


void
handle_new_connection(threadpool_t * p_tpool, int connfd)
{
    if (!p_tpool)
    {
        fprintf(stderr, "\n%s: No threadpool passed to function\n", __func__);
        return;
    }

    if (connfd < 0)
    {
        printf("[\033[1;31m FAILED\033[0m ] Accept new worker\n");
        printf("/submitter");
        return;
    }
    else
    {
        printf("[\033[0;32m  OK \033[0m ] Accept new worker");
        printf("/submitter");
    }

    //Lock connection list to make changes
    if (pthread_mutex_lock(&p_tpool->que->conn_lock))
    {
        perror("FAILED: ");
        return;
    }

    //NOTE: Code to add new connection to connection queue needs to be written here, assuming the program requires such a thing
    void * new_conn = 0;

    //Add new connection to connection array for use by threads
    enqueue(p_tpool->que->conn_queue, new_conn);

    p_tpool->que->pfds[p_tpool->que->nfds].fd = connfd;
    p_tpool->que->pfds[p_tpool->que->nfds].events = POLLIN;
    p_tpool->que->nfds++;

    pthread_mutex_unlock(&p_tpool->que->conn_lock);
    pthread_cond_broadcast(&p_tpool->que->conn_wake_up);
}

static void
get_initial_message(threadpool_t * p_tpool, int connfd)
{
    if (!p_tpool)
    {
        fprintf(stderr, "\n%s: No threadpool passed to function\n", __func__);
        return;
    }

    if(connfd < 0)
    {
        fprintf(stderr, "\n%s: Invalid connfd\n", __func__);
        return;
    }

    char incoming_msg [MAX_BUFF + 1] = {0};

    ssize_t num_bytes_read = recv(connfd, &incoming_msg, MAX_BUFF, 0);

    if(0 > num_bytes_read)
    {
        perror("\nFailed to recv: ");
        return;
    }

    if(0 == num_bytes_read)
    {
        return;
    }

    handle_new_message(p_tpool, incoming_msg, num_bytes_read, connfd);
}


/**
 * @brief Cycles through all open connections and closes them
 *
 * @param p_tpool threadpool context containing the connection list
 * @return bool true for success, false for failure
 */
static void
cleanup_worker_connections(threadpool_t * p_tpool)
{
    if (!p_tpool)
    {
        fprintf(stderr, "\n%s: No threadpool passed to function\n", __func__);
        return;
    }

    pthread_mutex_lock(&p_tpool->que->conn_lock);

    destroy_queue(&(p_tpool->que->conn_queue));

    pthread_mutex_unlock(&p_tpool->que->conn_lock);
    pthread_cond_broadcast(&p_tpool->que->conn_wake_up);

}

/** END OF FILE **/
