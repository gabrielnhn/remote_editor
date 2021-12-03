#ifndef __PACKET__
#define __PACKET__

#include "bitwise.h"
#include <stdbool.h>

#define DATA_BYTES 15
#define PACKET_MAX_BYTES (6 + DATA_BYTES)
#define HEADER 0b01111110
#define SERVER 0b10
#define CLIENT 0b01


// types
#define CD 0b0000
#define ACK 0b1000
#define NACK 0b1001
#define LCD 2
#define LS 3
#define LLS 4
#define VER 5
#define LINHA 6
#define LINHAS 7
#define EDIT 8
#define COMPILAR 9

#define LS_CONTENT 0b1011
#define ERROR 0b1111
#define END 0b1101


typedef struct{
    unsigned char header;
    unsigned char origin_address;
    unsigned char dest_address;
    unsigned char data_size;
    unsigned char packet_id;
    unsigned char type;
    char data[DATA_BYTES];
    unsigned char parity; 
} packet_t;

bool valid_packet(packet_t* packet, int id);

int make_packet_array(char* array, packet_t* packet);

int get_packet_from_array(char* array, packet_t* packet);

unsigned char get_parity(packet_t* packet);

void print_packet(packet_t* packet);


#endif