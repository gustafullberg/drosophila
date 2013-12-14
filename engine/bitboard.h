#ifndef _BITBOARD_H
#define _BITBOARD_H

#include <stdint.h>
#include "defines.h"

typedef uint64_t bitboard_t;

#define BITBOARD_POS_VALID(rank, file) (((rank) >= 0 && (rank) < 8 && (file) >= 0 && (file) < 8) ? (1) : (0))
#define BITBOARD_POSITION(pos) ((bitboard_t)1 << (pos))
#define BITBOARD_RANK_FILE(rank, file) ((bitboard_t)1 << (8*(rank) +(file)))

#define BITBOARD_GET_FILE(pos) ((pos) % 8)
#define BITBOARD_GET_RANK(pos) ((pos) / 8)

#define BITBOARD_FILE (bitboard_t)0x0101010101010101
#define BITBOARD_RANK (bitboard_t)0x00000000000000FF

#define BITBOARD_PROMOTION (bitboard_t)0xFF000000000000FF

#define BITBOARD_CLEAR(bitboard, pos) (bitboard &= ~BITBOARD_POSITION(pos))
#define BITBOARD_SET(bitboard, pos) (bitboard |= BITBOARD_POSITION(pos))

extern bitboard_t bitboard_less_than[NUM_POSITIONS];
extern bitboard_t bitboard_more_than[NUM_POSITIONS];
extern bitboard_t bitboard_file[NUM_POSITIONS];
extern bitboard_t bitboard_rank[NUM_POSITIONS];
extern bitboard_t bitboard_bltr[NUM_POSITIONS];
extern bitboard_t bitboard_tlbr[NUM_POSITIONS];
extern bitboard_t bitboard_left[NUM_POSITIONS];
extern bitboard_t bitboard_right[NUM_POSITIONS];
extern bitboard_t bitboard_up[NUM_POSITIONS];
extern bitboard_t bitboard_down[NUM_POSITIONS];
extern bitboard_t bitboard_up_left[NUM_POSITIONS];
extern bitboard_t bitboard_up_right[NUM_POSITIONS];
extern bitboard_t bitboard_down_left[NUM_POSITIONS];
extern bitboard_t bitboard_down_right[NUM_POSITIONS];
extern bitboard_t bitboard_king[NUM_POSITIONS];
extern bitboard_t bitboard_knight[NUM_POSITIONS];
extern bitboard_t bitboard_pawn_move[NUM_COLORS][NUM_POSITIONS];
extern bitboard_t bitboard_pawn_move2[NUM_COLORS][NUM_POSITIONS];
extern bitboard_t bitboard_pawn_capture[NUM_COLORS][NUM_POSITIONS];
extern bitboard_t bitboard_ep_capture[NUM_POSITIONS];
extern bitboard_t bitboard_ep_capturers[NUM_COLORS][NUM_FILES];
extern bitboard_t bitboard_king_castle_empty[NUM_COLORS];
extern bitboard_t bitboard_queen_castle_empty[NUM_COLORS];
extern bitboard_t bitboard_start_position[NUM_COLORS][NUM_TYPES-1];
extern bitboard_t bitboard_bad_pawn[NUM_COLORS];
extern bitboard_t bitboard_bad_knight[NUM_COLORS];
extern bitboard_t bitboard_bad_bishop[NUM_COLORS];
extern bitboard_t bitboard_zobrist[NUM_COLORS][NUM_TYPES-1][NUM_POSITIONS];
extern bitboard_t bitboard_zorbist_color;

void bitboard_init();
void bitboard_print_debug(bitboard_t bitboard);

static inline int bitboard_find_bit(bitboard_t bitboard)
{
#if __GNUC__
    return __builtin_ctzll(bitboard);
#else
    int i;
    for(i = 0; i < NUM_POSITIONS; i++) {
        if(bitboard & ((bitboard_t)1 << i)) {
            return i;
        }
    }
    return 0;
#endif
}

static inline int bitboard_count_bits(bitboard_t bitboard)
{
#if __GNUC__
    return __builtin_popcountll(bitboard);
#else
    int num_bits = 0;
    while(bitboard) {
        num_bits += bitboard & 1;
        bitboard = bitboard >> 1;
    }
    return num_bits;
#endif
}

#endif

