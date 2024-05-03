#ifndef BITBOARD_H
#define BITBOARD_H

#include "core.h"

#include <stdint.h>

typedef uint64_t Bitboard;

#define BB_EMPTY              0x0000000000000000ULL
#define BB_FULL               0xFFFFFFFFFFFFFFFFULL
#define BB_A_FILE             0x00000000000000FFULL
#define BB_H_FILE             0xFF00000000000000ULL
#define BB_FIRST_RANK         0x0101010101010101ULL
#define BB_EIGHTH_RANK        0x8080808080808080ULL
#define BB_A1_H8_DIAGONAL     0x8040201008040201ULL
#define BB_H1_A8_ANTIDIAGONAL 0x0102040810204080ULL
#define BB_LIGHT_SQUARES      0x55AA55AA55AA55AAULL
#define BB_DARK_SQUARES       0xAA55AA55AA55AA55ULL

Bitboard bb_from_sq(Square sq);
int bb_popcnt(Bitboard x);
void bb_print(Bitboard bb);

#endif