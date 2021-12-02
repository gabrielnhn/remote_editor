#include "network.h"
#include "packet.h"
#include "commands.h"
#include <errno.h>


char server_dir[STR_MAX];

int parse_command_packet(packet_t* packet, int* type, char* data)
{
    if (packet->type == CD)
    {
        int retval = cd(packet->data, server_dir);
        if (retval == SUCCEXY)
        {
            printf("Switched to %s\n", server_dir);
        }
        else
        {
            printf("cd failed\n");
        }

        *type = ACK;
        memset(data, 0, DATA_BYTES);
        memcpy(data, &retval, sizeof(char));
    }
    return 0;
}

int main()
{
    packet_t request, response;
    char packet_array[PACKET_MAX_BYTES];

    request.header = HEADER;
    request.dest_address = CLIENT;
    request.origin_address = SERVER;


    int socket = raw_socket_connection("lo");
    // LINUX
    struct timeval tv;
    tv.tv_sec = 0.5;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);


    printf("Connected.\n");
    get_realpath(".", server_dir);
    printf("Current directory is %s\n", server_dir);
    printf("Waiting for instructions from `client`.\n\n");


    int recv_retval, send_retval;
    char data[DATA_BYTES];
    int type;
    while(1)
    {
        recv_retval = recv(socket, &packet_array, PACKET_MAX_BYTES, 0);

        if ((recv_retval != -1) or (errno != 0))
        {
            // print_bits(PACKET_MAX_BYTES, packet_array);
            get_packet_from_array(packet_array, &request);
            if (not valid_packet(&request))
            {
                // printf("h: %d != %d\n", packet.header, HEADER);
                // printf("packet->data_size: %d\n", packet.data_size);
                // printf("packet->packet_id: %d\n", packet.packet_id);
                // printf("packet->type %d\n", packet.type);
                // printf("packet->data: ") ;
                // for(int i = 0; i < packet.data_size; i++)
                // {       
                //     printf("%c", packet.data[i]);
                // }

                printf("p: %d != %d\n", request.parity, get_parity(&request));
            }
            // printf("got a packet\n");
            // printf("%s\n", packet.data);
            parse_command_packet(&request, &type, data);

            memset(response.data, 0, DATA_BYTES);
            bit_copy(data, 0, response.data, 0, (strlen(data) + 1)*8);
            response.data_size = strlen(data) + 1;

            response.type = type;
            response.packet_id = 0;
            make_packet_array(packet_array, &response);

            send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
            if (send_retval == -1)
                printf("BRUH\n");

        }
        else
            printf("Nothing.");
        // printf("%d\n", errno);
    }   
}