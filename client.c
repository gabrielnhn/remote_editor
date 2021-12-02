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
        return ONE_NIGHT_STAND;
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
    // struct timeval tv;
    // tv.tv_sec = 0.5;
    // tv.tv_usec = 0;
    // setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    get_realpath(".", client_dir);

    // strcpy(client_dir, ".") ;

    char huge_buffer[A_LOT];


    char command[STR_MAX];
    int send_retval, recv_retval;
    int command_type;
    int data_size;
    bool sent_succexy;

    int msg_counter = 0;

    while(1)
    {
        ///////// GET COMMAND
        int type;
        char data[15];
        command_type = parse_str_command(command, &type, data, &data_size);
        int command_retval;

        if (command_type != NOP)
        {
            if (command_type == ONE_NIGHT_STAND)
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
                        // printf("try recv\n");
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
            else if (command_type == STREAM)
            {
                memset(huge_buffer, 0, A_LOT); // clean buffer
            }
        }
    }
}