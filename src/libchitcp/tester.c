#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "daemon_api.h"
#include "tester_peer.h"
#include "chitcp/types.h"
#include "chitcp/addr.h"
#include "chitcp/log.h"
#include "chitcp/debug_api.h"
#include "chitcp/socket.h"
#include "chitcp/tester.h"

/* See tester.h */
int chitcp_tester_init(chitcp_tester_t* tester)
{
    tester->server = malloc(sizeof(chitcp_tester_peer_t));
    tester->client = malloc(sizeof(chitcp_tester_peer_t));

    RET_ON_ERROR(pthread_mutex_init(&tester->server->lock_event, NULL),
            CHITCP_ESYNC);

    RET_ON_ERROR(pthread_cond_init(&tester->server->cv_event, NULL),
            CHITCP_ESYNC);

    tester->server->event = TEST_EVENT_NONE;
    tester->server->debug_handler_func = NULL;

    RET_ON_ERROR(pthread_mutex_init(&tester->client->lock_event, NULL),
            CHITCP_ESYNC);

    RET_ON_ERROR(pthread_cond_init(&tester->client->cv_event, NULL),
            CHITCP_ESYNC);

    tester->client->event = TEST_EVENT_NONE;
    tester->client->debug_handler_func = NULL;

    return CHITCP_OK;
}

/* See tester.h */
int chitcp_tester_start(chitcp_tester_t* tester)
{
    /* Create server thread */
    test_peer_thread_args_t *tpta_server = malloc(sizeof(test_peer_thread_args_t));
    tpta_server->tester = tester;
    tpta_server->peer = tester->server;

    if (pthread_create(&tester->server->peer_thread, NULL, chitcp_tester_peer_thread_func, tpta_server) < 0)
    {
        perror("Could not create tester's server thread");
        free(tpta_server);
        return CHITCP_ETHREAD;
    }

    /* Create client thread */
    test_peer_thread_args_t *tpta_client = malloc(sizeof(test_peer_thread_args_t));
    tpta_client->tester = tester;
    tpta_client->peer = tester->client;

    if (pthread_create(&tester->client->peer_thread, NULL, chitcp_tester_peer_thread_func, tpta_client) < 0)
    {
        perror("Could not create tester's client thread");
        free(tpta_client);
        return CHITCP_ETHREAD;
    }

    PROPAGATE_ON_ERROR( chitcp_tester_peer_event(tester->server, TEST_EVENT_INIT) );

    PROPAGATE_ON_ERROR( chitcp_tester_peer_event(tester->client, TEST_EVENT_INIT) );

    return CHITCP_OK;
}


int chitcp_tester_server_set_debug(chitcp_tester_t* tester, debug_event_handler handler, int event_flags)
{
    tester->server->debug_handler_func = handler;
    tester->server->debug_event_flags = event_flags;

    return CHITCP_OK;
}

int chitcp_tester_client_set_debug(chitcp_tester_t* tester, debug_event_handler handler, int event_flags)
{
    tester->client->debug_handler_func = handler;
    tester->client->debug_event_flags = event_flags;

    return CHITCP_OK;
}


/* See tester.h */
int chitcp_tester_server_listen(chitcp_tester_t* tester)
{
    return chitcp_tester_peer_event(tester->server, TEST_EVENT_LISTEN);
}

/* See tester.h */
int chitcp_tester_server_accept(chitcp_tester_t* tester)
{
    return chitcp_tester_peer_event(tester->server, TEST_EVENT_ACCEPT);
}

/* See tester.h */
int chitcp_tester_client_connect(chitcp_tester_t* tester)
{
    return chitcp_tester_peer_event(tester->client, TEST_EVENT_CONNECT);
}

/* See tester.h */
int chitcp_tester_client_close(chitcp_tester_t* tester)
{
    return chitcp_tester_peer_event(tester->server, TEST_EVENT_CLOSE);
}

/* See tester.h */
int chitcp_tester_server_close(chitcp_tester_t* tester)
{
    return chitcp_tester_peer_event(tester->client, TEST_EVENT_CLOSE);
}
