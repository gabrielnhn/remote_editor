#include "network.h"
#include "packet.h"
#include "errno.h"


int main()
{
    packet_t packet;
    char packet_array[PACKET_MAX_BYTES];


    char data = 42;

    bit_copy(&data, 0, packet.data, 0, 8);
    packet.dest_address = SERVER;
    packet.origin_address = CLIENT;
    packet.data_size = sizeof(char)*8;
    packet.type = 0;
    packet.packet_id = 0;

    make_packet_array(packet_array, &packet);

    int socket = raw_socket_connection("lo");

    int retval;
    while(1)
    {
        // BITNSLOTS(PACKET_MAX_BITS)
        retval = send(socket, &packet_array, 14, 0);
        printf("sent. retval=%d\n", retval);
        // printf("%d\n", errno);
        printf("%d\n", PACKET_MAX_BYTES);


    }
}