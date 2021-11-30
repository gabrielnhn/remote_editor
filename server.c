#include "network.h"

int main()
{
    char packet[69];
    socklen_t length = (socklen_t) 69;
    struct sockaddr_ll address;
    // int socket = RawSocketConnection("lo", &address);
    int socket = raw_socket_connection("lo");


    int retval;
    while(1)
    {
        // retval = recvfrom(socket, &packet, 69, NU, (struct sockaddr *) &address, &length);
        retval = recv(socket, &packet, 69, 0);
        if (retval != -1)
            printf("%s", packet);
        else
            printf("Nothing.");
    }   
}