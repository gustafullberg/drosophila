#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif
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

#define BITBOARD_WHITE_SQ ((bitboard_t)0x55AA55AA55AA55AA)
#define BITBOARD_BLACK_SQ ((bitboard_t)0xAA55AA55AA55AA55)

#define BITBOARD_CLEAR(bitboard, pos) (bitboard &= ~BITBOARD_POSITION(pos))
#define BITBOARD_SET(bitboard, pos) (bitboard |= BITBOARD_POSITION(pos))

extern bitboard_t bitboard_file[NUM_POSITIONS];
extern bitboard_t bitboard_rank[NUM_POSITIONS];
extern bitboard_t bitboard_left[NUM_POSITIONS];
extern bitboard_t bitboard_right[NUM_POSITIONS];
extern bitboard_t bitboard_up[NUM_POSITIONS];
extern bitboard_t bitboard_down[NUM_POSITIONS];
extern bitboard_t bitboard_up_left[NUM_POSITIONS];
extern bitboard_t bitboard_up_right[NUM_POSITIONS];
extern bitboard_t bitboard_down_left[NUM_POSITIONS];
extern bitboard_t bitboard_down_right[NUM_POSITIONS];
extern bitboard_t bitboard_between[NUM_POSITIONS][NUM_POSITIONS];
extern bitboard_t bitboard_king[NUM_POSITIONS];
extern bitboard_t bitboard_knight[NUM_POSITIONS];
extern bitboard_t bitboard_pawn_move[NUM_COLORS][NUM_POSITIONS];
extern bitboard_t bitboard_pawn_capture[NUM_COLORS][NUM_POSITIONS];
extern bitboard_t bitboard_ep_capture[NUM_POSITIONS];
extern bitboard_t bitboard_ep_capturers[NUM_COLORS][NUM_FILES];
extern bitboard_t bitboard_bishop[NUM_POSITIONS];
extern bitboard_t bitboard_rook[NUM_POSITIONS];
extern bitboard_t bitboard_king_castle_empty[NUM_COLORS];
extern bitboard_t bitboard_queen_castle_empty[NUM_COLORS];
extern bitboard_t bitboard_start_position[NUM_COLORS][NUM_TYPES-1];
extern const bitboard_t bitboard_zobrist[NUM_COLORS][NUM_TYPES-1][NUM_POSITIONS];
extern const bitboard_t bitboard_zobrist_color;
extern const bitboard_t bitboard_zobrist_ep[NUM_FILES+1];
extern const bitboard_t bitboard_zobrist_castling[NUM_COLORS][4];
extern char       distance[NUM_POSITIONS][NUM_POSITIONS];
#ifdef __BMI2__
extern bitboard_t bitboard_rook_inner[NUM_POSITIONS];
extern bitboard_t bitboard_bishop_inner[NUM_POSITIONS];
extern bitboard_t bitboard_rook_attacks[102400];
extern bitboard_t bitboard_bishop_attacks[5248];
extern int rook_attacks_offset[NUM_POSITIONS];
extern int bishop_attacks_offset[NUM_POSITIONS];
#endif

void BITBOARD_init();
void BITBOARD_print_debug(const bitboard_t bitboard);

static inline int BITBOARD_find_bit(const bitboard_t bitboard)
{
#if __GNUC__
    return __builtin_ctzll(bitboard);
#elif _MSC_VER && _WIN64
    unsigned long index;
    _BitScanForward64(&index, bitboard);
    return index;
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

static inline int BITBOARD_find_bit_reversed(const bitboard_t bitboard)
{
#if __GNUC__
    return 63 - __builtin_clzll(bitboard);
#elif _MSC_VER && _WIN64
    unsigned long index;
    _BitScanReverse64(&index, bitboard);
    return index;
#else
    int i;
    for(i = NUM_POSITIONS-1; i >= 0; i++) {
        if(bitboard & ((bitboard_t)1 << i)) {
            return i;
        }
    }
    return 0;
#endif
}

static inline int BITBOARD_count_bits(bitboard_t bitboard)
{
#if __GNUC__
    return __builtin_popcountll(bitboard);
#elif _MSC_VER && _WIN64
    return (int)__popcnt64(bitboard);
#else
    int num_bits = 0;
    while(bitboard) {
        num_bits += bitboard & 1;
        bitboard = bitboard >> 1;
    }
    return num_bits;
#endif
}


static inline bitboard_t BITBOARD_fill_north(bitboard_t b)
{
    b |= b << 8;
    b |= b << 16;
    b |= b << 32;
    return b;
}

static inline bitboard_t BITBOARD_fill_south(bitboard_t b)
{
    b |= b >> 8;
    b |= b >> 16;
    b |= b >> 32;
    return b;
}

#endif

