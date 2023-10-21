#include "common.h"
#include "server.h"

struct BlogOperation process_client_op(struct BlogOperation op_received, struct server_data *s_data)
{
    struct BlogOperation op_sent;
    op_sent.client_id = op_received.client_id;
    op_sent.operation_type = op_received.operation_type;
    op_sent.server_response = 1;
    strcpy(op_sent.topic, "");
    strcpy(op_sent.content, "");

    switch (op_received.operation_type)
    {
        case NEW_CONNECION:
            printf("client %d connected\n", op_received.client_id);  // In cases where id < 10, is it necessary to add a zero before the id?
            op_sent.client_id = op_received.client_id;
            break;
        
        case NEW_POST:
            printf("new post added in %s by %d\n", op_received.topic, op_received.client_id);
            // if topic doenst exist, create it
            break;
        
        case SUBSCRIBE:  // Treat the case where the client is already subscribed to the topic
            printf("client %d subscribed to %s\n", op_received.client_id, op_received.topic);
            // if topic doesn't exist, create it
            strcpy(op_sent.topic, op_received.topic);
            break;

        case LIST_TOPICS:
            if(s_data->topics_count == 0)
                strcpy(op_sent.content, "None");
            else
            {
                for(int i = 0; i < s_data->topics_count; i++)
                sprintf(op_sent.content, "%s; ", s_data->topics[i].topic_name);
            }
            break;
        
        case DISCONNECT:
            printf("client %d was disconnected\n", op_received.client_id);
            
            s_data->clients[op_received.client_id].id = 0;
            s_data->clients[op_received.client_id].sock = 0;
            
            for(int i = 0; i < s_data->topics_count; i++)
            {
                for(int j = 0; j < s_data->topics[i].subs_count; j++)
                {
                    if(s_data->topics[i].subscribers[j].id == op_received.client_id)
                    {
                        s_data->topics[i].subscribers[j].id = 0;
                        s_data->topics[i].subs_count--;
                    }
                }
            }
            
            break;
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

void init_client_array(struct client_data *data, int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        data[i].id = 0;
        data[i].sock = 0;
    }
}

void insert_client(struct client_data *client_data, struct client_data *clients)
{
    int i;
    for(i = 0; i < NUM_CLIENTS; i++)
    {
        if(clients[i].id == 0)
        {
            client_data->id = i;
            clients[i].id = client_data->id;
            clients[i].sock = client_data->sock;
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
    server_data->topics_count = 0;
    init_client_array(server_data->clients, NUM_CLIENTS);

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
        insert_client(client_data, server_data->clients);
        
        struct thread_info *t_data = malloc(sizeof(struct thread_info));
        t_data->server_data = server_data;
        t_data->client_data = client_data;

        pthread_t thread;
        pthread_create(&thread, NULL, client_thread, (void *)t_data);
        free(client_data);
    }
    return 0;
}