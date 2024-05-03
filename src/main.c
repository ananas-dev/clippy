#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "bitboard.h"

uint64_t random_u64() {
    static uint64_t number = 0xFFAAB58C5833FE89;

    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    return number;
}

typedef struct {
    Bitboard *ptr;
    Bitboard mask;
    uint64_t magic;
    uint8_t offset;
} Magic;

Magic magics[2][64];
Bitboard attack_table[107648];

Bitboard rook_attacks_slow(Square sq, Bitboard blockers) {
    Bitboard moves = BB_EMPTY;

    int start_rank = sq_rank(sq);
    int start_file = sq_file(sq);

    for (int rank = start_rank + 1; rank <= 7; rank++) {
        moves |= bb_from_sq(sq_get(start_file, rank));

        if (blockers & bb_from_sq(sq_get(start_file, rank))) {
            break;
        }
    }

    for (int rank = start_rank - 1; rank >= 0; rank--) {
        moves |= bb_from_sq(sq_get(start_file, rank));

        if (blockers & bb_from_sq(sq_get(start_file, rank))) {
            break;
        }
    }

    for (int file = start_file + 1; file <= 7; file++) {
        moves |= bb_from_sq(sq_get(file, start_rank));

        if (blockers & bb_from_sq(sq_get(file, start_rank))) {
            break;
        }
    }

    for (int file = start_file - 1; file >= 0; file--) {
        moves |= bb_from_sq(sq_get(file, start_rank));

        if (blockers & bb_from_sq(sq_get(file, start_rank))) {
            break;
        }
    }

    return moves;
};

Bitboard bishop_attacks_slow(Square sq, Bitboard blockers) {
    Bitboard moves = BB_EMPTY;

    int start_rank = sq_rank(sq);
    int start_file = sq_file(sq);

    for (int off = 1; start_file + off <= 7 && start_rank + off <= 7; off++) {
        moves |= bb_from_sq(sq_get(start_file + off, start_rank + off));

        if (blockers & bb_from_sq(sq_get(start_file + off, start_rank + off))) {
            break;
        }
    }

    for (int off = 1; start_file - off >= 0 && start_rank - off >= 0; off++) {
        moves |= bb_from_sq(sq_get(start_file - off, start_rank - off));

        if (blockers & bb_from_sq(sq_get(start_file - off, start_rank - off))) {
            break;
        }
    }

    for (int off = 1; start_file + off <= 7 && start_rank - off >= 0; off++) {
        moves |= bb_from_sq(sq_get(start_file + off, start_rank - off));

        if (blockers & bb_from_sq(sq_get(start_file + off, start_rank - off))) {
            break;
        }
    }

    for (int off = 1; start_file - off >= 0 && start_rank + off <= 7; off++) {
        moves |= bb_from_sq(sq_get(start_file - off, start_rank + off));

        if (blockers & bb_from_sq(sq_get(start_file - off, start_rank + off))) {
            break;
        }
    }


    return moves;
};

Bitboard bishop_mask(Square sq) {
    Bitboard moves = BB_EMPTY;

    int start_rank = sq_rank(sq);
    int start_file = sq_file(sq);

    for (int off = 1; start_file + off <= 6 && start_rank + off <= 6; off++) {
        moves |= bb_from_sq(sq_get(start_file + off, start_rank + off));
    }

    for (int off = 1; start_file - off >= 1 && start_rank - off >= 1; off++) {
        moves |= bb_from_sq(sq_get(start_file - off, start_rank - off));
    }

    for (int off = 1; start_file + off <= 6 && start_rank - off >= 1; off++) {
        moves |= bb_from_sq(sq_get(start_file + off, start_rank - off));
    }

    for (int off = 1; start_file - off >= 1 && start_rank + off <= 6; off++) {
        moves |= bb_from_sq(sq_get(start_file - off, start_rank + off));
    }

    return moves;
};

Bitboard rook_mask(Square sq) {
    Bitboard moves = BB_EMPTY;

    int start_rank = sq_rank(sq);
    int start_file = sq_file(sq);

    for (int rank = start_rank + 1; rank <= 6; rank++) {
        moves |= bb_from_sq(sq_get(start_file, rank));
    }

    for (int rank = start_rank - 1; rank >= 1; rank--) {
        moves |= bb_from_sq(sq_get(start_file, rank));
    }

    for (int file = start_file + 1; file <= 6; file++) {
        moves |= bb_from_sq(sq_get(file, start_rank));
    }

    for (int file = start_file - 1; file >= 1; file--) {
        moves |= bb_from_sq(sq_get(file, start_rank));
    }
    return moves;
};



size_t magic_index(Magic *entry, Bitboard blockers) {
    blockers &= entry->mask;
    blockers *= entry->magic;
    blockers >>= entry->offset;

    return blockers;
}

Bitboard rook_attacks(Square sq, Bitboard blockers) {
    Magic *entry = &magics[SLIDER_ROOK][sq];
    return entry->ptr[magic_index(entry, blockers)];
}

Bitboard bishop_attacks(Square sq, Bitboard blockers) {
    Magic *entry = &magics[SLIDER_BISHOP][sq];
    return entry->ptr[magic_index(entry, blockers)];
}


bool try_magic(Slider slider, Square sq, Magic *entry) {
    Bitboard blockers = BB_EMPTY;

    memset(entry->ptr, 0, (1 << (64 - entry->offset)) * sizeof(Bitboard));

    for (;;) {
        Bitboard moves;

        if (slider == SLIDER_ROOK) {
            moves = rook_attacks_slow(sq, blockers);
        } else if (slider == SLIDER_BISHOP) {
            moves = bishop_attacks_slow(sq, blockers);
        } else {
            // TODO: proper exit
            assert(false);
        }

        Bitboard *table_entry = &entry->ptr[magic_index(entry, blockers)];

        if (*table_entry == BB_EMPTY) {
            *table_entry = moves;
        } else if (*table_entry != moves) {
            return false;
        }

        blockers |= ~(entry->mask);
        blockers += 1ULL;
        blockers &= entry->mask;

        if (blockers == BB_EMPTY) {
            return true;
        }
    }
}

void init_magics() {
    Bitboard *ptr = attack_table;
    for (Slider slider = SLIDER_ROOK; slider <= SLIDER_BISHOP; slider++) {
        for (Square sq = SQ_A1; sq <= SQ_H8; sq++) {
            Bitboard mask;
            if (slider == SLIDER_ROOK) {
                mask = rook_mask(sq);
            } else if (slider == SLIDER_BISHOP) {
                mask = bishop_mask(sq);
            } else {
                // TODO: proper exit
                assert(false);
            }
            uint8_t index_bits = bb_popcnt(mask);

            for (;;) {
                uint64_t magic = random_u64() & random_u64() & random_u64();

                if (bb_popcnt((mask * magic) & 0xFF00000000000000ULL) < 6) {
                    continue;
                }

                Magic entry = {
                    .mask = mask,
                    .magic = magic,
                    .offset = 64 - index_bits,
                    .ptr = ptr,
                };

                if (try_magic(slider, sq, &entry)) {
                    magics[slider][sq] = entry;
                    ptr += (1 << (64 - entry.offset));
                    break;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    init_magics();

    Bitboard blockers = BB_EMPTY;

    blockers = BB_EMPTY;
    blockers |= bb_from_sq(SQ_E5);
    blockers |= bb_from_sq(SQ_E2);
    blockers |= bb_from_sq(SQ_C2);
    blockers |= bb_from_sq(SQ_F3);

    bb_print(rook_attacks(SQ_E4, blockers));
    bb_print(bishop_attacks(SQ_E4, blockers));

    return 0;
}