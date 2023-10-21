#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"

void process_server_op(struct BlogOperation op_received)
{
    if(op_received.operation_type == NEW_CONNECION)
    {

    }
    else if(op_received.operation_type == NEW_POST)
    {

    }
    else if(op_received.operation_type == LIST_TOPICS)
    {

    }
    else if(op_received.operation_type == SUBSCRIBE)
    {

    }
    else if(op_received.operation_type == DISCONNECT)
    {

    }
}

struct BlogOperation process_input(char command[], const int id)
{
    struct BlogOperation op_sent;
    op_sent.client_id = id;
    op_sent.server_response = 0;
    strcpy(op_sent.topic, "");

    char *first_word;
    first_word = strtok(command, " ");
    
    if(strcmp(first_word, "publish"))
    {

    }
    else if(strcmp(first_word, "subscribe"))
    {

    }
    else if(strcmp(first_word, "list"))
    {

    }
    else if(strcmp(first_word, "disconnect"))
    {

    }
    else
    {
        printf("Error! Invalid command\n");
        exit(1);
    }

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

    while(1)
    {
        struct BlogOperation op_sent;
        op_sent.client_id = myid;
        
        char command[100];

        fgets(command, sizeof(command), stdin);

        // op_sent = process_input(op_topic, int id);
        
        size_t count_bytes_sent = send(sockfd, &op_sent, sizeof(struct BlogOperation), 0);
        if(count_bytes_sent != sizeof(struct BlogOperation))
            logexit("send");
        
        if(op_sent.operation_type == DISCONNECT)
        {
            close(sockfd);
            break;
        }

        receive_all(sockfd, &op_received, sizeof(struct BlogOperation));
        
        process_server_action(op_received);
    }
}