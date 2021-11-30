#include "network.h"
#include "bitwise.h"

#define HEADER 0b01111110
#define SERVER 0b10
#define CLIENT 0b01

typedef struct{
    unsigned header: 8;
    unsigned dest_address: 2;
    unsigned origin_address: 2;
    unsigned data_size: 4;
    unsigned packet_id: 3;
    unsigned type: 4;
    unsigned data_and_parity: (16 + 8);
} packet_t;

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

}

int make_packet(packet_t* packet, int origin, int dest, int size, int id, int type, void* data)
{
    packet->header = HEADER;
    packet->origin_address = origin;
    packet->dest_address = dest;
    packet->data_size = size;
    packet->packet_id = id;
    packet->type = type;
    packet->data_and_parity = data;

    

    packet->data_and_parity = 15;

    print_bits(sizeof(packet_t), &packet);
    return 0;
}

int main()
{
    make_packet();
    return 0;
}