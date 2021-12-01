#include "network.h"
#include "packet.h"


int main()
{
    packet_t packet;
    char packet_array[PACKET_MAX_BYTES];

    int socket = raw_socket_connection("lo");

    int retval;
    while(1)
    {
        retval = recv(socket, &packet_array, PACKET_MAX_BYTES, 0);

        if (retval != -1)
            {
                get_packet_from_array(packet_array, &packet);
                print_bits(PACKET_MAX_BYTES, packet_array);

                if (packet.parity != get_parity(&packet))
                {
                    printf("%d != %d", packet.parity, get_parity(&packet));
                    printf("size: %d \n", packet.data_size);
                    printf("parity: %d \n", packet.parity);
                    printf("data: %s", packet.data);
                    perror("BRUH");
                    exit(0);
                }
                printf("%d\n", *packet.data);
            }
        else
            printf("Nothing.");
    }   
}