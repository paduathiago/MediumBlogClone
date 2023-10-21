#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"

void process_server_op(struct BlogOperation op_received)
{
    if(op_received.operation_type == )
    {

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

    struct BlogOperation op_received;
    while(1)
    {
        struct BlogOperation op_sent;
        op_sent.type = -1;
        
        char op_topic[50];
        char content[2048];

        fgets(content, sizeof(content), stdin);
        sscanf(content, );
        
        size_t count_bytes_sent = send(sockfd, &op_sent, sizeof(struct BlogOperation), 0);
        if(count_bytes_sent != sizeof(struct BlogOperation))
            logexit("send");
        
        if(op_sent.operation_type == )
        {
            close(sockfd);
            break;
        }

        receive_all(sockfd, &op_received, sizeof(struct BlogOperation));
        
        process_server_action(op_received);
    }
}