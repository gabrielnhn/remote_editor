#include "network.h"
#include "packet.h"
#include "commands.h"
#include <errno.h>
#include <unistd.h>

char trash[STR_MAX];
char server_dir[STR_MAX];
char huge_buffer[STR_MAX];
int huge_buffer_counter = 0;


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
        
        return ONE_NIGHT_STAND;
    }
    
    if (packet->type == LS)
    {
        printf("GOT LS\n");
        ls_to_string(server_dir, huge_buffer);
        return STREAM;
    }
    return NOP;
}

int main()
{
    packet_t request, response;
    char packet_array[PACKET_MAX_BYTES];

    response.header = HEADER;
    response.dest_address = CLIENT;
    response.origin_address = SERVER;

    int socket = raw_socket_connection("lo");

    printf("Connected.\n");
    get_realpath(".", server_dir);
    printf("Current directory is %s\n", server_dir);
    // printf("Waiting for instructions from `client`.\n\n");


    int recv_retval, send_retval;
    char data[DATA_BYTES];
    int type;
    int data_size;
    int msg_counter = -1;

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
                    int command_type = parse_command_packet(&request, &type, data, &data_size);

                    if (command_type == ONE_NIGHT_STAND)
                    {
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
                    else if (command_type == STREAM)
                    {
                        huge_buffer_counter = 0;
                        int length = strlen(huge_buffer) + 1;
                        msg_counter = (msg_counter + 1);
                        response.packet_id = msg_counter;
                        msg_counter = (msg_counter + 1);

                        while(huge_buffer_counter < length)
                        {
                            bool sent_succexy = false;

                            if (sent_succexy)
                            {
                                memcpy(response.data, huge_buffer + huge_buffer_counter, 15);
                                huge_buffer_counter += 16;

                                response.data_size = 15;
                                response.type = LS_CONTENT;

                                msg_counter = (msg_counter + 1) % 16;
                                response.packet_id = msg_counter;
                                // printf("response id %d\n", response.packet_id);

                                make_packet_array(packet_array, &response);
                                msg_counter = (msg_counter + 2) % 16;
                            }

                            // send response
                            printf("Sending packet, id %d, i%d/%d\n", response.packet_id, huge_buffer_counter, length);
                            send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                            printf("%d, BRO WHAT\n", send_retval);
                            if (send_retval == -1)
                                printf("BRUH\n");

                            usleep(TIME_BETWEEN_TRIES);
                            
                            // get acknowledgement
                            // receive response FROM CLIENT
                            bool got_something = false;
                            int local_counter = 0;

                            while (not got_something)
                            {
                                // printf("receiving ACK?\n");
                                recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);
                                // printf("recv retval = %d\n\n", recv_retval);

                                if (recv_retval != -1)
                                {
                                    get_packet_from_array(packet_array, &request);
                                }
                                // check request
                                if ((recv_retval != -1) and (valid_packet(&request, (msg_counter /*+1*/) % 16) or valid_packet(&request, (msg_counter -2) % 16))
                                    and request.origin_address == CLIENT)
                                {
                                    // REAL PACKAGE!!!!

                                    if (request.type == NACK)
                                    {
                                        printf("GOT NACK\n");
                                        sent_succexy = false;
                                        got_something = true;
                                    }
                                    else if (request.type == ACK)
                                    {
                                        printf("GOT ACK\n");
                                        sent_succexy = true;
                                        got_something = true;
                                    }

                                    else if (request.type == LS)
                                    {
                                        printf("GOT LS (AGAIN)\n");
                                        sent_succexy = false;
                                        got_something = true;
                                        huge_buffer_counter = 0;
                                    }

                                    got_something = true;
                                }
                                else
                                {

                                    if (recv_retval == -1)
                                        true;

                                    else
                                    {
                                        local_counter++;
                                        if (request.origin_address == CLIENT)
                                        {
                                            print_packet(&request);
                                            printf("wanted id %d\n", msg_counter);
                                        }
                                        else
                                            printf("b.");
                                    }


                                    if (local_counter > MAX_RECEIVE_TRIES)
                                        got_something = true;
                                }
                            }
                            // msg_counter = (msg_counter + 1) % 16; 
                        }
                    }
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