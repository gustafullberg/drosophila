#ifndef _MOVE_H
#define _MOVE_H

#include "bitboard.h"

void MOVEGEN_pawn(const int color, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *pawn_push, bitboard_t *pawn_push2, bitboard_t *pawn_capture, bitboard_t *pawn_promotion, bitboard_t *pawn_capture_promotion);
void MOVEGEN_knight(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_bishop(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_rook(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_queen(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_king(const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void MOVEGEN_piece(const int type, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);

#endif

