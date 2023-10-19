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