#ifndef SERVER_H
#define SERVER_H

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
    char name[50];
    struct client_data subscribers[NUM_CLIENTS];
    int subs_count;
};

struct server_data
{
    struct client_data clients[NUM_CLIENTS];
    struct topic_data topics[NUM_TOPICS];
    int topics_count;
};

struct thread_info
{
    struct server_data *server_data;
    struct client_data client_data;
};

#endif