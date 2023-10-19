#include "common.h"

struct client_data
{
    int client_id;
    int client_sock;
};

//struct BlogOperation process_client_op();

void *client_thread(void *data)
{
    struct client_data *client_data = (struct client_data *)data;
    int client_id = client_data->client_id;
    int client_sock = client_data->client_sock;

    struct BlogOperation op_received, op_sent;
    
    printf("client %d connected\n", client_id);
    
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
        
        client_id++;

        pthread_t thread;
        pthread_create(&thread, NULL, client_thread, (void *)client_data);
        free(client_data);
    }
    return 0;
}