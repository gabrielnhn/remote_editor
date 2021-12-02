#include "network.h"
#include "packet.h"
#include "errno.h"
#include <stdbool.h>
#include "commands.h"

#define STR_MAX 100

char client_dir[STR_MAX];
char server_dir[STR_MAX];


int parse_str_command(char* command, int* type, char* data)
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


    if (strncmp(command, "cd", strlen("cd")) == 0)
    {
        strcpy(data, command + 3);
        *type = 0;
        return CD;
    }
    else if (strncmp(command, "lcd", strlen("lcd")) == 0)
    {
        if (cd(command + 4, client_dir))
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
    struct timeval tv;
    tv.tv_sec = 0.5;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    get_realpath(".", client_dir);

    // strcpy(client_dir, ".") ;


    char command[STR_MAX];
    int send_retval, recv_retval;
    int command_id;
    bool sent_succexy;

    int msg_counter = 0;

    while(1)
    {
        ///////// GET COMMAND
        int type;
        char data[15];
        command_id = parse_str_command(command, &type, data);
        

        if (command_id != NOP)
        {
            sent_succexy = false;
            while (not sent_succexy)
            {
                memset(request.data, 0, DATA_BYTES);
                bit_copy(data, 0, request.data, 0, (strlen(data) + 1)*8);
                request.data_size = strlen(data) + 1;

                request.type = type;
                request.packet_id = msg_counter;

                make_packet_array(packet_array, &request);
                
                send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                
                if (send_retval == -1)
                    printf("Error: nothing was sent.\n");

                recv_retval = recv(socket, &packet_array, PACKET_MAX_BYTES, 0);
                if (recv_retval == -1)
                    printf("BRUH\n");

                get_packet_from_array(packet_array, &response);
                if (valid_packet(&response, (msg_counter + 2) % 16) && response.origin_address == SERVER)
                    printf("Got something\n");
                else
                    printf("Got nothing.\n");

                if (response.type == ACK)
                {
                    printf("cd retval = %d.\n", *response.data);
                    sent_succexy = true;
                }
            }
            msg_counter += 2;
        }
    }
}