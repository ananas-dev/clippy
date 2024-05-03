#include "core.h"
#include "bitboard.h"

typedef struct {
    Bitboard by_color[2];
    Bitboard by_role[6];
} Board;