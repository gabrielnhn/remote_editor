#include "network.h"

int main()
{
    char packet[69];

    strcpy(packet, "BRUH");

    struct sockaddr_ll address;
    int socket = RawSocketConnection("lo", &address);
    printf("Socket: %d\n", socket);
    int retval;
    while(1)
    {
        retval = sendto(socket, &packet, 69, 0, (const struct sockaddr *) &address,
        (socklen_t) sizeof(address));
        // printf("%d\n", retval); 
    }
}