#include "bitwise.h"

void bit_copy(char* src, int src_start, char* dest, int dest_start, size_t n)
{
    int counter;
    for(counter = 0; counter < n; counter++)
    {
        if (BITTEST(src, src_start + counter))
        {
            // printf("Set");
            BITSET(dest, dest_start + counter);
        }
        else
        {
            // printf("Clear");
            BITCLEAR(dest, dest_start + counter);
        }
    }
}


void print_bits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

void print_bits_to_str(size_t const size, void const * const ptr, char* str)
{
    strcpy(str, "");

    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    int len = 0;

    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            len += sprintf(str+len, "%u", byte);
        }
    }
}