#include "network.h"
#include "packet.h"

bool valid_packet(packet_t* packet, int id)
// if id == -1, won't check for parity.
{
    if (packet->header != HEADER){
        // printf("header........\n");
        return false;
    }

    if (get_parity(packet) != packet->parity){
        // printf("parity........\n");
        return false;
    }

    if (id != -1){
        if (packet->packet_id != id){
            // printf("id........\n");
            return false;
        }
    }

    return true;
}

unsigned char get_parity(packet_t* packet)
{
    unsigned char parity = packet->data_size;
    parity = parity ^ packet->packet_id;
    parity = parity ^ packet->type;

    for(int i = 0; i < packet->data_size; i++)
    {
        parity = parity ^ packet->data[i];
    }

    return parity;
}

int get_packet_from_array(char* array, packet_t* packet)
{
    packet->header = 0;
    packet->origin_address = 0;
    packet->dest_address = 0;
    packet->origin_address = 0;
    packet->data_size = 0;
    packet->packet_id = 0;
    packet->type = 0;
    strcpy(packet->data, "");
    packet->parity = 0;

    int index = 0;

    // GET HEADER
    bit_copy(array, 0, (char*) &packet->header, 0, 8);

    // GET DEST
    index = 8;
    bit_copy(array, index, (char*) &packet->dest_address, 0, 2);

    index += 2;  

    // GET ORIGIN
    bit_copy(array, index, (char*) &packet->origin_address, 0, 2);
    index += 2;

    // GET SIZE
    bit_copy(array, index, (char*) &packet->data_size, 0, 4);
    index += 4;

    // GET ID
    bit_copy(array, index, (char*) &packet->packet_id, 0, 4);
    index += 4;

    // GET TYPE
    bit_copy(array, index, (char*) &packet->type, 0, 4);
    index += 4;


    // GET DATA
    memset(packet->data, 0, DATA_BYTES);
    bit_copy(array, index, (char*) packet->data, 0, packet->data_size*8);
    index += packet->data_size * 8;

    // GET PARITY
    packet->parity = 0;
    bit_copy(array, index, (char*) &packet->parity, 0, 8);
    index += 8;

    return 0;
}


int make_packet_array(char* array, packet_t* packet)
{
    unsigned char dest = packet->dest_address;
    unsigned char origin = packet->origin_address;
    unsigned char size = packet->data_size;
    unsigned char id = packet->packet_id;
    unsigned char type = packet->type;
    void* data = packet->data;

    int index = 0;

    // EMPTY PACKET!
    memset(array, 0, PACKET_MAX_BYTES);

    // SET HEADER
    char h = HEADER;
    bit_copy((char*) &h, 0, array, 0, 8);

    index += 8;

    // SET DEST
    bit_copy((char*) &dest, 0, array, index, 2);
    index += 2;  

    // SET ORIGIN
    bit_copy((char*) &origin, 0, array, index, 2);
    index += 2;

    // SET SIZE
    bit_copy((char*) &size, 0, array, index, 4);
    index += 4;

    // SET ID
    bit_copy((char*) &id, 0, array, index, 4);
    index += 4;

    // SET TYPE
    bit_copy((char*) &type, 0, array, index, 4);
    index += 4;

    // SET DATA
    bit_copy((char*) data, 0, array, index, size*8);
    index += size * 8;


    // SET PARITY
    packet->parity = get_parity(packet);
    bit_copy((char*) &packet->parity, 0, array, index, 8);
    index += 8;

    return 0;
}



void print_packet(packet_t* packet)
{
    printf("packet->origin: ");
    if (packet->origin_address == SERVER)
        printf("SERVER\n");
    else if (packet->origin_address == CLIENT)
        printf("CLIENT\n");
    else
        printf("%d\n",packet->origin_address);
    printf("packet->data_size: %d\n", packet->data_size);
    printf("packet->packet_id: %d\n", packet->packet_id);
    printf("packet->type %d\n", packet->type);
    printf("packet->data: ") ;
    for(int i = 0; i < packet->data_size; i++)
    {       
        printf("%c", packet->data[i]);
    }
    printf("\nparity %d\n", packet->parity);

}