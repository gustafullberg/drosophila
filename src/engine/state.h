#ifndef STATE_H
#define STATE_H

#include "defines.h"
#include "bitboard.h"

/* Type describing a single move */
typedef uint32_t move_t;
/* Bits 32 - 22 score        */
/* Bits 21 - 18 special      */
/* Bits 17 - 15 capture_type */
/* Bits 14 - 12 type         */
/* Bits 11 -  6 pos_to       */
/* Bits  5 -  0 pos_from     */

#define MOVE_POS_FROM_SHIFT             0
#define MOVE_POS_FROM_MASK              (0x3F<<(MOVE_POS_FROM_SHIFT))

#define MOVE_POS_TO_SHIFT               6
#define MOVE_POS_TO_MASK                (0x3F<<(MOVE_POS_TO_SHIFT))

#define MOVE_TYPE_SHIFT                 12
#define MOVE_TYPE_MASK                  (0x7<<(MOVE_TYPE_SHIFT))

#define MOVE_CAPTURE_TYPE_SHIFT         15
#define MOVE_CAPTURE_TYPE_MASK          (0x7<<(MOVE_CAPTURE_TYPE_SHIFT))

#define MOVE_SPECIAL_FLAGS_SHIFT        18
#define MOVE_SPECIAL_FLAGS_MASK         (0xF<<(MOVE_SPECIAL_FLAGS_SHIFT))

#define MOVE_SCORE_SHIFT                22
#define MOVE_SCORE_MASK                 (0x3FF<<(MOVE_SCORE_SHIFT))

/* Move "special" flags */
#define MOVE_QUIET                      0x0
#define MOVE_DOUBLE_PAWN_PUSH           0x1
#define MOVE_KING_CASTLE                0x2
#define MOVE_QUEEN_CASTLE               0x3
#define MOVE_CAPTURE                    0x4
#define MOVE_EP_CAPTURE                 0x5
#define MOVE_KNIGHT_PROMOTION           0x8
#define MOVE_BISHOP_PROMOTION           0x9
#define MOVE_ROOK_PROMOTION             0xA
#define MOVE_QUEEN_PROMOTION            0xB
#define MOVE_KNIGHT_PROMOTION_CAPTURE   0xC
#define MOVE_BISHOP_PROMOTION_CAPTURE   0xD
#define MOVE_ROOK_PROMOTION_CAPTURE     0xE
#define MOVE_QUEEN_PROMOTION_CAPTURE    0xF

#define MOVE_GET_POS_FROM(move)             (((move) & MOVE_POS_FROM_MASK) >> MOVE_POS_FROM_SHIFT)
#define MOVE_GET_POS_TO(move)               (((move) & MOVE_POS_TO_MASK) >> MOVE_POS_TO_SHIFT)
#define MOVE_GET_TYPE(move)                 (((move) & MOVE_TYPE_MASK) >> MOVE_TYPE_SHIFT)
#define MOVE_GET_CAPTURE_TYPE(move)         (((move) & MOVE_CAPTURE_TYPE_MASK) >> MOVE_CAPTURE_TYPE_SHIFT)
#define MOVE_GET_SPECIAL_FLAGS(move)        (((move) & MOVE_SPECIAL_FLAGS_MASK) >> MOVE_SPECIAL_FLAGS_SHIFT)

#define MOVE_IS_CAPTURE(move)               ((move) & (MOVE_CAPTURE << MOVE_SPECIAL_FLAGS_SHIFT))
#define MOVE_IS_PROMOTION(move)             ((move) & (MOVE_KNIGHT_PROMOTION << MOVE_SPECIAL_FLAGS_SHIFT))
#define MOVE_IS_CAPTURE_OR_PROMOTION(move)  ((move) & ((MOVE_CAPTURE | MOVE_KNIGHT_PROMOTION) << MOVE_SPECIAL_FLAGS_SHIFT))
#define MOVE_PROMOTION_TYPE(move)           ((MOVE_GET_SPECIAL_FLAGS(move) & 0x8) ? ((MOVE_GET_SPECIAL_FLAGS(move) & 0xB)-7) : (0))


/* Type describing the state of the game */
typedef struct chess_state_t {
    bitboard_t    bitboard[NUM_COLORS*NUM_TYPES+1];
    bitboard_t    hash;
    move_t        last_move;
    char          castling[2];
    unsigned char ep_file;
    unsigned char player;
    char          halfmove_clock;
} chess_state_t;

#define WHITE_PIECES    0
#define BLACK_PIECES    (NUM_TYPES)
#define OCCUPIED        (NUM_COLORS*NUM_TYPES)

#define STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_SHIFT     0
#define STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK      (1<<(STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_SHIFT))

#define STATE_FLAGS_KING_CASTLE_POSSIBLE_SHIFT      1
#define STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK       (1<<(STATE_FLAGS_KING_CASTLE_POSSIBLE_SHIFT))

#define STATE_EN_PASSANT_NONE                       8


void STATE_reset(chess_state_t *s);
int  STATE_generate_moves(const chess_state_t *s, int num_checkers, bitboard_t block_check, bitboard_t pinners, bitboard_t pinned, move_t *moves);
int  STATE_generate_moves_quiescence(const chess_state_t *s, int num_checkers, bitboard_t block_check, bitboard_t pinners, bitboard_t pinned, move_t *moves);
int  STATE_generate_moves_simple(const chess_state_t *s, move_t *moves);
int  STATE_apply_move(chess_state_t *s, const move_t move);
int  STATE_checkers_and_pinners(const chess_state_t *s, bitboard_t *block_check, bitboard_t *pinners, bitboard_t *pinned);
void STATE_compute_hash(chess_state_t *s);
int  STATE_risk_zugzwang(const chess_state_t *s);
void STATE_move_print_debug(const move_t move);
void STATE_board_print_debug(const chess_state_t *s);

#endif
