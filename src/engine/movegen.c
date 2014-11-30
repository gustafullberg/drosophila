#include <assert.h>
#include <stdio.h>
#include "defines.h"
#include "movegen.h"

void MOVEGEN_pawn(const int color, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *pawn_push, bitboard_t *pawn_push2, bitboard_t *pawn_capture, bitboard_t *pawn_promotion, bitboard_t *pawn_capture_promotion)
{
    bitboard_t empty;
    
    empty = ~own & ~opponent;
    
    /* One push */
    *pawn_push = bitboard_pawn_move[color][position] & empty;
    
    /* Two push */
    *pawn_push2 = empty & bitboard_pawn_move2[color][position] & ((*pawn_push << 8) | (*pawn_push >> 8));
    
    /* Capture */
    *pawn_capture = bitboard_pawn_capture[color][position] & opponent;
    
    /* Promotion, no capture */
    *pawn_promotion = *pawn_push & BITBOARD_PROMOTION;
    *pawn_push &= ~BITBOARD_PROMOTION;
    
    /* Promotion, capture */
    *pawn_capture_promotion = *pawn_capture & BITBOARD_PROMOTION;
    *pawn_capture &= ~BITBOARD_PROMOTION;
}

void MOVEGEN_knight(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    *captures = bitboard_knight[position] & opponent;
    *moves = bitboard_knight[position] & ~(own | opponent);
}

void MOVEGEN_bishop(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    bitboard_t occupied;
    bitboard_t ul_moves, ur_moves, dl_moves, dr_moves;
    bitboard_t bishop_moves;
    bitboard_t blockers;
    int blocker_pos;
    
    occupied = own | opponent;
    
    /* UP-LEFT */
    ul_moves = bitboard_up_left[position];
    blockers = ul_moves & occupied;
    blocker_pos = BITBOARD_find_bit(blockers | (bitboard_t)0x8000000000000000);
    ul_moves ^= bitboard_up_left[blocker_pos];
    
    /* UP-RIGHT */
    ur_moves = bitboard_up_right[position];
    blockers = ur_moves & occupied;
    blocker_pos = BITBOARD_find_bit(blockers | (bitboard_t)0x8000000000000000);
    ur_moves ^= bitboard_up_right[blocker_pos];

    /* DOWN-LEFT */
    dl_moves = bitboard_down_left[position];
    blockers = dl_moves & occupied;
    blocker_pos = BITBOARD_find_bit_reversed(blockers | 1);
    dl_moves ^= bitboard_down_left[blocker_pos];

    /* DOWN-RIGHT */
    dr_moves = bitboard_down_right[position];
    blockers = dr_moves & occupied;
    blocker_pos = BITBOARD_find_bit_reversed(blockers | 1);
    dr_moves ^= bitboard_down_right[blocker_pos];
    
    bishop_moves = ul_moves | ur_moves | dl_moves | dr_moves;
    
    *moves = bishop_moves & ~occupied;
    *captures = bishop_moves & opponent;
}

void MOVEGEN_rook(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    bitboard_t occupied;
    bitboard_t left_moves, right_moves, up_moves, down_moves;
    bitboard_t rook_moves;
    bitboard_t blockers;
    int blocker_pos;
    
    occupied = own | opponent;
    
    /* LEFT */
    left_moves = bitboard_left[position];
    blockers = left_moves & occupied;
    blocker_pos = BITBOARD_find_bit_reversed(blockers | 1);
    left_moves ^= bitboard_left[blocker_pos];

    /* RIGHT */
    right_moves = bitboard_right[position];
    blockers = right_moves & occupied;
    blocker_pos = BITBOARD_find_bit(blockers | (bitboard_t)0x8000000000000000);
    right_moves ^= bitboard_right[blocker_pos];
    
    /* UP */
    up_moves = bitboard_up[position];
    blockers = up_moves & occupied;
    blocker_pos = BITBOARD_find_bit(blockers | (bitboard_t)0x8000000000000000);
    up_moves ^= bitboard_up[blocker_pos];
    
    /* DOWN */
    down_moves = bitboard_down[position];
    blockers = down_moves & occupied;
    blocker_pos = BITBOARD_find_bit_reversed(blockers | 1);
    down_moves ^= bitboard_down[blocker_pos];
    
    rook_moves = left_moves | right_moves | up_moves | down_moves;
    *moves = rook_moves & ~occupied;
    *captures = rook_moves & opponent;
}

void MOVEGEN_queen(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    bitboard_t moves_tmp, captures_tmp;
    MOVEGEN_bishop(position, own, opponent, moves, captures);
    MOVEGEN_rook(position, own, opponent, &moves_tmp, &captures_tmp);
    *moves |= moves_tmp;
    *captures |= captures_tmp;
}

void MOVEGEN_king(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    *moves = bitboard_king[position] & ~own & ~opponent;
    *captures = bitboard_king[position] & opponent;
}

void MOVEGEN_piece(const int type, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    switch(type) {
    case PAWN:
        /* MOVEGEN_pawn should be used instead */
        assert(0);
        break;
    case KNIGHT:
        MOVEGEN_knight(position, own, opponent, moves, captures);
        break;
    case BISHOP:
        MOVEGEN_bishop(position, own, opponent, moves, captures);
        break;
    case ROOK:
        MOVEGEN_rook(position, own, opponent, moves, captures);
        break;
    case QUEEN:
        MOVEGEN_queen(position, own, opponent, moves, captures);
        break;
    case KING:
        MOVEGEN_king(position, own, opponent, moves, captures);
        break;
    default:
        break;
    }
}
