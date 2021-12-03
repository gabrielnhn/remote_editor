#include "network.h"
#include "packet.h"
#include "commands.h"
#include <errno.h>
#include <unistd.h>

char server_dir[STR_MAX];
char huge_buffer[A_LOT];
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
        
        return CD;
    }
    
    if (packet->type == LS)
    {
        printf("ls received\n");
        int retval = ls_to_string(server_dir, huge_buffer);

        if (retval != SUCCEXY)
        {
            printf("LS ERROR\n");
            *data = retval;
            *data_size = 1;
            *type = ERROR;
            return ERROR;
        }

        *type = LS_CONTENT;
        return LS;
    }

    if (packet->type == VER)
    {
        printf("ver received\n");
        
        char path[STR_MAX];
        strcpy(path, packet->data);
        int retval = indexed_cat(path, huge_buffer);
        if (retval != SUCCEXY)
        {
            printf("VER ERROR\n");
            *data = retval;
            *type = ERROR;
            *data_size = 1;
            return ERROR;
        }
        printf("ver was successfull.\n");
        *type = FILE_CONTENT;
        return VER;
    }

    if (packet->type == LINHA)
    {
        printf("LINHA received\n");
        
        strcpy(huge_buffer, packet->data);
        int retval = check_filename(huge_buffer);
        if (retval != SUCCEXY)
        {
            printf("LINHA ERROR\n");
            *data = retval;
            *type = ERROR;
            *data_size = 1;
            return ERROR;
        }
        printf("linha has valid filename.\n");
        *type = ACK;
        return LINHA;
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

    char data[DATA_BYTES];
    int recv_retval, send_retval;
    int type;
    int data_size;
    int msg_counter = -1;

    bool from_client;
    bool duplicate = false;
    int previous_id;

    while(1)
    {
        // try to receive request
        recv_retval = recv(socket, &packet_array, PACKET_MAX_BYTES, 0);
        if (recv_retval != -1)
        {
            get_packet_from_array(packet_array, &request);

            from_client = (request.origin_address == CLIENT);

            if (from_client)
            {
                if (msg_counter == -1)
                {
                    // start counter
                    msg_counter = request.packet_id;
                    printf("Packet counter started as %d\n", msg_counter);
                }

                printf("Got %d, expecting %d\n", request.packet_id, msg_counter);

                // Check for duplicate. It's a way to filter out requests.

                previous_id = msg_counter - 2;
                if (previous_id < 0)
                    previous_id = 16 + previous_id;

                if (request.packet_id == previous_id)
                    duplicate = not duplicate;
                else
                    duplicate = false;
            }

            if (from_client and (not duplicate))
            // do not ignore this command.
            {
                if (valid_packet(&request, -1))
                {
                    msg_counter = (request.packet_id) % 16;
                    // execute command
                    int command_id = parse_command_packet(&request, &type, data, &data_size);

                    int line1, line2;

                    if (command_id == LINHA)
                    {
                        // LINHA has a different overhead when compared to other commands.
                        // we'll treat this and then use the foundations below this clause.

                        bool got_lines_query = false;
                        msg_counter = (msg_counter + 1) % 16;

                        int send_counter = 0;
                        while (not got_lines_query and send_counter < MAX_SEND_TRIES)
                        {
                            // set ACK response

                            memset(response.data, 0, DATA_BYTES);
                            memcpy(response.data, data, data_size);
                            response.data_size = data_size;
                            response.type = type;
                            response.packet_id = msg_counter;
                            make_packet_array(packet_array, &response);

                            // printf("response type:");
                            // print_bits(1, &response.type);
                            // printf("\n");

                            // printf("Sending retval=%d on msg %d \n", *response.data, msg_counter);
                            // printf("response id %d\n", response.packet_id);

                            // send response
                            send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                            if (send_retval == -1)
                                printf("Could not send response.\n");

                            usleep(TIME_BETWEEN_TRIES);

                            // try to receive LINHAS_INDEXES from client
                            int type_of_response = LINHAS_INDEXES;
                            bool got_something = false;
                            int recv_counter = 0;

                            while (not got_something and (recv_counter < MAX_RECEIVE_TRIES))
                            // if data_stream_finished, client won't receive LS_CONTENT anymore.
                            {
                                // printf("try recv\n");
                                recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                                if (recv_retval != -1){
                                    get_packet_from_array(packet_array, &response);
                                }
                                else
                                    memset(packet_array, 0, PACKET_MAX_BYTES);

                                // check response
                                if ((recv_retval != -1) and valid_packet(&response, (msg_counter + 1) % 16)
                                    and response.origin_address == CLIENT)
                                {
                                    // REAL PACKAGE!!!!
                                    if (response.type == type_of_response)
                                    {
                                        printf("Got line: '%d'\n", *response.data);
                                        line1 = *response.data; 
                                        got_something = true;
                                        got_lines_query = true;
                                    }
                                    else if (response.type == command_id)
                                    {
                                        got_something = true;
                                    }

                                    got_something = true;
                                }
                                // timeout
                                else
                                {
                                    if (recv_retval != -1)
                                    {
                                        if (response.origin_address == CLIENT){
                                            // printf("Didnt want %d. wanted %d\n", response.packet_id, (msg_counter + 1) % 16);
                                            // print_packet(&response);
                                            // printf("\n");
                                        }
                                    }
                                    recv_counter++;
                                }
                            }
                            send_counter++;
                        }

                        if (got_lines_query)
                        {
                            char path[STR_MAX];
                            strcpy(path, huge_buffer);
                            int retval = get_line(path, line1, huge_buffer);
                            if (retval != SUCCEXY)
                            {
                                printf("LINHA failed");
                                // error, use next case to send it
                                command_id = ERROR;
                                type = ERROR;
                                *data = retval;
                                data_size = 1;
                            }
                            //else:
                            //    as command_id is still LINHA, we will still send the contents of huge_buffer.

                        }


                    }


                    if ((command_id == CD) or (command_id == ERROR))
                    {
                        // set response packet once.
                        memset(response.data, 0, DATA_BYTES);
                        memcpy(response.data, data, data_size);
                        response.data_size = data_size;
                        response.type = type;
                        printf("response type:");
                        print_bits(1, &response.type);
                        printf("\n");

                        msg_counter = (msg_counter + 1) % 16;
                        response.packet_id = msg_counter;

                        printf("Sending retval=%d on msg %d \n", *response.data, msg_counter);
                        // printf("response id %d\n", response.packet_id);
                        make_packet_array(packet_array, &response);

                        // send response
                        send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                        if (send_retval == -1)
                            printf("Could not send response.\n");

                        msg_counter = (msg_counter + 1) % 16;
                        usleep(TIME_BETWEEN_TRIES);
                    }
                    

                    // LS LS LS OR VER



                    if ((command_id == LS) or (command_id == VER) or (command_id == LINHA))
                    {
                        // Now it gets tricky.
                        // huge_buffer has LS output.

                        huge_buffer_counter = 0;
                        int length = strlen(huge_buffer);

                        msg_counter = (msg_counter + 1) % 16;

                        bool sent_succexy = true;
                        printf("Starting transmission:");
                        while(huge_buffer_counter < length)
                        {
                            if (sent_succexy)
                            // Can send next message
                            {
                                // Set next response packet

                                memset(response.data, 0, DATA_BYTES);
                                strncpy(response.data, huge_buffer + huge_buffer_counter, 14);
                                // printf("sending response.data: '%s'\n", response.data);

                                huge_buffer_counter += strlen(response.data);
                                // printf("counter: %d < %d\n", huge_buffer_counter, length);

                                response.data_size = strlen(response.data) + 1;
                                response.type = type;
                                response.packet_id = msg_counter;

                                make_packet_array(packet_array, &response);
                                printf("##");
                                sent_succexy = false;
                            }

                            // send response
                            // printf("Sending packet, id %d, i%d/%d\n", response.packet_id, huge_buffer_counter, length);
                            send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);

                            if (send_retval == -1)
                                printf("Could not send LS data\n");

                            usleep(TIME_BETWEEN_TRIES);
                            

                            // get acknowledgement FROM CLIENT
                            bool got_something = false;
                            int recv_counter = 0;

                            while (not got_something and (recv_counter < MAX_RECEIVE_TRIES))
                            {
                                recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                                if (recv_retval != -1)
                                    get_packet_from_array(packet_array, &request);
                                else
                                    memset(packet_array, 0, PACKET_MAX_BYTES);
                                

                                // check request
                                if ((recv_retval != -1) and (valid_packet(&request, (msg_counter + 1) % 16))
                                    and request.origin_address == CLIENT)
                                {
                                    // printf("Got %d\n", request.packet_id);
                                    // REAL PACKAGE!!!!

                                    if (request.type == NACK)
                                    {
                                        // printf("GOT NACK\n");
                                        sent_succexy = false;
                                        got_something = true;
                                    }
                                    else if (request.type == ACK)
                                    {
                                        // printf("GOT ACK\n");
                                        sent_succexy = true;
                                        got_something = true;
                                    }

                                    else if (request.type == LS)
                                    {
                                        // printf("GOT LS (AGAIN)\n");
                                        sent_succexy = false;
                                        got_something = true;
                                        huge_buffer_counter = 0;
                                    }

                                    got_something = true;
                                }
                                else
                                {
                                    recv_counter++;
                                }
                                if (sent_succexy)
                                    msg_counter = (msg_counter + 2) % 16;

                            }
                        }

                        // SEND END OF TRANSMISSION

                        sent_succexy = false;
                        while (not sent_succexy)
                        {
                            // set END response

                            response.type = END;
                            response.packet_id = msg_counter;
                            response.data_size = 0;
                            make_packet_array(packet_array, &response);
                            send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                            if (send_retval != -1)
                                printf("Sent END\n");
                            else
                                printf("Could not send END\n");


                            usleep(TIME_BETWEEN_TRIES);

                            bool got_something = false;
                            int recv_counter = 0;

                            while (not got_something and (recv_counter < MAX_RECEIVE_TRIES))
                            {
                                recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);
                                if (recv_retval != -1)
                                    get_packet_from_array(packet_array, &request);
                                else
                                    memset(packet_array, 0, PACKET_MAX_BYTES);

                                // check for last ACK
                                if ((recv_retval != -1) and (valid_packet(&request, (msg_counter + 1) % 16))
                                    and request.origin_address == CLIENT)
                                {
                                    // REAL PACKAGE!!!!
                                    if (request.type == NACK)
                                    {
                                        got_something = true;
                                    }
                                    else if (request.type == ACK)
                                    {
                                        sent_succexy = true;
                                        got_something = true;
                                    }
                                    got_something = true;
                                }
                                else
                                {
                                    recv_counter++;
                                }
                            }
                        }
                        msg_counter = (msg_counter + 1) % 16;
                        printf("Done.\n");
                    }
                }


                else // CLIENT reasking for previous message?
                {
                    if (valid_packet(&request, previous_id))
                    {
                        printf("Resending message %d\n", previous_id + 1);
                        // resend response

                        response.packet_id = previous_id + 1;
                        make_packet_array(packet_array, &response);

                        // print_packet(&response);
                        send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                        if (send_retval == -1)
                            printf("Could not resend message\n");
                        
                        usleep(TIME_BETWEEN_TRIES);
                    }
                }
            } 
            else // message was ignored.
            {
                printf("ignored %d.\n", request.packet_id);
                // if (duplicate)
                //     printf("duplicate\n");
                // else
                //     printf("not from_client\n");
            }
        }
        else
            // No value from recv()
            // printf("Nothing received.");
            true;   
    }
}