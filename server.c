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
            print_bits(PACKET_MAX_BYTES, packet_array);
            get_packet_from_array(packet_array, &packet);
            if (not valid_packet(&packet))
            {
                printf("h: %d != %d\n", packet.header, HEADER);
                printf("packet->data_size: %d\n", packet.data_size);
                printf("packet->packet_id: %d\n", packet.packet_id);
                printf("packet->type %d\n", packet.type);
                printf("packet->data: ") ;
                for(int i = 0; i < packet.data_size; i++)
                {       
                    printf("%c", packet.data[i]);
                }

                printf("p: %d != %d\n", packet.parity, get_parity(&packet));
            }
            printf("%s\n", packet.data);
        }
        else
            printf("Nothing.");
    }   
}