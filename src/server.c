#include "common.h"

struct client_data
{
    int client_id;
    int client_sock;
};

struct BlogOperation process_client_op(struct BlogOperation op_received)
{
    struct BlogOperation op_sent;
    op_sent.client_id = op_received.client_id;
    op_sent.operation_type = op_received.operation_type;
    op_sent.server_response = 1;

    switch (op_received.operation_type)
    {
        case NEW_CONNECION:
            printf("client %d connected\n", op_received.client_id);  // In cases where id < 10, is it necessary to add a zero before the id?
            op_sent.client_id = op_received.client_id;
            strcpy(op_sent.topic, "");
            strcpy(op_sent.content, "");
            break;
        
        case NEW_POST:
            printf("new post added in %s by %d\n", op_received.topic, op_received.client_id);
            break;
        
        case SUBSCRIBE:
            printf("client %d subscribed to %s\n", op_received.client_id, op_received.topic);
            strcpy(op_sent.topic, op_received.topic);
            strcpy(op_sent.content, "");
            break;

        case LIST_TOPICS:
            break;
        
        // case DISCONNECT?
    }
    return op_sent;
}

void *client_thread(void *data)
{
    struct client_data *client_data = (struct client_data *)data;
    int client_id = client_data->client_id;
    int client_sock = client_data->client_sock;

    struct BlogOperation op_received, op_sent;
    
    while(1)
    {
        receive_all(client_sock, &op_received, sizeof(struct BlogOperation));

        if(op_received.operation_type == DISCONNECT)
        {
            printf("client %d was disconnected\n", op_received.client_id);
            close(client_sock);
            break;
        }
        // op_sent = process_client_op(op_received);
        
        size_t count_bytes_sent = send(client_sock, &op_sent, sizeof(struct BlogOperation), 0);
        if(count_bytes_sent != sizeof(struct BlogOperation))
            logexit("send");
    }
}   


int main(int argc, char *argv[])
{
    int opt;

    char *ip_version = argv[1];
    char * port = argv[2];

    struct sockaddr_storage storage;

    server_sockaddr_init(ip_version, port, &storage);
    int sockfd = socket(storage.ss_family, SOCK_STREAM, 0);

    if (bind(sockfd, (struct sockaddr *)&storage, sizeof(storage)) != 0)
        logexit("bind");

    if (listen(sockfd, 1) != 0)
        logexit("listen");

    while (1)
    {
        struct sockaddr_storage client_storage;
        struct sockaddr *client_addr = (struct sockaddr *)(&client_storage);
        socklen_t client_addr_len = sizeof(client_storage);

        int client_id = 1;

        int client_sock = accept(sockfd, client_addr, &client_addr_len);
        if (client_sock == -1)
            logexit("accept");

        struct client_data *client_data = malloc(sizeof(struct client_data));
        client_data->client_id = client_id;
        client_data->client_sock = client_sock;
        
        client_id++;  // REVIEW: How to handle client_id when one of the clients disconnect?

        pthread_t thread;
        pthread_create(&thread, NULL, client_thread, (void *)client_data);
        free(client_data);
    }
    return 0;
}