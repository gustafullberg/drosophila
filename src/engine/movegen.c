#include <assert.h>
#include <immintrin.h>
#include <stdio.h>
#include "defines.h"
#include "movegen.h"

void MOVEGEN_all_pawns(const int color, const bitboard_t pawns, const bitboard_t own, const bitboard_t opponent, bitboard_t *pawn_push, bitboard_t *pawn_push2, bitboard_t *pawn_capture_from_left, bitboard_t *pawn_capture_from_right, bitboard_t *pawn_promotion, bitboard_t *pawn_promotion_capture_from_left, bitboard_t *pawn_promotion_capture_from_right)
{
    bitboard_t empty = ~own & ~opponent;

    if(color == WHITE) {
        *pawn_push = (pawns << 8) & empty;
        *pawn_push2 = ((pawns &  (BITBOARD_RANK << 8)) << 16) & empty & (empty << 8);
	*pawn_capture_from_left = ((pawns & ~(BITBOARD_FILE << 7)) << 9) & opponent;
	*pawn_capture_from_right = ((pawns & ~(BITBOARD_FILE << 0)) << 7) & opponent;
        *pawn_promotion = *pawn_push & (BITBOARD_RANK << 56);
        *pawn_promotion_capture_from_left = *pawn_capture_from_left & (BITBOARD_RANK << 56);
	*pawn_promotion_capture_from_right = *pawn_capture_from_right & (BITBOARD_RANK << 56);
    } else {
        *pawn_push = (pawns >> 8) & empty;
        *pawn_push2 = ((pawns &  (BITBOARD_RANK << 48)) >> 16) & empty & (empty >> 8);
	*pawn_capture_from_left = ((pawns & ~(BITBOARD_FILE << 7)) >> 7) & opponent;
	*pawn_capture_from_right = ((pawns & ~(BITBOARD_FILE << 0)) >> 9) & opponent;
        *pawn_promotion = *pawn_push & (BITBOARD_RANK << 0);
	*pawn_promotion_capture_from_left = *pawn_capture_from_left & (BITBOARD_RANK << 0);
	*pawn_promotion_capture_from_right = *pawn_capture_from_right & (BITBOARD_RANK << 0);
    }

    *pawn_push ^= *pawn_promotion;
    *pawn_capture_from_left ^= *pawn_promotion_capture_from_left;
    *pawn_capture_from_right ^= *pawn_promotion_capture_from_right;
}

void MOVEGEN_knight(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
    *captures = bitboard_knight[position] & opponent;
    *moves = bitboard_knight[position] & ~(own | opponent);
}

void MOVEGEN_bishop(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
#ifdef __BMI2__
    bitboard_t occupied = own | opponent;
    bitboard_t all_moves = bitboard_bishop_attacks[bishop_attacks_offset[position] + _pext_u64(occupied, bitboard_bishop_inner[position])];
    *moves = all_moves & ~occupied;
    *captures = all_moves & opponent;
#else
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
#endif
}

void MOVEGEN_rook(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
#ifdef __BMI2__
    bitboard_t occupied = own | opponent;
    bitboard_t all_moves = bitboard_rook_attacks[rook_attacks_offset[position] + _pext_u64(occupied, bitboard_rook_inner[position])];
    *moves = all_moves & ~occupied;
    *captures = all_moves & opponent;
#else
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
#endif
}

void MOVEGEN_queen(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures)
{
#ifdef __BMI2__
    bitboard_t occupied = own | opponent;
    bitboard_t all_moves = bitboard_rook_attacks[rook_attacks_offset[position] + _pext_u64(occupied, bitboard_rook_inner[position])];
    all_moves |= bitboard_bishop_attacks[bishop_attacks_offset[position] + _pext_u64(occupied, bitboard_bishop_inner[position])];
    *moves = all_moves & ~occupied;
    *captures = all_moves & opponent;
#else
    bitboard_t moves_tmp, captures_tmp;
    MOVEGEN_bishop(position, own, opponent, moves, captures);
    MOVEGEN_rook(position, own, opponent, &moves_tmp, &captures_tmp);
    *moves |= moves_tmp;
    *captures |= captures_tmp;
#endif
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
        /* MOVEGEN_all_pawns should be used instead */
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
