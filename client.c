#include "network.h"
#include "packet.h"
#include "errno.h"
#include <stdbool.h>
#include "commands.h"
#include <unistd.h>

#define STR_MAX 100

char client_dir[STR_MAX];
char server_dir[STR_MAX];
char trash[STR_MAX];


int parse_str_command(char* command, int* type, char* data, int* data_size)
{
    printf("client@%s : ", client_dir);
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
        strcpy(data, command + 3);
        *data_size = strlen(command + 3) + 1;
        *type = 0;
        return CD;
    }
    else if (strncmp(command, "lcd ", strlen("lcd ")) == 0)
    {
        if (cd(command + 4, client_dir) == SUCCEXY)
        {
            // printf("updated\n");
            true;
        }
        else
            printf("lcd failed.\n");
        
        return NOP; // local operation
    } 
    else if (strncmp(command, "ls", strlen("ls")) == 0)
    {
        *data_size = 0;
        *type = LS;
        return LS;
    }
    else if (strncmp(command, "lls", strlen("lls")) == 0)
    {
        ls(client_dir);
        return NOP; // local operation.
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

    char huge_buffer[A_LOT];
    // int huge_buffer_counter = 0;

    char command[STR_MAX];
    int send_retval, recv_retval;
    int command_id;
    int data_size;
    bool sent_succexy;
    bool got_succexy;

    int msg_counter = 0;

    while(1)
    {
        ///////// GET COMMAND
        int type;
        char data[15];
        command_id = parse_str_command(command, &type, data, &data_size);
        int command_retval;

        if (command_id != NOP)
        {
            if (command_id == CD)
            {
                int counter = 0;

                sent_succexy = false;
                while (not sent_succexy and counter < MAX_SEND_TRIES)
                {
                    // set request packet
                    memset(request.data, 0, DATA_BYTES);
                    memcpy(request.data, data, data_size);
                    request.data_size = data_size;
                    request.type = type;
                    request.packet_id = msg_counter;
                    make_packet_array(packet_array, &request);
                    
                    // send request
                    printf("sending\n");
                    send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                    if (send_retval == -1)
                        printf("Error: nothing was sent.\n");

                    usleep(TIME_BETWEEN_TRIES);

                    // receive response FROM SERVER
                    bool got_something = false;
                    int local_counter = 0;

                    while (not got_something)
                    {
                        recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                        if (recv_retval != -1)
                            get_packet_from_array(packet_array, &response);

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
                            local_counter++;
                            if (local_counter > MAX_RECEIVE_TRIES)
                                got_something = true;
                        }
                    }
                    // printf("counter: %d\n", counter);
                    counter++;
                }
                if (sent_succexy)
                {
                    msg_counter += 2;
                    printf("retval: %d\n", command_retval);
                }
                else
                {
                    printf("Command was not received by server.\n");
                }
            }



            // LS LS lS



            else if (command_id == LS)
            {
                strcpy(huge_buffer, "");
                // huge_buffer_counter = 0;
                
                // set request packet
                memset(request.data, 0, DATA_BYTES);
                memcpy(request.data, data, data_size);
                request.data_size = data_size;
                request.type = type;
                
                // // send request
                if (send_retval == -1)
                    printf("Error: nothing was sent.\n");

                usleep(TIME_BETWEEN_TRIES);

                bool request_validated = false;

                bool LS_over = false;
                while (not LS_over)
                {
                    // for every not final packet 
                    int counter = 0;

                    got_succexy = false;
                    while (not got_succexy and counter < MAX_SEND_TRIES)
                    {
                        // set packet
                        memset(request.data, 0, DATA_BYTES);
                        request.data_size = data_size;
                        request.type = type;
                        request.packet_id = msg_counter;
                        make_packet_array(packet_array, &request);
                        
                        // send packet
                        // printf("sending packet, id %d, type %d\n", request.packet_id, request.type);
                        send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                        if (send_retval == -1)
                            printf("Error: nothing was sent.\n");

                        usleep(TIME_BETWEEN_TRIES);

                        // receive response FROM SERVER
                        bool got_something = false;
                        int local_counter = 0;

                        while (not got_something)
                        {
                            // printf("try recv\n");
                            recv_retval = recv(socket,&packet_array, PACKET_MAX_BYTES, 0);

                            if (recv_retval != -1){
                                get_packet_from_array(packet_array, &response);
                            }

                            // check response
                            if ((recv_retval != -1) and valid_packet(&response, (msg_counter + 1) % 16)
                                and response.origin_address == SERVER)
                            {
                                // REAL PACKAGE!!!!
                                if (response.type == LS_CONTENT)
                                {
                                    // printf("Got content: '%s'\n", response.data);
                                    strcat(huge_buffer, response.data);
                                    got_something = true;
                                    got_succexy = true;
                                }
                                else if (response.type == ERROR)
                                {
                                    printf("command failed\n");
                                    got_something = true;
                                    got_succexy = true;
                                }
                                else if (response.type == END)
                                {
                                    // printf("transmission ended\n");
                                    got_something = true;
                                    got_succexy = true;
                                    LS_over = true;
                                }
                                got_something = true;
                            }
                            // timeout
                            else
                            {
                                if (recv_retval != -1)
                                {
                                    if (response.origin_address == SERVER){
                                        // printf("Didnt want %d. wanted %d\n", response.packet_id, (msg_counter + 1) % 16);
                                        // print_packet(&response);
                                        // printf("\n");
                                    }
                                    // else
                                    //     printf("b.");
                                }
                                local_counter++;
                                if (local_counter > MAX_RECEIVE_TRIES)
                                    got_something = true;
                            }
                        }
                        counter++;

                        if (got_something and not got_succexy)
                        {
                            if (request_validated)
                                type = NACK;
                            else
                                type = LS;

                            data_size = 0;
                        }
                        if (got_something and got_succexy)
                        {
                            msg_counter = (msg_counter + 2) % 16;
                            type = ACK;
                        }
                    }
                }
                printf("%s\n", huge_buffer);
            }
        }
    }
}