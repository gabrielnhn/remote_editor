#include "network.h"
#include "packet.h"


int main()
{
    packet_t packet;
    char packet_array[PACKET_MAX_BYTES];

    int socket = raw_socket_connection("lo");
    printf("Connected.\n");
    printf("Waiting for instructions from `client`.\n\n");

    int retval;
    while(1)
    {
        retval = recv(socket, &packet_array, PACKET_MAX_BYTES, 0);

        if (retval != -1)
            {
                get_packet_from_array(packet_array, &packet);
                if (packet.parity != get_parity(&packet))
                {
                    printf("%d != %d", packet.parity, get_parity(&packet));
                    perror("BRUH");
                    exit(0);
                }
                printf("%s\n", packet.data);
            }
        else
            printf("Nothing.");
    }   
}