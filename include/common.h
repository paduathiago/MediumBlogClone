#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};

// Action Types:
# define NEW_CONNECION 1
# define NEW_POST 2
# define LIST_TOPICS 3
# define SUBSCRIBE 4
# define DISCONNECT 5

void server_sockaddr_init(char *protocol, char * addr, struct sockaddr_storage *storage);
size_t receive_all(int socket, void *buffer, size_t size);

#endif
