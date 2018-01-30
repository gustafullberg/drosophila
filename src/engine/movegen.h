#ifndef MOVE_H
#define MOVE_H

#include "bitboard.h"

void MOVEGEN_all_pawns(const int color, const bitboard_t pawns, const bitboard_t own, const bitboard_t opponent, bitboard_t *pawn_push, bitboard_t *pawn_push2, bitboard_t *pawn_capture_from_left, bitboard_t *pawn_capture_from_right, bitboard_t *pawn_promotion, bitboard_t *pawn_promotion_capture_from_left, bitboard_t *pawn_promotion_capture_from_right);
void MOVEGEN_knight(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_bishop(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_rook(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_queen(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_king(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_piece(const int type, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);

#endif

