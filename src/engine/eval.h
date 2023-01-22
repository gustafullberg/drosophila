#ifndef EVAL_H
#define EVAL_H

#include "state.h"

/* Material value */
#define PAWN_VALUE      20
#define KNIGHT_VALUE    65
#define BISHOP_VALUE    65
#define ROOK_VALUE     100
#define QUEEN_VALUE    195
#define BISHOP_PAIR     10

extern const short piecesquare[7][64];

typedef struct {
    struct {
        int pawn[64];
        int knight[64];
        int bishop[64];
        int rook[64];
        int queen[64];
        int king_midgame[64];
        int king_endgame[64];
    } psq;
    struct {
        int knight[9];
        int bishop[14];
        int rook_o[15];
        int rook_e[15];
        int queen_o[28];
        int queen_e[28];
    } mobility;
    struct {
        int pawn[5];
        int knight[5];
        int bishop[5];
        int rook[5];
    } threat;
    struct {
        int knight[8];
        int bishop[8];
        int rook[8];
        int queen[8];
        int scaling_midgame[5];
    } pressure;
    struct {
        int pawn_guards_minor;
        int pawn_guards_pawn;
        int pawn_shield_1;
        int pawn_shield_2;
        int pawn_passed_o;
        int pawn_passed_e;
        int pawn_passed_dist_kings_diff_e;
        int pawn_passed_dist_own_king_e;
        int pawn_passed_unblocked;
        int pawn_passed_unreachable_e;
        int pawn_isolated_o;
        int pawn_isolated_e;
        int knight_reduction;
        int rook_open_file_o;
        int rook_open_file_e;
        int rook_halfopen_file_o;
        int rook_halfopen_file_e;
        int rook_rearmost_pawn_o;
        int rook_rearmost_pawn_e;
        int tempo;
        int pawn_passed_scaling[8];
    } positional;
} eval_param_t;

void  EVAL_pawn_types(const chess_state_t *s, bitboard_t attack[NUM_COLORS], bitboard_t *passedPawns, bitboard_t *isolatedPawns);
short EVAL_evaluate_board(const chess_state_t *s);
int   EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos);
int   EVAL_draw(const chess_state_t *s);

#endif
