#include "network.h"
#include "packet.h"
#include "errno.h"
#include <stdbool.h>

#define STR_MAX 100

int get_command(char* command)
{
    bool valid = true;

    printf("client > ");
    char* fgets_retval = fgets(command, STR_MAX, stdin);
    
    if (fgets_retval == NULL)
    // END OF FILE
    {
        printf("\nExiting...\n");
        exit(0);
    }

    if (strncmp(command, "cd", strlen("cd")) == 0)
    {

    }
    else if (strncmp(command, "lcd", strlen("lcd")) == 0)
    {

    } 
    else if (strncmp(command, "\n", strlen("\n")) == 0)
    // newline
    {
        valid = false;
    }
    else
    // Command unknown
    {
        valid = false;
        command[strcspn(command, "\n")] = 0; // remove '\n'
        printf("%s: command not found\n", command);
    }
    return valid;
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
    bool valid;
    while(1)
    {
        ///////// GET COMMAND
        valid = get_command(command);
        

        if (valid)
        {
            memset(packet.data, 0, DATA_BYTES);
            bit_copy(command, 0, packet.data, 0, (strlen(command) + 1)*8);
            // memcpy(packet.data, command, strlen(command) + 1);
            packet.data_size = strlen(command) + 1;
            // packet.data_size = DATA_BYTES;

            packet.type = 0;
            packet.packet_id = 0;
            make_packet_array(packet_array, &packet);

            for(int i = 0; i < packet.data_size; i++)
            {       
                printf("%c", packet.data[i]);
            }

            printf("parity %d\n", packet.parity);

            
            send_retval = send(socket, &packet_array, PACKET_MAX_BYTES, 0);
        }
    }
}