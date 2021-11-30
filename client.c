#include "network.h"

int main()
{
    char packet[69];

    strcpy(packet, "BRUH");

    struct sockaddr_ll address;
    // int socket = RawSocketConnection("lo", &address);
    int socket = raw_socket_connection("lo");

    int retval;
    while(1)
    {
        retval = send(socket, &packet, 69, 0);
        // retval = sendto(socket, &packet, 69, 0, (const struct sockaddr *) &address,
        // (socklen_t) sizeof(address));
        // printf("%d\n", retval); 
    }
}