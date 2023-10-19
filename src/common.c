#include "common.h"

void server_sockaddr_init(char *protocol, char * addr, struct sockaddr_storage *storage)
{
    if(strcmp(protocol, "v4") != 0 && strcmp(protocol, "v6") != 0)
    {
        printf("Error! Invalid IP Version\n");
        exit(1);
    }
    if(strcmp(protocol, "v4") == 0)
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(atoi(addr));
        addr4->sin_addr.s_addr = INADDR_ANY;
    }
    else if(strcmp(protocol, "v6") == 0)
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(atoi(addr));
        addr6->sin6_addr = in6addr_any;
    }
}

size_t receive_all(int socket, void *buffer, size_t size)
{
    size_t total_received = 0;

    while (total_received < size) {
        size_t bytes_received = recv(socket, buffer + total_received, size - total_received, 0);

        if (bytes_received <= 0)
        {
            if (bytes_received == 0)  // Connection closed by peer
                return total_received;
            else 
            {
                perror("Error receiving data");
                return -1;
            }
        }
        total_received += bytes_received;
    }

    return total_received;
}