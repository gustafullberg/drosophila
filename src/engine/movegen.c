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
    /* The move generation of bishops and rooks is inspired by Nagaskaki */
    /* http://www.mayothi.com/nagaskakichess6.html                       */
    bitboard_t occupied;
    bitboard_t ul_moves, ur_moves, dl_moves, dr_moves;
    bitboard_t bishop_moves;
    
    occupied = own | opponent;
    
    /* UP-LEFT */
    ul_moves = bitboard_up_left[position] & occupied;
    ul_moves = ul_moves << 7 |
               ul_moves << 14 |
               ul_moves << 21 |
               ul_moves << 28 |
               ul_moves << 35 |
               ul_moves << 42;
    ul_moves &= bitboard_up_left[position];
    ul_moves ^= bitboard_up_left[position];
    ul_moves &= ~own;
    
    /* UP-RIGHT */
    ur_moves = bitboard_up_right[position] & occupied;
    ur_moves = ur_moves << 9 |
               ur_moves << 18 |
               ur_moves << 27 |
               ur_moves << 36 |
               ur_moves << 45 |
               ur_moves << 54;
    ur_moves &= bitboard_up_right[position];
    ur_moves ^= bitboard_up_right[position];
    ur_moves &= ~own;
    
    /* DOWN-LEFT */
    dl_moves = bitboard_down_left[position] & occupied;
    dl_moves = dl_moves >> 9 |
               dl_moves >> 18 |
               dl_moves >> 27 |
               dl_moves >> 36 |
               dl_moves >> 45 |
               dl_moves >> 54;
    dl_moves &= bitboard_down_left[position];
    dl_moves ^= bitboard_down_left[position];
    dl_moves &= ~own;
    
    /* DOWN-RIGHT */
    dr_moves = bitboard_down_right[position] & occupied;
    dr_moves = dr_moves >> 7 |
               dr_moves >> 14 |
               dr_moves >> 21 |
               dr_moves >> 28 |
               dr_moves >> 35 |
               dr_moves >> 42;
    dr_moves &= bitboard_down_right[position];
    dr_moves ^= bitboard_down_right[position];
    dr_moves &= ~own;
    
    bishop_moves = ul_moves | ur_moves | dl_moves | dr_moves;
    
    *moves = bishop_moves & ~opponent;
    *captures = bishop_moves & opponent;
}

void MOVEGEN_rook(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    /* The move generation of bishops and rooks is inspired by Nagaskaki */
    /* http://www.mayothi.com/nagaskakichess6.html                       */
    bitboard_t occupied;
    bitboard_t left_moves, right_moves, up_moves, down_moves;
    bitboard_t rook_moves;
    
    occupied = own | opponent;
    
    /* LEFT */
    left_moves = bitboard_left[position] & occupied;
    left_moves = left_moves >> 1 |
                 left_moves >> 2 |
                 left_moves >> 3 |
                 left_moves >> 4 |
                 left_moves >> 5 |
                 left_moves >> 6;
    left_moves &= bitboard_left[position];
    left_moves ^= bitboard_left[position];
    left_moves &= ~own;
    
    /* RIGHT */
    right_moves = bitboard_right[position] & occupied;
    right_moves = right_moves << 1 |
                  right_moves << 2 |
                  right_moves << 3 |
                  right_moves << 4 |
                  right_moves << 5 |
                  right_moves << 6;
    right_moves &= bitboard_right[position];
    right_moves ^= bitboard_right[position];
    right_moves &= ~own;
    
    /* UP */
    up_moves = bitboard_up[position] & occupied;
    up_moves = up_moves << 8 |
               up_moves << 16 |
               up_moves << 24 |
               up_moves << 32 |
               up_moves << 40 |
               up_moves << 48;
    up_moves &= bitboard_up[position];
    up_moves ^= bitboard_up[position];
    up_moves &= ~own;
    
    /* DOWN */
    down_moves = bitboard_down[position] & occupied;
    down_moves = down_moves >> 8 |
               down_moves >> 16 |
               down_moves >> 24 |
               down_moves >> 32 |
               down_moves >> 40 |
               down_moves >> 48;
    down_moves &= bitboard_down[position];
    down_moves ^= bitboard_down[position];
    down_moves &= ~own;
    
    rook_moves = left_moves | right_moves | up_moves | down_moves;
    *moves = rook_moves & ~opponent;
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
