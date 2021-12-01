#include "network.h"
#include "packet.h"
#include "errno.h"
#include <stdbool.h>
#include "commands.h"

#define STR_MAX 100


int get_command(char* command)
{
    printf("client > ");
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
        return CD;
    }
    else if (strncmp(command, "lcd", strlen("lcd")) == 0)
    {
        return LCD;
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

    packet_t packet;
    char packet_array[PACKET_MAX_BYTES];

    packet.header = HEADER;
    packet.dest_address = SERVER;
    packet.origin_address = CLIENT;

    int socket = raw_socket_connection("lo");

    char command[STR_MAX];
    int send_retval;
    int command_id;
    bool sent_succexy;
    while(1)
    {
        ///////// GET COMMAND
        command_id = get_command(command);
        

        if (command_id != NOP)
        {
            sent_succexy = false;
            while (not sent_succexy)
            {
                memset(packet.data, 0, DATA_BYTES);
                bit_copy(command, 0, packet.data, 0, (strlen(command) + 1)*8);
                // memcpy(packet.data, command, strlen(command) + 1);
                packet.data_size = strlen(command) + 1;
                // packet.data_size = DATA_BYTES;

                packet.type = 0;
                packet.packet_id = 0;
                make_packet_array(packet_array, &packet);
                
                send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
                if (send_retval == -1)
                    printf("Error: nothing was sent.");
                else
                    sent_succexy = true;
            }
        }
    }
}