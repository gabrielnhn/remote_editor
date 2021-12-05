#include "network.h"
#include "packet.h"
#include "commands.h"
#include <errno.h>
#include <unistd.h>

char server_dir[STR_MAX];
char huge_buffer[A_LOT];
int huge_buffer_counter = 0;
char path[STR_MAX];


int parse_command_packet(packet_t* packet, int* type, char* data, int* data_size)
{
    printf("packet type %d\n", packet->type);
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
        print_packet(packet);
        printf("ver received, file '%s'\n", packet->data);
        
        // strcpy(path, packet->data);
        int retval = indexed_cat(packet->data, huge_buffer);
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

    if (packet->type == LINHAS)
    {
        printf("LINHAS received\n");
        
        strcpy(huge_buffer, packet->data);
        int retval = check_filename(huge_buffer);
        if (retval != SUCCEXY)
        {
            printf("LINHAS ERROR\n");
            *data = retval;
            *type = ERROR;
            *data_size = 1;
            return ERROR;
        }
        printf("linhas has valid filename.\n");
        *type = ACK;
        return LINHAS;
    }
    
    if (packet->type == EDIT)
    {
        printf("EDIT received\n");
        
        strcpy(path, packet->data);
        int retval = check_filename(path);
        if (retval != SUCCEXY)
        {
            printf("EDIT ERROR\n");
            *data = retval;
            *type = ERROR;
            *data_size = 1;
            return ERROR;
        }
        printf("edit has valid filename.\n");

        *type = ACK;
        return EDIT;
    }

    if (packet->type == COMPILAR)
    {
        printf("Got COMPILAR\n");
        *type = ACK;
        *data_size = 0;
        return COMPILAR;
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

                    // RECEIVE LINHA_INDEXES

                    if ((command_id == LINHA) or (command_id == LINHAS) or (command_id == EDIT))
                    {
                        // LINHA has a different overhead when compared to other commands.
                        // we'll treat this and then use the foundations below this clause.

                        bool got_lines_query = false;
                        bool should_end = false;
                        msg_counter = (msg_counter + 1) % 16;

                        int send_counter = 0;
                        while ((not should_end) and (send_counter < MAX_SEND_TRIES))
                        {
                            if (got_lines_query)
                                should_end = true;
                            // set ACK response

                            memset(response.data, 0, DATA_BYTES);
                            response.data_size = 0;
                            response.type = type;
                            response.packet_id = msg_counter;
                            make_packet_array(packet_array, &response);

                            // printf("response type:");
                            // print_bits(1, &response.type);
                            // printf("\n");

                            // printf("Sending retval=%d on msg %d \n", *response.data, msg_counter);

                            // send response
                            // printf("Sending type %d id %d\n", response.type, response.packet_id);
                            // print_packet(&response);
                            send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                            if (send_retval == -1)
                                printf("Could not send response.\n");

                            usleep(TIME_BETWEEN_TRIES);

                            // try to receive LINHAS_INDEXES from client
                            int type_of_request = LINHAS_INDEXES;
                            bool got_something = false;
                            int recv_counter = 0;

                            while (not got_lines_query and not got_something and (recv_counter < MAX_RECEIVE_TRIES))
                            // if data_stream_finished, client won't receive LS_CONTENT anymore.
                            {
                                // printf("try recv\n");
                                recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                                if (recv_retval != -1){
                                    get_packet_from_array(packet_array, &request);
                                }
                                else
                                    memset(packet_array, 0, PACKET_MAX_BYTES);

                                // check request
                                if ((recv_retval != -1) and valid_packet(&request, (msg_counter + 1) % 16)
                                    and request.origin_address == CLIENT)
                                {
                                    // REAL PACKAGE!!!!
                                    if (request.type == type_of_request)
                                    {
                                        if ((command_id == LINHA) or (command_id == EDIT))
                                        {
                                            printf("Got line: '%d', on id %d\n", *request.data, request.packet_id);
                                            line1 = *request.data; 
                                        }
                                        else
                                        {
                                            line1 = *request.data;
                                            line2 = *(request.data + sizeof(line1));
                                        }
                                        got_something = true;
                                        got_lines_query = true;
                                        msg_counter = (msg_counter + 2) % 16;
                                    }
                                    else if (request.type == command_id)
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
                                        if (request.origin_address == CLIENT){
                                            printf("Didnt want %d. wanted %d\n", request.packet_id, (msg_counter + 1) % 16);
                                            // print_packet(&request);
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
                            if ((command_id == LINHA) or (command_id == LINHAS))
                                strcpy(path, huge_buffer);

                            int retval = SUCCEXY;
                            if (command_id == LINHA)
                                retval = get_line(path, line1, huge_buffer);

                            else if (command_id == LINHAS)
                                retval = get_lines(path, line1, line2, huge_buffer);

                            if (retval != SUCCEXY)
                            // didnt work
                            {
                                printf("command failed\n");
                                // error, use next case to send it
                                command_id = ERROR;
                                type = ERROR;
                                *data = retval;
                                data_size = 1;
                                msg_counter = (msg_counter + 1) % 16;

                            }
                            else
                            {
                                // printf("get_line worked\n");
                                type = FILE_CONTENT;
                                // command_id = LINHA;
                                msg_counter = (msg_counter + 1) % 16;
                            }
                        }
                        else
                        {
                            printf("Failed to get LINHAS_INDEXES\n\n\n");
                            command_id = NOP;
                        }
                    }
                    //else:
                    //    as command_id is still LINHA, we will still send the contents of huge_buffer.


                    // SEND ACK/ERROR ONCE.
                    if ((command_id == CD) or (command_id == ERROR) /*or (command_id == COMPILAR)*/)
                    {
                        // set response packet once.
                        memset(response.data, 0, DATA_BYTES);
                        memcpy(response.data, data, data_size);
                        response.data_size = data_size;
                        response.type = type;
                        // printf("response type:");
                        // print_bits(1, &response.type);
                        // printf("\n");

                        msg_counter = (msg_counter + 1) % 16;
                        response.packet_id = msg_counter;

                        printf("Sending retval=%d on msg %d \n", *response.data, msg_counter);
                        // printf("response id %d\n", response.packet_id);
                        make_packet_array(packet_array, &response);

                        // send response
                        send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                        if (send_retval == -1)
                            printf("Could not send response.\n");

                        msg_counter = (msg_counter + 2) % 16;
                        usleep(TIME_BETWEEN_TRIES);
                    }
                    
                    // RECEIVE DATA STREAM FROM CLIENT

                    if ((command_id == EDIT) or (command_id == COMPILAR))
                    {
                        if ((command_id == EDIT))
                        {
                            msg_counter = (msg_counter - 1);
                            if (msg_counter < 0)
                                msg_counter = 16 + msg_counter;
                        }
                        else if (command_id == COMPILAR)
                            msg_counter = (msg_counter + 1) % 16;

                        // printf("Starting to receive. counter: %d\n", msg_counter);
                        // else if (command_id == COMPILAR)
                        // {
                        //     msg_counter = (msg_counter + 1) % 16;
                        // }

                        int type_of_request = FILE_CONTENT;
                        bool response_validated = false; // first ACK command was acknowledged

                        // if (command_id == COMPILAR)
                        //     response_validated = true;

                        // set response packet
                        if ((command_id == EDIT) or (command_id == COMPILAR))
                        {
                            type_of_request = FILE_CONTENT;
                            data_size = 0;
                            memset(response.data, 0, DATA_BYTES);
                            type = ACK;
                            response.type = type;
                        }

                        // RESET BUFFER
                        strcpy(huge_buffer, "");

                        bool command_finished = false; // should end
                        bool data_stream_finished = false; // should end after this iteration
                        bool error = false;
                        bool got_succexy;
                        int error_from_client = 0;

                        int send_counter = 0;
                        while (not command_finished and send_counter < MAX_SEND_TRIES and not error)
                        {
                            if (data_stream_finished)
                                command_finished = true;

                            send_counter = 0;
                            got_succexy = false; // got LS_CONTENT
                        
                            while (not got_succexy and send_counter < MAX_SEND_TRIES)
                            {
                                // set ACK or NACK
                                // memset(response.data, 0, DATA_BYTES);
                                response.data_size = data_size;
                                response.type = type;
                                response.packet_id = msg_counter;
                                make_packet_array(packet_array, &response);
                                // print_packet(&response);
                                
                                // send ACK/NACK
                                printf("sending packet, id %d, type %d\n", response.packet_id, response.type);
                                send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                                if (send_retval == -1)
                                    printf("Error: nothing was sent.\n");

                                usleep(TIME_BETWEEN_TRIES);

                                // receive FILE_CONTENT from client
                                bool got_something = false;
                                int recv_counter = 0;

                                while (not got_something and (recv_counter < MAX_RECEIVE_TRIES) 
                                    and not data_stream_finished)
                                // if data_stream_finished, client won't receive LS_CONTENT anymore.
                                {

                                    recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                                    if (recv_retval != -1){
                                        get_packet_from_array(packet_array, &request);
                                    }
                                    else
                                        memset(packet_array, 0, PACKET_MAX_BYTES);

                                    // check request
                                    if ((recv_retval != -1) and valid_packet(&request, (msg_counter + 1) % 16)
                                        and request.origin_address == CLIENT)
                                    {
                                        // REAL PACKAGE!!!!
                                        if (request.type == type_of_request)
                                        {
                                            response_validated = true;
                                            printf("Got content: '%s' on id %d\n", request.data, request.packet_id);
                                            strcat(huge_buffer, request.data);
                                            got_something = true;
                                            got_succexy = true;
                                        }
                                        else if (request.type == END)
                                        {
                                            printf("Got END\n");
                                            got_something = true;
                                            got_succexy = true;
                                            data_stream_finished = true;
                                        }
                                        else if (request.type == ERROR)
                                        {
                                            got_something = true;
                                            got_succexy = true;
                                            printf("Got error message\n");
                                            error_from_client = *request.data;
                                            error = true;
                                        }
                                        else
                                            printf("Got unexpected type %d\n", request.type);

                                        got_something = true;
                                    }
                                    // timeout
                                    else
                                    {
                                        if (recv_retval != -1)
                                        {
                                            if (request.origin_address == CLIENT){
                                                printf("Didnt want %d. wanted %d\n", request.packet_id, (msg_counter + 1) % 16);
                                                // print_packet(&request);
                                                // printf("\n");
                                            }
                                        }
                                        recv_counter++;
                                    }
                                }
                                send_counter++;

                                if (not got_succexy)
                                // got LS_CONTENT successfully
                                {
                                    if (not data_stream_finished){
                                        if (response_validated)
                                            type = NACK;
                                        else
                                            type = command_id;

                                        data_size = 0;
                                    }
                                }
                                else
                                {
                                    msg_counter = (msg_counter + 2) % 16;
                                    printf("counter: %d\n", msg_counter);
                                    type = ACK;
                                }
                            }
                        }

                        // DATA STREAM ENDED

                        if (error)
                        {
                            printf("client sent error '%d'\n", error_from_client);
                        }

                        if (command_finished){
                            printf("huge buffer: '%s'\n", huge_buffer);
                            if (command_id == EDIT)
                            {
                                int retval = edit(path, line1, huge_buffer);
                                printf("EDIT RETVAL: %d\n", retval);
                            }
                            else if (command_id == COMPILAR)
                            {
                                int retval = compile(huge_buffer, huge_buffer);
                                printf("COMPILE RETVAL: %d\n", retval);
                                printf("HUGE BUFFER: '%s'\n", huge_buffer);
                            }
                        }

                        else{
                            printf("DATA STREAM error.\n");
                            // printf("%s", huge_buffer);
                        }
                        

                    }


                    // SEND DATA STREAM

                    if ((command_id == LS) or (command_id == VER) or
                        (command_id == LINHA) or (command_id == LINHAS) or (command_id == COMPILAR))
                    {
                        // Now it gets tricky.
                        // huge_buffer has LS output.

                        huge_buffer_counter = 0;
                        int length = strlen(huge_buffer);


                        msg_counter = (msg_counter + 1) % 16;

                        bool sent_succexy = true;
                        printf("Starting transmission:\n");
                        int send_counter = 0;

                        while(huge_buffer_counter < length and send_counter < MAX_SEND_TRIES)
                        {
                            if (sent_succexy)
                            // Can send next message
                            {
                                // Set next response packet
                                memset(response.data, 0, DATA_BYTES);
                                strncpy(response.data, huge_buffer + huge_buffer_counter, 14);
                                // printf("sending response.data: '%s'\n", response.data);
                                // print_packet

                                huge_buffer_counter += strlen(response.data);
                                // printf("counter: %d < %d\n", huge_buffer_counter, length);

                                response.data_size = strlen(response.data) + 1;
                                response.type = type;
                                response.packet_id = msg_counter;

                                make_packet_array(packet_array, &response);
                                printf("#");
                                sent_succexy = false;
                                send_counter = 0;
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
                                    if ((recv_retval != -1) and request.origin_address == CLIENT){
                                        // printf("Didnt want %d. wanted %d\n", request.packet_id, (msg_counter + 1) % 16);
                                        // print_packet(&request);
                                    }
                                    recv_counter++;
                                }
                                if (sent_succexy)
                                    msg_counter = (msg_counter + 2) % 16;

                            }
                            send_counter++;
                        }

                        if (huge_buffer_counter >= length)
                        {
                            // SEND END OF TRANSMISSION
                            send_counter = 0;
                            sent_succexy = false;
                            while (not sent_succexy and send_counter < MAX_SEND_TRIES)
                            {
                                // set END response

                                response.type = END;
                                response.packet_id = msg_counter;
                                response.data_size = 0;
                                make_packet_array(packet_array, &response);
                                send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                                if (send_retval != -1)
                                    printf("\nSent END\n");
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
                                send_counter++;
                            }
                            if (not sent_succexy)
                            {
                                printf("failed to send END\n");
                            }
                        }
                        else
                        {
                            printf("failed to send file\n");
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