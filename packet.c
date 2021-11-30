#include "network.h"
#include "bitwise.h"

#define HEADER 0b01111110
#define SERVER 0b10
#define CLIENT 0b01

#define PACKET_MAX 46

typedef struct{
    unsigned char header;
    unsigned char origin_address;
    unsigned char dest_address;
    unsigned char data_size;
    unsigned char packet_id;
    unsigned char type;
    long data;
    unsigned char parity; 
} packet_t;

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

int make_packet_array(char* packet, char dest, char origin, char size, char id, char type, void* data)
{
    int index = 0;

    // EMPTY PACKET!
    memset(packet, 0, BITNSLOTS(PACKET_MAX));
    // print_bits(sizeof(BITNSLOTS(PACKET_MAX)), packet);
    // printf("\n");

    // SET HEADER
    char h = HEADER;
    // print_bits(sizeof(h), &h);    
    // printf("\n");
    bit_copy((char*) &h, 0, packet, 0, 8);

    index += 8;

    // SET DEST
    bit_copy((char*) &dest, 0, packet, index, 2);
    index += 2;  

    // SET ORIGIN
    bit_copy((char*) &origin, 0, packet, index, 2);
    index += 2;

    // SET SIZE
    bit_copy((char*) &size, 0, packet, index, 4);
    index += 4;

    // SET ID
    bit_copy((char*) &id, 0, packet, index, 4);
    index += 4;

    // SET TYPE
    bit_copy((char*) &type, 0, packet, index, 4);
    index += 4;

    // SET DATA
    bit_copy((char*) data, 0, packet, index, size);
    index += size;

    print_bits(BITNSLOTS(PACKET_MAX), packet);
    return 0;
}

int main()
{
    // packet_t packet;
    char data = 42;
    char packet[BITNSLOTS(PACKET_MAX)];
    make_packet_array(packet, SERVER, CLIENT, sizeof(char)*8, 0, 0, &data);
    // printf("%d", sizeof(data));
    return 0;
}