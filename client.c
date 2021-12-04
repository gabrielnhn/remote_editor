#include "network.h"
#include "packet.h"
#include "errno.h"
#include <stdbool.h>
#include "commands.h"
#include <unistd.h>

#define STR_MAX 100

char huge_buffer[A_LOT];

char client_dir[STR_MAX];
int msg_counter = 0;
int linha1, linha2;

int parse_str_command(char* command)
{
    printf("client[%d]@%s : ", msg_counter, client_dir);
    char* fgets_retval = fgets(command, STR_MAX, stdin);
    
    if (fgets_retval == NULL)
    // END OF FILE
    {
        printf("\nExiting...\n");
        exit(0);
    }
    command[strcspn(command, "\n")] = 0; // remove '\n'


    if (strncmp(command, "cd ", strlen("cd ")) == 0)
    {
        return CD;
    }
    else if (strncmp(command, "lcd ", strlen("lcd ")) == 0)
    {
        if (cd(command + 4, client_dir) != SUCCEXY)
            printf("lcd failed.\n");
        
        return NOP; // local operation
    } 
    else if (strncmp(command, "ls", strlen("ls")) == 0)
    {
        return LS;
    }
    else if (strncmp(command, "ver", strlen("ls")) == 0)
    {
        return VER;
    }
    else if (strncmp(command, "lls", strlen("lls")) == 0)
    {
        ls(client_dir);
        return NOP; // local operation.
    }
    else if (strncmp(command, "linha", strlen("linha")) == 0)
    {
        sscanf(command + 6, "%d %s", &linha1, huge_buffer);
        return LINHA; // local operation.
    }

    else if (strncmp(command, "\n", strlen("\n")) == 0)
    // newline
    {
        return NOP;
    }
    else
    // Command unknown
    {
        printf("%s: command not found\n", command);
        return NOP;
    }
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);

    packet_t request, response;
    char packet_array[PACKET_MAX_BYTES];

    request.header = HEADER;
    request.dest_address = SERVER;
    request.origin_address = CLIENT;

    int socket = raw_socket_connection("lo");
    
    get_realpath(".", client_dir);

    char command[STR_MAX];
    int send_retval, recv_retval;
    int command_id;
    int data_size;
    bool sent_succexy;
    bool got_succexy;

    while(1)
    {
        //  GET COMMAND
        int type;
        char data[DATA_BYTES];
        command_id = parse_str_command(command);
        int command_retval;

        if (command_id != NOP)
        {
            if (command_id == CD)
            {
                // remove "cd " from command string
                strcpy(data, command + 3);
                data_size = strlen(command + 3) + 1;
                type = CD;

                // set request packet
                memset(request.data, 0, DATA_BYTES);
                memcpy(request.data, data, data_size);
                request.data_size = data_size;
                request.type = type;
                request.packet_id = msg_counter;
                make_packet_array(packet_array, &request);

                int send_counter = 0;
                sent_succexy = false;
                while (not sent_succexy and send_counter < MAX_SEND_TRIES)
                {
                    // send request
                    // printf("sending\n");
                    send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                    if (send_retval == -1)
                        printf("Error: nothing was sent.\n");

                    usleep(TIME_BETWEEN_TRIES);

                    // receive response FROM SERVER
                    bool got_something = false;
                    int recv_counter = 0;

                    while (not got_something and (recv_counter < MAX_RECEIVE_TRIES))
                    {
                        recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                        if (recv_retval != -1)
                            get_packet_from_array(packet_array, &response);
                        else
                            memset(packet_array, 0, PACKET_MAX_BYTES);

                        // check response
                        if ((recv_retval != -1) and valid_packet(&response, (msg_counter + 1) % 16)
                            and response.origin_address == SERVER)
                        {
                            // printf("Got something\n");
                            if (response.type == ACK or response.type == ERROR)
                            {
                                command_retval = *response.data;
                                got_something = true;
                                sent_succexy = true;
                            }
                        }
                        else
                        {
                            if (response.origin_address == SERVER){
                                // printf("Got %d, wanted %d\n", response.packet_id, (msg_counter + 1) % 16);
                                // print_packet(&response);
                            }

                            recv_counter++;
                        }
                        usleep(TIME_BETWEEN_TRIES);

                    }
                    send_counter++;
                }

                if (sent_succexy)
                // CD command is finished
                {
                    msg_counter = (msg_counter + 2) % 16;
                    // printf("counter: %d\n", msg_counter);

                    if (command_retval != 0)
                        printf("retval: %d\n", command_retval);
                }
                else
                {
                    printf("Command was not received by server.\n");
                }
            }
            // LS LS OR VER

            if (command_id == LINHA)
            {
                // send LINHA

                bool sent_linha = false;
                bool sent_query = false;
                while((not sent_linha) or (not sent_query))
                {
                    if (not sent_linha)
                    {
                        // printf("Sending linha\n");
                        // Set LINHA packet
                        memset(request.data, 0, DATA_BYTES);
                        strncpy(request.data, huge_buffer, 14);
                        request.data_size = strlen(request.data) + 1;
                        request.type = LINHA;

                        request.packet_id = msg_counter;
                        // printf("counter: %d < %d\n", huge_buffer_counter, length);
                        make_packet_array(packet_array, &request);
                    }
                    else
                    {
                        // printf("Sending linhas_indexes\n");
                        // Set LINHAS_INDEXES PACKET
                        *request.data = linha1;
                        request.data_size = 1;
                        request.type = LINHAS_INDEXES;
                        request.packet_id = msg_counter;
                        // printf("counter: %d < %d\n", huge_buffer_counter, length);
                        make_packet_array(packet_array, &request);
                    }
                    
                    // send request
                    // printf("Sending packet, id %d, i%d/%d\n", request.packet_id, huge_buffer_counter, length);
                    send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);

                    if (send_retval == -1)
                        printf("Could not send LINHA\n");

                    usleep(TIME_BETWEEN_TRIES);
                    
                    // get acknowledgement FROM SERVER
                    bool got_something = false;
                    int recv_counter = 0;

                    while (not got_something and (recv_counter < MAX_RECEIVE_TRIES))
                    {
                        recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                        if (recv_retval != -1)
                            get_packet_from_array(packet_array, &response);
                        else
                            memset(packet_array, 0, PACKET_MAX_BYTES);
                        
                        // check response
                        if ((recv_retval != -1) and (valid_packet(&response, (msg_counter + 1) % 16))
                            and response.origin_address == SERVER)
                        {
                            // printf("Got %d\n", response.packet_id);
                            // REAL PACKAGE!!!!

                            if (response.type == NACK)
                            {
                                got_something = true;
                            }
                            else if (response.type == ACK)
                            {
                                // printf("GOT ACK\n");
                                if (sent_linha)
                                    sent_query = true;
                                else
                                    sent_linha = true;
                                got_something = true;
                                msg_counter = (msg_counter + 2) % 16;
                            }
                            else
                                printf("Got something completely unexpected, type %d\n", response.type);
                            
                            got_something = true;
                        }
                        else
                        {
                            if (recv_retval != -1)
                            {
                                if (response.origin_address == SERVER){
                                    // printf("Didnt want %d. wanted %d\n", response.packet_id, (msg_counter + 1) % 16);
                                    // printf("RECEIVED WRONG?:\n");
                                    // print_packet(&response);
                                }
                            }
                            recv_counter++;
                        }
                    }
                }
            }



            if ((command_id == LS) or (command_id == VER) or (command_id == LINHA))
            {
                int type_of_response;
                bool request_validated = false; // LS command was acknowledged

                // set request packet
                if (command_id == LS)
                {
                    type_of_response = LS_CONTENT;
                    data_size = 0;
                    memset(request.data, 0, DATA_BYTES);
                    type = command_id;
                    request.type = type;
                }
                else if (command_id == VER)
                {
                    type_of_response = FILE_CONTENT;
                    // remove "ver "
                    strcpy(request.data, command + 4);
                    data_size = strlen(command + 4);
                    // printf("ver arg: %s\n", request.data);
                    type = command_id;
                    request.type = type;
                }
                else if (command_id == LINHA)
                {
                    memset(request.data, 0, DATA_BYTES);
                    type_of_response = FILE_CONTENT;
                    type = ACK;
                    request_validated = true;
                    data_size = 0;
                    // printf("TAPORRA\n");
                }
                
                strcpy(huge_buffer, "");

                bool command_finished = false; // should end
                bool data_stream_finished = false; // should end after this iteration
                bool error = false;

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
                        // memset(request.data, 0, DATA_BYTES);
                        request.data_size = data_size;
                        request.type = type;
                        request.packet_id = msg_counter;
                        make_packet_array(packet_array, &request);
                        // print_packet(&request);
                        
                        // send ACK/NACK
                        // printf("sending packet, id %d, type %d\n", request.packet_id, request.type);
                        send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                        if (send_retval == -1)
                            printf("Error: nothing was sent.\n");

                        usleep(TIME_BETWEEN_TRIES);

                        // receive LS_CONTENT from server
                        bool got_something = false;
                        int recv_counter = 0;

                        while (not got_something and (recv_counter < MAX_RECEIVE_TRIES) 
                               and not data_stream_finished)
                        // if data_stream_finished, client won't receive LS_CONTENT anymore.
                        {

                            recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                            if (recv_retval != -1){
                                get_packet_from_array(packet_array, &response);
                            }
                            else
                                memset(packet_array, 0, PACKET_MAX_BYTES);

                            // check response
                            if ((recv_retval != -1) and valid_packet(&response, (msg_counter + 1) % 16)
                                and response.origin_address == SERVER)
                            {
                                // REAL PACKAGE!!!!
                                if (response.type == type_of_response)
                                {
                                    request_validated = true;
                                    printf("Got content: '%s'\n", response.data);
                                    strcat(huge_buffer, response.data);
                                    got_something = true;
                                    got_succexy = true;
                                }
                                else if (response.type == END)
                                {
                                    // printf("transmission ended\n");
                                    got_something = true;
                                    got_succexy = true;
                                    data_stream_finished = true;
                                }
                                else if (response.type == ERROR)
                                {
                                    got_something = true;
                                    got_succexy = true;
                                    printf("Got error message\n");
                                    command_retval = *response.data;
                                    error = true;
                                }
                                else
                                    printf("Got unexpected type %d\n", response.type);
                                got_something = true;
                            }
                            // timeout
                            else
                            {
                                if (recv_retval != -1)
                                {
                                    if (response.origin_address == SERVER){
                                        printf("Didnt want %d. wanted %d\n", response.packet_id, (msg_counter + 1) % 16);
                                        // print_packet(&response);
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
                            if (request_validated)
                                type = NACK;
                            else
                                type = command_id;

                            data_size = 0;
                        }
                        else
                        {
                            msg_counter = (msg_counter + 2) % 16;
                            // printf("counter: %d\n", msg_counter);
                            type = ACK;
                        }
                    }
                }

                // Command endend

                if (error)
                {
                    printf("server sent error '%d'\n", command_retval);
                }

                if (command_finished){
                    printf("%s\n", huge_buffer);
                }
                else
                    printf("command failed.\n");
                

            }
        }
    }
}