#ifndef _MOVE_H
#define _MOVE_H

#include <stdint.h>
#include "defines.h"
#include "bitboard.h"

/* Bits 21 - 18 special      */
/* Bits 17 - 15 capture_type */
/* Bits 14 - 12 type         */
/* Bits 11 -  6 pos_to       */
/* Bits  5 -  0 pos_from     */
typedef uint32_t move_t;

/* Move "special" flags */
#define MOVE_QUIET						0x0
#define MOVE_DOUBLE_PAWN_PUSH			0x1
#define MOVE_KING_CASTLE				0x2
#define MOVE_QUEEN_CASTLE				0x3
#define MOVE_CAPTURE					0x4
#define MOVE_EP_CAPTURE					0x5
#define MOVE_KNIGHT_PROMOTION			0x8
#define MOVE_BISHOP_PROMOTION			0x9
#define MOVE_ROOK_PROMOTION				0xA
#define MOVE_QUEEN_PROMOTION			0xB
#define MOVE_KNIGHT_PROMOTION_CAPTURE	0xC
#define MOVE_BISHOP_PROMOTION_CAPTURE	0xD
#define MOVE_ROOK_PROMOTION_CAPTURE		0xE
#define MOVE_QUEEN_PROMOTION_CAPTURE	0xF

#define MOVE_POS_FROM(move) ((move) & 0x3F)
#define MOVE_POS_TO(move)   (((move) >> 6) & 0x3F)
#define MOVE_TYPE(move)     (((move) >> 12) & 0x7)
#define MOVE_OPPONENT_TYPE(move)     (((move) >> 15) & 0x7)
#define MOVE_SPECIAL(move)  (((move) >> 18) & 0xF)

#define MOVE_IS_PROMOTION(move)     (MOVE_SPECIAL(move) & 0x8)
#define MOVE_PROMOTION_TYPE(move)   ((MOVE_SPECIAL(move) & 0xB)-7)

void move_pawn(const int color, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *pawn_push, bitboard_t *pawn_push2, bitboard_t *pawn_capture, bitboard_t *pawn_promotion, bitboard_t *pawn_capture_promotion);
void move_bishop(const int color, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void move_rook(const int color, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);

void move_piece(const int color, const int type, const int position, const bitboard_t own, const bitboard_t opponent, bitboard_t *moves, bitboard_t *captures);
void move_print_debug(const move_t move);

#endif

