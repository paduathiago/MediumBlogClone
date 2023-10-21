#include "common.h"
#include "server.h"


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
            // if topic doenst exist, create it
            break;
        
        case SUBSCRIBE:  // Treat the case where the client is already subscribed to the topic
            printf("client %d subscribed to %s\n", op_received.client_id, op_received.topic);
            // if topic doesn't exist, create it
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
    struct thread_info *t_data = (struct thread_info *)data;
    int client_sock = t_data->client_data->sock;

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

void init_server_data(struct server_data *server_data)
{
    int i;
    for(i = 0; i < NUM_CLIENTS; i++)
    {
        server_data->clients[i].id = 0;
        server_data->clients[i].sock = 0;
    }
}

void insert_client(struct server_data *server_data, struct client_data *client_data)
{
    int i;
    for(i = 0; i < NUM_CLIENTS; i++)
    {
        if(server_data->clients[i].id == 0)
        {
            server_data->clients[i].id = i;
            server_data->clients[i].sock = client_data->sock;
            break;
        }
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

    struct server_data *server_data = malloc(sizeof(struct server_data));
    init_server_data(server_data);

    while (1)
    {
        struct sockaddr_storage client_storage;
        struct sockaddr *client_addr = (struct sockaddr *)(&client_storage);
        socklen_t client_addr_len = sizeof(client_storage);

        int client_sock = accept(sockfd, client_addr, &client_addr_len);
        if (client_sock == -1)
            logexit("accept");

        struct client_data *client_data = malloc(sizeof(struct client_data));
        client_data->sock = client_sock;
        insert_client(server_data, client_data);
        
        struct thread_info *t_data = malloc(sizeof(struct thread_info));
        t_data->server_data = server_data;
        t_data->client_data = client_data;

        pthread_t thread;
        pthread_create(&thread, NULL, client_thread, (void *)t_data);
        free(client_data);
    }
    return 0;
}