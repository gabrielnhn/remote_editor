#include "network.h"
#include "packet.h"


int main()
{
    packet_t packet;
    char packet_array[BITNSLOTS(PACKET_MAX)];

    int socket = raw_socket_connection("lo");

    int retval;
    while(1)
    {
        retval = recv(socket, &packet_array, BITNSLOTS(PACKET_MAX) - 2, 0);

        if (retval != -1)
            // printf("%s", packet_array);
            printf("A");
        else
            printf("Nothing.");
    }   
}