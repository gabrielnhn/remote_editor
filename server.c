#include "network.h"
#include "packet.h"
#include "commands.h"
#include <errno.h>
#include <unistd.h>


char server_dir[STR_MAX];

int parse_command_packet(packet_t* packet, int* type, char* data, int* data_size)
{
    if (packet->type == CD)
    {
        printf("cd to %s\n", packet->data);
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
        *data_size = sizeof(char);
    }
    return 0;
}

int main()
{
    packet_t request, response;
    char packet_array[PACKET_MAX_BYTES];

    response.header = HEADER;
    response.dest_address = CLIENT;
    response.origin_address = SERVER;


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
    int data_size;
    int msg_counter = -1;

    while(1)
    {
        recv_retval = recv(socket, &packet_array, PACKET_MAX_BYTES, 0);

        if (recv_retval != -1)
        {
            // get request
            get_packet_from_array(packet_array, &request);

            if (msg_counter == -1)
            {
                // start counter
                msg_counter = request.packet_id;
                printf("Packet counter started as %d\n", msg_counter);
            }

            if (valid_packet(&request, msg_counter))
            {
                // execute command
                parse_command_packet(&request, &type, data, &data_size);

                // set response packet

                memset(response.data, 0, DATA_BYTES);
                // bit_copy(data, 0, response.data, 0, (strlen(data) + 1)*8);
                memcpy(response.data, data, data_size);
                response.data_size = data_size;
                response.type = type;
                printf("Sending retval = %d\n", *response.data);

                msg_counter = (msg_counter + 1) % 16;
                response.packet_id = msg_counter;
                printf("response id %d\n", response.packet_id);

                make_packet_array(packet_array, &response);


                // send response
                send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                if (send_retval == -1)
                    printf("BRUH\n");

                msg_counter = (msg_counter + 1) % 16;
            }
            else
            {
                int a = msg_counter - 2;
                if (a < 0)
                    a = 16 - a;
    
                if (valid_packet(&request, a))
                {
                    printf("resending message %d\n", a);
                    // resend response
                    make_packet_array(packet_array, &response);
                    send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                    if (send_retval == -1)
                        printf("BRUH\n");
                }
            }
        }
        else
            printf("Sent Nothing.");
        // printf("%d\n", errno);
    }   
}