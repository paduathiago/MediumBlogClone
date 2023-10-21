#include <stdio.h>
#include <stdlib.h>

#include "common.h"

struct client_data
{
    int id;
    int sock;
};

struct topic_data
{
    char topic_name[100];
    struct client_data topic_subscribers[NUM_CLIENTS];
    int topic_subscribers_count;
};

struct server_data
{
    struct client_data clients[NUM_CLIENTS];
    struct topic_data topics[NUM_TOPICS];
    int client_count;
    // posição disponível para ID
};

struct thread_info
{
    struct server_data *server_data;
    struct client_data *client_data;
};