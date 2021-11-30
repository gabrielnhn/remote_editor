#include "network.h"
#include "bitwise.h"

#define HEADER 0b01111110
#define SERVER 0b10
#define CLIENT 0b01

// typedef struct{
//     unsigned header: 8;
//     unsigned dest_address: 2;
//     unsigned origin_address: 2;
//     unsigned data_size: 4;
//     unsigned packet_id: 3;
//     unsigned type: 4;
//     char data_and_parity[6];
//     // unsigned data_and_parity: (16 + 8);
// } packet_t;

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

int get_parity(packet_t* packet)
{
    char str[100] = "";
    print_bits_to_str(sizeof(packet_t), packet, str);
    // print_bits(sizeof(packet_t), packet);
    printf("%s", str);
    return 0;
}

int make_packet_array(char* packet, char origin, char dest, char size, char id, char type, void* data)
{
    packet = malloc(sizeof(BITNSLOTS(50)));
    if (packet == NULL)
        return -1;

    // EMPTY PACKET!
    memset(packet, 0, sizeof(BITNSLOTS(50)));
    // print_bits(sizeof(BITNSLOTS(50)), packet);
    // printf("\n");

    // SET HEADER
    char h = HEADER;
    // print_bits(sizeof(h), &h);    
    // printf("\n");
    bit_copy((char*) &h, 0, packet, 0, 8);

    // SET ORIGIN
    bit_copy((char*) &origin, 0, packet, 8, 2);

    print_bits(sizeof(BITNSLOTS(50)), packet);
    return 0;
}

int main()
{
    // packet_t packet;
    int data = 42;
    char* packet = NULL;
    make_packet_array(packet, CLIENT, SERVER, sizeof(data), 0, 0, &data);
    // printf("%d", sizeof(data));
    return 0;
}