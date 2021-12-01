#include "network.h"
#include "packet.h"


int main()
{
    packet_t packet;
    char packet_array[BITNSLOTS(PACKET_MAX_BITS)];

    int socket = raw_socket_connection("lo");

    int retval;
    while(1)
    {
        retval = recv(socket, &packet_array, BITNSLOTS(PACKET_MAX_BITS), 0);

        if (retval != -1)
            {
                get_packet_from_array(packet_array, &packet);
                printf("%d\n", *packet.data);
            }
        else
            printf("Nothing.");
    }   
}