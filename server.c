#include "network.h"
#include "packet.h"
#include "commands.h"
#include <errno.h>
#include <unistd.h>

char trash[STR_MAX];
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
            *type = ACK;
        }
        else
        {
            printf("cd failed\n");
            *type = ERROR;
        }
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
    // struct timeval tv;
    // tv.tv_sec = 0.5;
    // tv.tv_usec = 0;
    // setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);


    printf("Connected.\n");
    get_realpath(".", server_dir);
    printf("Current directory is %s\n", server_dir);
    printf("Waiting for instructions from `client`.\n\n");


    int recv_retval, send_retval;
    char data[DATA_BYTES];
    int type;
    int data_size;
    int msg_counter = -1;

    bool should_be_ignored = true;
    bool from_client;
    bool duplicate = false;

    while(1)
    {
        recv_retval = recv(socket, &packet_array, PACKET_MAX_BYTES, 0);
        if (recv_retval != -1)
        {
            get_packet_from_array(packet_array, &request);

            // Check for duplicate
            int previous_id = msg_counter - 2;
            if (previous_id < 0)
                previous_id = 16 - previous_id;
            

            if (request.packet_id == previous_id)
                duplicate = not duplicate;
            else
                duplicate = false;

            from_client = (request.origin_address == CLIENT);

            if (from_client and (not duplicate))
            {

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
                    memcpy(response.data, data, data_size);
                    response.data_size = data_size;
                    response.type = type;
                    printf("Sending retval = %d\n", *response.data);

                    msg_counter = (msg_counter + 1) % 16;
                    response.packet_id = msg_counter;
                    // printf("response id %d\n", response.packet_id);

                    make_packet_array(packet_array, &response);


                    // send response
                    send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                    if (send_retval == -1)
                        printf("BRUH\n");

                    msg_counter = (msg_counter + 1) % 16;

                    usleep(TIME_BETWEEN_TRIES);
                }
                else
                {
                    if (valid_packet(&request, previous_id))
                    {
                        printf("resending message %d\n", previous_id);
                        // resend response
                        make_packet_array(packet_array, &response);
                        send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                        if (send_retval == -1)
                            printf("BRUH\n");
                        usleep(TIME_BETWEEN_TRIES);
                        
                    }
                }
            } 
            else
            {
                // printf("ignored.");
                // if (duplicate)
                //     printf("duplicate\n");
                // else
                //     printf("not from_client\n");
            }
        }
        else
            true;   
            // printf("Nothing received.");
    }
}