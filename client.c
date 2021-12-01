#include "network.h"
#include "packet.h"

int main()
{
    packet_t packet;
    char packet_array[BITNSLOTS(PACKET_MAX)];

    char* data = "BRUH";

    bit_copy(data, 0, packet.data, 0, 8);
    packet.dest_address = SERVER;
    packet.origin_address = CLIENT;
    packet.data_size = sizeof(char)*8;
    packet.type = 0;
    packet.packet_id = 0;

    make_packet_array(packet_array, &packet);


    struct sockaddr_ll address;
    int socket = raw_socket_connection("lo");

    int retval;
    while(1)
    {
        printf("sent.");
        retval = send(socket, &packet_array, BITNSLOTS(PACKET_MAX), 0);
    }
}