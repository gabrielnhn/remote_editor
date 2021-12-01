#include "network.h"
#include "packet.h"
#include "errno.h"


int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);

    packet_t packet;
    char packet_array[PACKET_MAX_BYTES];


    char data = 42;

    packet.dest_address = SERVER;
    packet.origin_address = CLIENT;
    packet.type = 0;
    packet.packet_id = 0;


    int socket = raw_socket_connection("lo");


    char command[100];
    int retval;
    while(1)
    {
        fgets(command, 100, stdin);
        memset(packet.data, 0, DATA_BYTES);
        bit_copy(command, 0, packet.data, 0, 8*DATA_BYTES);
        packet.data_size = strlen(command) + 1;

        memset(packet_array, 0, PACKET_MAX_BYTES);
        make_packet_array(packet_array, &packet);

        retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);

    }
}