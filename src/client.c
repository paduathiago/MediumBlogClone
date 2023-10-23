#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"

void process_server_op(struct BlogOperation op_received)
{
    if(op_received.operation_type == NEW_POST)
    {
        printf("new post added in %s by %d\n", op_received.topic, op_received.client_id);
        printf("%s", op_received.content);
    }
    else if(op_received.operation_type == LIST_TOPICS || op_received.operation_type == SUBSCRIBE)
        printf("%s", op_received.content);
}

struct BlogOperation process_input(char command[], const int id)
{
    struct BlogOperation op_sent;
    op_sent.client_id = id;
    op_sent.server_response = 0;
    strcpy(op_sent.topic, "");
    strcpy(op_sent.content, "");

    char *first_word;
    first_word = strtok(command, " \n");
    
    if(strcmp(first_word, "publish") == 0)
    {
        if (strcmp(strtok(NULL, " "), "in"))
        {
            printf("Error! Invalid command\n");
            exit(1);
        }
        op_sent.operation_type = NEW_POST;
        strcpy(op_sent.topic, strtok(NULL, " \n"));
        fgets(op_sent.content, sizeof(op_sent.content), stdin);
    }
    else if(strcmp(first_word, "subscribe") == 0)
    {
        op_sent.operation_type = SUBSCRIBE;
        strcpy(op_sent.topic, strtok(NULL, " \n"));
    }
    else if(strcmp(first_word, "list") == 0)
    {
        if (strcmp(strtok(NULL, " \n"), "topics"))
        {
            printf("Error! Invalid command\n");
            exit(1);
        }
        op_sent.operation_type = LIST_TOPICS;
    }
    else if(strcmp(first_word, "exit") == 0)
    {
        op_sent.operation_type = DISCONNECT;
    }
    /*
    else if(strcmp(first_word, "unsubscribe"))
    {
        op_sent.operation_type = UNSUBSCRIBE;
        strcpy(op_sent.topic, strtok(NULL, " "));
    }
    */
    else
    {
        printf("Error! Invalid command\n");
        exit(1);
    }
    return op_sent;
}

void *send_messages(void *data)
{
    int sockfd = *(int *)data;
    struct BlogOperation op_sent, op_received;
    op_sent.operation_type = NEW_CONNECION;
    op_sent.client_id = 0;
    op_sent.server_response = 0;
    strcpy(op_sent.topic, "");
    strcpy(op_sent.content, "");

    size_t count_bytes_sent = send(sockfd, &op_sent, sizeof(struct BlogOperation), 0);
    if(count_bytes_sent != sizeof(struct BlogOperation))
        logexit("send");

    receive_all(sockfd, &op_received, sizeof(struct BlogOperation)); // recv client's ID
    int myid = op_received.client_id;

    char command[100];
    while(1)
    {
        fgets(command, sizeof(command), stdin);
        op_sent = process_input(command, myid);
        
        size_t count_bytes_sent = send(sockfd, &op_sent, sizeof(struct BlogOperation), 0);
        if(count_bytes_sent != sizeof(struct BlogOperation))
            logexit("send");
        
        if(op_sent.operation_type == DISCONNECT)
        {
            close(sockfd);
            sockfd = -1;
            break;
        }    
    }
    pthread_exit(EXIT_SUCCESS); 
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Error! No IP or Port specified\n");
        return 1;
    }

    struct sockaddr_storage storage;
    parse_addr(argv[1], argv[2], &storage);

    int sockfd = socket(storage.ss_family, SOCK_STREAM, 0);
    if(sockfd == -1) 
        logexit("socket");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(connect(sockfd, addr, sizeof(storage)) != 0)
        logexit("connect");

    pthread_t thread;
    if(pthread_create(&thread, NULL, send_messages, &sockfd) != 0)
        logexit("pthread_create");

    while(sockfd != -1)
    {   
        struct BlogOperation op_received;
        receive_all(sockfd, &op_received, sizeof(struct BlogOperation));
        process_server_op(op_received);
    }
}