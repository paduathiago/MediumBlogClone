#include "common.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>

sem_t mutex;

void init_client_array(struct client_data data[], int size)
{
    for(int i = 0; i < size; i++)
    {
        data[i].id = 0;
        data[i].sock = 0;
    }
}

/*
Insert client in the first available position in the array.
Return the client's id if successful, -1 otherwise.
*/
int insert_client(struct client_data *client_data, struct client_data clients[])
{
    sem_wait(&mutex);
    for(int i = 0; i < NUM_CLIENTS; i++)
    {
        if(clients[i].id == 0)
        {
            clients[i] = *client_data;
            sem_post(&mutex);
            return i + 1;
        }
    }
    sem_post(&mutex);
    return -1;
}

struct topic_data create_topic(char *topic_name)
{
    struct topic_data new_topic;
    strcpy(new_topic.name, topic_name);
    init_client_array(new_topic.subscribers, NUM_CLIENTS);
    new_topic.subs_count = 0;
    return new_topic;
}

void remove_client_from_topic(int id, struct topic_data *topic_data)
{
    sem_wait(&mutex);
    for(int i = 0; i < NUM_CLIENTS; i++)
    {
        if(topic_data->subscribers[i].id == id )
        {
            topic_data->subscribers[i].id = 0;
            topic_data->subs_count--;
            break;
        }
    }
    sem_post(&mutex);
}

struct BlogOperation process_client_op(struct BlogOperation op_received, struct server_data *s_data, struct client_data *c_data)
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
            op_sent.client_id = c_data->id;
            if(op_sent.client_id < 10)
                printf("client 0%d connected\n", op_sent.client_id);
            else
                printf("client %d connected\n", op_sent.client_id);
            break;
        
        case NEW_POST:
            strcpy(op_sent.topic, op_received.topic);
            strcpy(op_sent.content, op_received.content);
            if(op_sent.client_id < 10)
                printf("new post added in %s by 0%d\n", op_received.topic, op_received.client_id);
            else
                printf("new post added in %s by %d\n", op_received.topic, op_received.client_id);
            
            int found_topic = 0;
            for(int i = 0; i < s_data->topics_count; i++)
            {
                if(strcmp(s_data->topics[i].name, op_received.topic) == 0)
                {
                    for(int j = 0; j < NUM_CLIENTS; j++)  // Send new post to all subscribers
                    {
                        if(s_data->topics[i].subscribers[j].id != 0)
                            send(s_data->topics[i].subscribers[j].sock, &op_sent, sizeof(struct BlogOperation), 0);
                    }
                    found_topic = 1;
                    break;
                }
            }
            if(!found_topic)  // if topic doesn't exist, create it
            {
                struct topic_data new_topic = create_topic(op_received.topic);
                sem_wait(&mutex);
                s_data->topics[s_data->topics_count] = new_topic;
                s_data->topics_count++;
                sem_post(&mutex);
            }
            break;
        
        case SUBSCRIBE:
            found_topic = 0;
            for(int i = 0; i < s_data->topics_count; i++)
            {
                if(strcmp(s_data->topics[i].name, op_received.topic) == 0)
                {
                    found_topic = 1;
                    for(int j = 0; j < NUM_CLIENTS; j++)
                    {
                        if(s_data->topics[i].subscribers[j].id == c_data->id)
                        {
                            strcpy(op_sent.content, "error: already subscribed\n");
                            return op_sent;
                        }
                    }
                    insert_client(c_data, s_data->topics[i].subscribers);
                    s_data->topics[i].subs_count++;
                    break;
                }
            }
            if(!found_topic)  // if topic doesn't exist, create it and add client to it
            {
                struct topic_data new_topic = create_topic(op_received.topic);
                insert_client(c_data, new_topic.subscribers);
                new_topic.subs_count++;
                sem_wait(&mutex);
                s_data->topics[s_data->topics_count] = new_topic;
                s_data->topics_count++;
                sem_post(&mutex);
            }
            strcpy(op_sent.topic, op_received.topic);
            if(op_sent.client_id < 10)
                printf("client 0%d subscribed to %s\n", op_received.client_id, op_received.topic);
            else
                printf("client %d subscribed to %s\n", op_received.client_id, op_received.topic);
            break;
        
        case UNSUBSCRIBE:
            found_topic = 0;
            for(int i = 0; i < s_data->topics_count; i++)
            {
                if(strcmp(s_data->topics[i].name, op_received.topic) == 0)
                {
                    found_topic = 1;
                    remove_client_from_topic(op_received.client_id, &(s_data->topics[i]));
                    if(op_sent.client_id < 10)
                        printf("client 0%d unsubscribed to %s\n", op_received.client_id, op_received.topic);
                    else
                        printf("client %d unsubscribed to %s\n", op_received.client_id, op_received.topic);
                    break;
                }
            }
            if(!found_topic)
            {
                strcpy(op_sent.content, "error: topic doesn't exist\n");
                return op_sent;
            }
            strcpy(op_sent.topic, op_received.topic);
            break;

        case LIST_TOPICS:
            if(s_data->topics_count == 0)
                strcpy(op_sent.content, "no topics available\n");
            else
            {
                for(int i = 0; i < s_data->topics_count; i++)
                {
                    if(i != s_data->topics_count - 1)
                    {
                        strcat(op_sent.content, s_data->topics[i].name);
                        strcat(op_sent.content, "; ");
                    }
                    else
                    {
                        strcat(op_sent.content, s_data->topics[i].name);
                        strcat(op_sent.content, "\n");
                    }
                }
            }
            break;
        
        case DISCONNECT:
            s_data->clients[op_received.client_id - 1].id = 0;

            for(int i = 0; i < s_data->topics_count; i++)  // Remove client from all topics
                remove_client_from_topic(op_received.client_id, &(s_data->topics[i]));

            op_sent.operation_type = DISCONNECT;
            if(op_sent.client_id < 10)
                printf("client 0%d was disconnected\n", op_received.client_id);
            else
                printf("client %d was disconnected\n", op_received.client_id);
            break;
    }
    return op_sent;
}

