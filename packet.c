#include "network.h"
#include "packet.h"

// int make_packet(packet_t* packet, int origin, int dest, int size, int id, int type, void* data)
// {
//     packet->header = HEADER;
//     packet->origin_address = origin;
//     packet->dest_address = dest;
//     packet->data_size = size;
//     packet->packet_id = id;
//     packet->type = type;
    
//     memset(packet->data_and_parity, 0, 6);

//     // memcpy(packet->data_and_parity, data, size);


//     get_parity(packet);

//     // packet->data_and_parity = 15;

//     // print_bits(sizeof(packet_t), &packet);
//     return 0;
// }

// [0:7] Header
// [8:9] Destination Address
// [10:11] Origin Address
// [12:15] Data Size
// [16:19] Sequencialization (packet ID)
// [20] Type
// [21: 20 + Size] # if Size > 0 # DATA
// [21 + Size: 29 + Size] Parity

// int get_parity(packet_t* packet)
// {
//     char str[100] = "";
//     print_bits_to_str(sizeof(packet_t), packet, str);
//     // print_bits(sizeof(packet_t), packet);
//     printf("%s", str);
//     return 0;
// }

int get_packet_from_array(char* array, packet_t* packet)
{
    char dest = 0;
    char origin = 0;
    char size = 0;
    char id = 0;
    char type = 0;

    int index = 0;

    // GET HEADER
    char h;
    bit_copy(array, 0, (char*) &h, 0, 8);

    index = 8;
    // GET DEST
    bit_copy(array, index, &dest, 0, 2);

    index += 2;  

    // GET ORIGIN
    bit_copy(array, index, &origin, 0, 2);
    index += 2;

    // GET SIZE
    bit_copy(array, index, (char*) &size, 0, 4);
    index += 4;

    // GET ID
    bit_copy(array, index, (char*) &id, 0, 4);
    index += 4;

    // GET TYPE
    bit_copy(array, index, (char*) &type, 0, 4);
    index += 4;

    // GET DATA
    bit_copy(array, index, packet->data, 0, size);
    index += size;

    packet->dest_address =  dest;
    packet->origin_address =  origin;
    packet->data_size =  size;
    packet->packet_id =  id;
    packet->type =  type;

    // print_bits(BITNSLOTS(PACKET_MAX), array);
    return 0;
}


int make_packet_array(char* array, packet_t* packet)
{
    char dest = packet->dest_address;
    char origin = packet->origin_address;
    char size = packet->data_size;
    char id = packet->packet_id;
    char type = packet->type;
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
    bit_copy((char*) data, 0, array, index, size);
    index += size;

    return 0;
}

int not_main()
{
    packet_t packet1, packet2;
    char data = 42;
    char array[PACKET_MAX_BYTES];
    bit_copy(&data, 0, packet1.data, 0, 8);
    packet1.dest_address = SERVER;
    packet1.origin_address = CLIENT;
    packet1.data_size = sizeof(char)*8;
    packet1.type = 0;
    packet1.packet_id = 0;

    make_packet_array(array, &packet1);

    get_packet_from_array(array, &packet2);

    char* new_data = (char*) (packet2.data);
    printf("%d", *new_data);

    return 0;
}