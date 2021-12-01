#ifndef __PACKET__
#define __PACKET__

#include "bitwise.h"


#define PACKET_MAX 46
#define HEADER 0b01111110
#define SERVER 0b10
#define CLIENT 0b01

typedef struct{
    unsigned char header;
    unsigned char origin_address;
    unsigned char dest_address;
    unsigned char data_size;
    unsigned char packet_id;
    unsigned char type;
    char data[BITNSLOTS(16)];
    unsigned char parity; 
} packet_t;

int make_packet_array(char* array, packet_t* packet);

int get_packet_from_array(char* array, packet_t* packet);

#endif