void *client_thread(void *data)
{
    struct thread_info *t_data = (struct thread_info *)data;
    int client_sock = t_data->client_data.sock;

    struct BlogOperation op_received, op_sent;
    while(1)
    {
        receive_all(client_sock, &op_received, sizeof(struct BlogOperation));

        op_sent = process_client_op(op_received, t_data->server_data, &(t_data->client_data));
        
        if (op_received.operation_type == NEW_POST)
            continue;
        
        size_t count_bytes_sent = send(client_sock, &op_sent, sizeof(struct BlogOperation), 0);
        if(count_bytes_sent != sizeof(struct BlogOperation))
            logexit("send");
        
        if(op_received.operation_type == DISCONNECT)
        {
            close(client_sock);
            break;
        }
    }
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    sem_init(&mutex, 0, 1);
    char *ip_version = argv[1];
    char * port = argv[2];

    struct sockaddr_storage storage;

    server_sockaddr_init(ip_version, port, &storage);
    int sockfd = socket(storage.ss_family, SOCK_STREAM, 0);
    int enable = 1;
        if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) 
            logexit("setsockopt");

    if (bind(sockfd, (struct sockaddr *)&storage, sizeof(storage)) != 0)
        logexit("bind");

    if (listen(sockfd, 1) != 0)
        logexit("listen");

    struct server_data server_data;
    server_data.topics_count = 0;
    init_client_array(server_data.clients, NUM_CLIENTS);

    struct thread_info thread_info[10];

    while (1)
    {
        struct sockaddr_storage client_storage;
        struct sockaddr *client_addr = (struct sockaddr *)(&client_storage);
        socklen_t client_addr_len = sizeof(client_storage);

        int client_sock = accept(sockfd, client_addr, &client_addr_len);
        if (client_sock == -1)
            logexit("accept");

        struct client_data c_data;
        c_data.sock = client_sock;
        int id = insert_client(&c_data, server_data.clients);
        c_data.id = id;
        server_data.clients[id - 1].id = id;
        
        struct thread_info t_data;
        t_data.server_data = &server_data;
        t_data.client_data = c_data;
        thread_info[id - 1] = t_data;

        pthread_t thread;
        if(pthread_create(&thread, NULL, client_thread, &thread_info[id -1]) != 0)
            logexit("pthread_create");
    }
    return 0;
}
