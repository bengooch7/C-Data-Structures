#ifndef SERVER_H
#define SERVER_H

#include "../threadpool/threadpool.h"
#include "../queues/queues.h"
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <inttypes.h>
#include <byteswap.h>
#include <string.h>

#define DEFAULT_TCP_PORT  44473
#define DEFAULT_UDP_PORT  4545
#define TIMEOUT           10000


void *
start_server(void * p_pool);

void
handle_new_connection(threadpool_t * p_tpool, int connfd);

#endif /* SERVER_H */

/**END OF FILE**/
