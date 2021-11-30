#ifndef __BITWISE__
#define __BITWISE__

#include <sys/types.h>
#include <limits.h>     /* for CHAR_BIT */
#include <string.h>
#include <stdio.h>


// auxiliar macros
#define BITMASK(b)      (1 << ((b) % CHAR_BIT))           // generate bit mask
#define BITSLOT(b)      ((b) / CHAR_BIT)                  // in which cell?
#define BITNSLOTS(nb)   ((nb + CHAR_BIT - 1) / CHAR_BIT)  // number of cells

// bit operations
#define BITSET(array, b)    ((array)[BITSLOT(b)] |=  BITMASK(b))  // bit = 1
#define BITCLEAR(array, b)  ((array)[BITSLOT(b)] &= ~BITMASK(b))  // bit = 0
#define BITTEST(array, b)   ((array)[BITSLOT(b)] &   BITMASK(b))  // bit == 1 ?
#define BITTOGGLE(array, b) ((array)[BITSLOT(b)] ^=  BITMASK(b))  // bit = ^bit

// Exemplos de uso dessa implementação:

// char bitarray[BITNSLOTS(47)];           // declarar um mapa de 47 bits
// BITSET(bitarray, 23);                   // ligar  o bit 23
// if (BITTEST(bitarray, 35)) { ... }      // testar o bit 35

void print_bits(size_t const size, void const * const ptr);

void print_bits_to_str(size_t const size, void const * const ptr, char* str);

void bit_copy(char* src, int src_start, char* dest, int dest_start, size_t n);

#endif