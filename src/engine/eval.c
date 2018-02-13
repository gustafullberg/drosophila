#include "eval.h"
#include "movegen.h"

//int tune[20] = { 3, 1, 5, 2, 10, 10, 5, -5, 10, 25, -3, -2, 2, 8, 4, 4, 2, 0, 0, 2 };
//int tune[20] = { 2, 1, 5, 2, 11, 10, 6, -5,  6, 22, -3, -2, 2, 8, 2, 5, 4, 0, 3, 1 };
//int tune[20] = { 2, 1, 5, 2, 11, 10, 5, -5,  6, 22, -3, -2, 2, 8, 2, 5, 4, 0, 3, 1 };
//int tune[20] = { 2, 1, 5, 2, 11, 10, 5, 1,  6, 22, -3, -2, 2, 8, 2, 5, 4, 0, 3, 1 };
int tune[20] = { 2, 1, 5, 2, 11, 10, 6, 1,  6, 22, -3, -2, 2, 8, 2, 5, 4, 0, 3, 1 };

/* Positional */
#define PAWN_GUARDS_MINOR                tune[0]
#define PAWN_GUARDS_PAWN                 tune[1]
#define PAWN_SHIELD_1                    tune[2]
#define PAWN_SHIELD_2                    tune[3]
#define PAWN_PASSED_O                    tune[4]
#define PAWN_PASSED_E                   tune[5]
#define PAWN_PASSED_DIST_KINGS_DIFF_E   tune[6]
#define PAWN_PASSED_DIST_OWN_KING_E     tune[7]
#define PAWN_PASSED_UNBLOCKED_E         tune[8]
#define PAWN_PASSED_UNREACHABLE_E       tune[9]
#define PAWN_ISOLATED_O                 tune[10]
#define PAWN_ISOLATED_E                 tune[11]
#define KNIGHT_REDUCTION                 tune[12]
#define ROOK_OPEN_FILE_O                 tune[13]
#define ROOK_OPEN_FILE_E                 tune[14]
#define ROOK_HALFOPEN_FILE_O             tune[15]
#define ROOK_HALFOPEN_FILE_E             tune[16]
#define ROOK_REARMOST_PAWN_O             tune[17]
#define ROOK_REARMOST_PAWN_E             tune[18]
#define TEMPO                            tune[19]

static const int passed_pawn_scaling[8] = { 0, 7, 28, 64, 114, 178, 256, 0}; // Q8
static const int king_queen_tropism[8]  = { 0,  7, 7, 5, 0, 0, 0, 0 };
static const int king_rook_tropism[8]   = { 0,  5, 5, 3, 0, 0, 0, 0 };
static const int king_bishop_tropism[8] = { 0,  3, 3, 2, 0, 0, 0, 0 };
static const int king_knight_tropism[8] = { 0,  0, 0, 2, 0, 0, 0, 0 };
short mobility_knight[9] = { -6, -4, -2, -1, 0, 1, 2, 3, 4 };
short mobility_bishop[14] = { -6, -4, -3, -2, -2, -1, 0, 0, 1, 1, 2, 2, 3, 3 };
short mobility_rook_o[15] = { -3, -2, -2, -1, -1, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 };
short mobility_rook_e[15] = { -8, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
short mobility_queen_o[28] = { -3, -2, -2, -2, -2, -2, -1, -1, -1, -1, -1, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3 };
short mobility_queen_e[28] = { -10, -9, -7, -6, -5, -5, -4, -3, -2, -1, -1, 0, 0, 1, 1, 2, 3, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10, 10 };

static const short sign[2] = { 1, -1 };

/* Game progress: 256 = opening, 0 = endgame */
static int EVAL_game_progress(short material[2])
{
    const int m_hi = 2 * (QUEEN_VALUE + 2 * ROOK_VALUE + BISHOP_VALUE);
    const int m_lo = 2 * (2 * KNIGHT_VALUE);
    const int sum_material = material[WHITE] + material[BLACK];
    if(sum_material > m_hi) {
        return 256;
    } else if(sum_material < m_lo) {
        return 0;
    } else {
        return 256 * (sum_material - m_lo) / (m_hi - m_lo);
    }
}

void EVAL_pawn_types(const chess_state_t *s, bitboard_t attack[NUM_COLORS], bitboard_t *passedPawns, bitboard_t *isolatedPawns)
{
    bitboard_t pawns[2];
    bitboard_t frontFill[2];
    bitboard_t attackSpan[2];
    bitboard_t attackFrontFill[2];
    bitboard_t attackFileFill[2];

    /* Location of pawns */
    pawns[WHITE] = s->bitboard[WHITE_PIECES+PAWN];
    pawns[BLACK] = s->bitboard[BLACK_PIECES+PAWN];

    /* Squares attacked by pawns */
    attack[WHITE] = ((pawns[WHITE] & ~(BITBOARD_FILE<<0)) << 7) | ((pawns[WHITE] & ~(BITBOARD_FILE<<7)) << 9);
    attack[BLACK] = ((pawns[BLACK] & ~(BITBOARD_FILE<<0)) >> 9) | ((pawns[BLACK] & ~(BITBOARD_FILE<<7)) >> 7);

    /* Front-fill */
    frontFill[WHITE] = BITBOARD_fill_north(pawns[WHITE]);
    frontFill[BLACK] = BITBOARD_fill_south(pawns[BLACK]);

    /* Attack-span */
    attackSpan[WHITE] = BITBOARD_fill_north(attack[WHITE]);
    attackSpan[BLACK] = BITBOARD_fill_south(attack[BLACK]);

    /* Combined front-fill and attack-span */
    attackFrontFill[WHITE] = frontFill[WHITE] | attackSpan[WHITE];
    attackFrontFill[BLACK] = frontFill[BLACK] | attackSpan[BLACK];

    /* Fill in both directions */
    attackFileFill[WHITE] = BITBOARD_fill_south(attackSpan[WHITE]);
    attackFileFill[BLACK] = BITBOARD_fill_north(attackSpan[BLACK]);

    /* Passed pawns */
    *passedPawns = (pawns[WHITE] & ~attackFrontFill[BLACK]) |
                   (pawns[BLACK] & ~attackFrontFill[WHITE]);

    /* Isolated pawns */
    *isolatedPawns = (pawns[WHITE] & ~attackFileFill[WHITE]) |
                     (pawns[BLACK] & ~attackFileFill[BLACK]);
}

static short EVAL_pawn_shield(const chess_state_t *s)
{
    const bitboard_t white_queenside = 0x0000000000000006;
    const bitboard_t white_kingside  = 0x00000000000000E0;
    const bitboard_t black_queenside = 0x0600000000000000;
    const bitboard_t black_kingside  = 0xE000000000000000;
    short score = 0;

    if(s->bitboard[WHITE_PIECES+KING] & white_queenside) {
        score += BITBOARD_count_bits((white_queenside <<  8) & s->bitboard[WHITE_PIECES+PAWN]) * PAWN_SHIELD_1;
        score += BITBOARD_count_bits((white_queenside << 16) & s->bitboard[WHITE_PIECES+PAWN]) * PAWN_SHIELD_2;
    } else if(s->bitboard[WHITE_PIECES+KING] & white_kingside) {
        score += BITBOARD_count_bits((white_kingside <<  8) & s->bitboard[WHITE_PIECES+PAWN]) * PAWN_SHIELD_1;
        score += BITBOARD_count_bits((white_kingside << 16) & s->bitboard[WHITE_PIECES+PAWN]) * PAWN_SHIELD_2;
    }

    if(s->bitboard[BLACK_PIECES+KING] & black_queenside) {
        score -= BITBOARD_count_bits((black_queenside >>  8) & s->bitboard[BLACK_PIECES+PAWN]) * PAWN_SHIELD_1;
        score -= BITBOARD_count_bits((black_queenside >> 16) & s->bitboard[BLACK_PIECES+PAWN]) * PAWN_SHIELD_2;
    } else if(s->bitboard[BLACK_PIECES+KING] & black_kingside) {
        score -= BITBOARD_count_bits((black_kingside >>  8) & s->bitboard[BLACK_PIECES+PAWN]) * PAWN_SHIELD_1;
        score -= BITBOARD_count_bits((black_kingside >> 16) & s->bitboard[BLACK_PIECES+PAWN]) * PAWN_SHIELD_2;
    }

    return score;
}

short EVAL_evaluate_board(const chess_state_t *s)
{
    short pawn_material_score[NUM_COLORS] = { 0, 0 };
    short material_score[NUM_COLORS]      = { 0, 0 };
    short positional_score[NUM_COLORS]    = { 0, 0 };
    short positional_score_o[NUM_COLORS]  = { 0, 0 };
    short positional_score_e[NUM_COLORS]  = { 0, 0 };
    int   king_pos[NUM_COLORS];
    int   rearmost_pawn[NUM_COLORS];
    short score = 0;
    int   game_progress;
    int   color;
    int   pos;
    int   piece_mobility;
    bitboard_t pieces;
    bitboard_t pos_bitboard;
    bitboard_t pawnAttacks[2], passedPawns, isolatedPawns;
    bitboard_t moves, captures;

    rearmost_pawn[WHITE] = s->bitboard[WHITE_PIECES+PAWN] ? BITBOARD_GET_RANK(BITBOARD_find_bit(s->bitboard[WHITE_PIECES+PAWN])) : -1;
    rearmost_pawn[BLACK] = s->bitboard[BLACK_PIECES+PAWN] ? BITBOARD_GET_RANK(BITBOARD_find_bit_reversed(s->bitboard[BLACK_PIECES+PAWN])) : -1;

    EVAL_pawn_types(s, pawnAttacks, &passedPawns, &isolatedPawns);

    /* Kings */
    king_pos[WHITE] = BITBOARD_find_bit(s->bitboard[WHITE_PIECES + KING]);
    king_pos[BLACK] = BITBOARD_find_bit(s->bitboard[BLACK_PIECES + KING]);

    for(color = WHITE; color <= BLACK; color++) {
        int pos_mask = color * 0x38;
        bitboard_t own_pieces = s->bitboard[NUM_TYPES*color + ALL];
        bitboard_t opp_pieces = s->bitboard[NUM_TYPES*(color^1) + ALL];
        bitboard_t diagonal_sliders = s->bitboard[NUM_TYPES*color + BISHOP] | s->bitboard[NUM_TYPES*color + QUEEN];
        bitboard_t straight_sliders = s->bitboard[NUM_TYPES*color + ROOK] | s->bitboard[NUM_TYPES*color + QUEEN];

        /* Pawns */
        pieces = s->bitboard[NUM_TYPES*color + PAWN];
        while(pieces) {
            int rank;
            pos = BITBOARD_find_bit(pieces);
            pos_bitboard = BITBOARD_POSITION(pos);
            rank = BITBOARD_GET_RANK(pos^pos_mask);
            pawn_material_score[color] += PAWN_VALUE;
            positional_score[color] += piecesquare[PAWN][pos^pos_mask];
            positional_score_o[color] += (pos_bitboard & pawnAttacks[color]) ? PAWN_GUARDS_PAWN : 0; /* Guarded by other pawn */

            /* Passed pawn */
            if(pos_bitboard & passedPawns) {
                /* Initial bonus for passed pawn */
                short bonus_o = PAWN_PASSED_O;
                short bonus_e = PAWN_PASSED_E;

                /* Distance to kings */
                int dist_own_king = distance[king_pos[color]][pos];
                int dist_opp_king = distance[king_pos[color^1]][pos];
                bonus_e += (dist_opp_king - dist_own_king) * PAWN_PASSED_DIST_KINGS_DIFF_E;
                bonus_e += dist_own_king * PAWN_PASSED_DIST_OWN_KING_E;

                /* Unblocked? */
                if((bitboard_pawn_move[color][pos] & s->bitboard[OCCUPIED]) == 0) {
                    bonus_e += PAWN_PASSED_UNBLOCKED_E;

                    /* Unreachable by opponent king? */
                    int dist_prom = 7 - rank;
                    int prom_pos = (pos^pos_mask) + dist_prom * 8;
                    int dist_prom_opp_king = distance[king_pos[color^1]^pos_mask][prom_pos] - (color != s->player);
                    bonus_e += PAWN_PASSED_UNREACHABLE_E * (dist_prom < dist_prom_opp_king);
                }

                /* Scale bonus with rank */
                int scale_factor = passed_pawn_scaling[rank];
                positional_score_o[color] += (short)((int)bonus_o * scale_factor >> 8);
                positional_score_e[color] += (short)((int)bonus_e * scale_factor >> 8);
            }

            /* Isolated pawn */
            if(pos_bitboard & isolatedPawns) {
                positional_score_o[color] += PAWN_ISOLATED_O;
                positional_score_e[color] += PAWN_ISOLATED_E;
            }

            pieces ^= pos_bitboard;
        }

        /* Knights */
        pieces = s->bitboard[NUM_TYPES*color + KNIGHT];
        int num_opp_pawns = BITBOARD_count_bits(s->bitboard[NUM_TYPES*(color^1) + PAWN]);
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            pos_bitboard = BITBOARD_POSITION(pos);
            material_score[color] += KNIGHT_VALUE - (8 - num_opp_pawns) * KNIGHT_REDUCTION;
            positional_score[color] += piecesquare[KNIGHT][pos^pos_mask];
            piece_mobility = BITBOARD_count_bits(bitboard_knight[pos] & ~(own_pieces | pawnAttacks[color^1]));
            positional_score[color] += mobility_knight[piece_mobility];
            positional_score_o[color] += (pos_bitboard & pawnAttacks[color]) ? PAWN_GUARDS_MINOR : 0; /* Guarded by pawn */
            positional_score[color] += king_knight_tropism[(int)distance[king_pos[color^1]][pos]];
            pieces ^= pos_bitboard;
        }

        /* Bishops */
        pieces = s->bitboard[NUM_TYPES*color + BISHOP];
        if((pieces & BITBOARD_BLACK_SQ) && (pieces & BITBOARD_WHITE_SQ)) {
            /* Bishops on white/black squares => pair bonus */
            material_score[color] += BISHOP_PAIR;
        }
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            pos_bitboard = BITBOARD_POSITION(pos);
            material_score[color] += BISHOP_VALUE;
            positional_score[color] += piecesquare[BISHOP][pos^pos_mask];
            MOVEGEN_bishop(pos, own_pieces & ~diagonal_sliders, opp_pieces, &moves, &captures);
            piece_mobility = BITBOARD_count_bits((moves | captures) & ~pawnAttacks[color^1]);
            positional_score[color] += mobility_bishop[piece_mobility];
            positional_score_o[color] += (pos_bitboard & pawnAttacks[color]) ? PAWN_GUARDS_MINOR : 0; /* Guarded by pawn */
            positional_score[color] += king_bishop_tropism[(int)distance[king_pos[color^1]][pos]];
            pieces ^= pos_bitboard;
        }

        /* Rooks */
        pieces = s->bitboard[NUM_TYPES*color + ROOK];
        while(pieces) {
            bitboard_t file;
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += ROOK_VALUE;
            positional_score[color] += piecesquare[ROOK][pos^pos_mask];
            MOVEGEN_rook(pos, own_pieces & ~straight_sliders, opp_pieces, &moves, &captures);
            piece_mobility = BITBOARD_count_bits((moves | captures) & ~pawnAttacks[color^1]);
            positional_score_o[color] += mobility_rook_o[piece_mobility];
            positional_score_e[color] += mobility_rook_e[piece_mobility];
            positional_score[color] += king_rook_tropism[(int)distance[king_pos[color^1]][pos]];

            /* Check for (half-)open files */
            file = (BITBOARD_FILE << BITBOARD_GET_FILE(pos));
            if((file & s->bitboard[NUM_TYPES*color + PAWN]) == 0) {
                if((file & s->bitboard[NUM_TYPES*(color^1) + PAWN]) == 0) {
                    /* Open file */
                    positional_score_o[color] += ROOK_OPEN_FILE_O;
                    positional_score_e[color] += ROOK_OPEN_FILE_E;
                } else {
                    /* Half-open file */
                    positional_score_o[color] += ROOK_HALFOPEN_FILE_O;
                    positional_score_e[color] += ROOK_HALFOPEN_FILE_E;
                }
            }

            /* Rooks on the rank of the opponents rearmost pawns */
            if(BITBOARD_GET_RANK(pos) == rearmost_pawn[color^1]) {
                positional_score_o[color] += ROOK_REARMOST_PAWN_O;
                positional_score_e[color] += ROOK_REARMOST_PAWN_E;
            }

            pieces ^= BITBOARD_POSITION(pos);
        }

        /* Queens */
        pieces = s->bitboard[NUM_TYPES*color + QUEEN];
        while(pieces) {
            bitboard_t moves_b, captures_b, moves_r, captures_r;
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += QUEEN_VALUE;
            positional_score[color] += piecesquare[QUEEN][pos^pos_mask];
            MOVEGEN_bishop(pos, own_pieces & ~diagonal_sliders, opp_pieces, &moves_b, &captures_b);
            MOVEGEN_rook(pos, own_pieces & ~straight_sliders, opp_pieces, &moves_r, &captures_r);
            piece_mobility = BITBOARD_count_bits((moves_b | captures_b | moves_r | captures_r) & ~pawnAttacks[color^1]);
            positional_score_o[color] += mobility_queen_o[piece_mobility];
            positional_score_e[color] += mobility_queen_e[piece_mobility];
            positional_score[color] += king_queen_tropism[(int)distance[king_pos[color^1]][pos]];
            pieces ^= BITBOARD_POSITION(pos);
        }

        /* King */
        positional_score_o[color] += piecesquare[KING][king_pos[color]^pos_mask];
        positional_score_e[color] += piecesquare[KING+1][king_pos[color]^pos_mask];
    }

    score += pawn_material_score[WHITE] - pawn_material_score[BLACK];
    score += material_score[WHITE] - material_score[BLACK];
    score += positional_score[WHITE] - positional_score[BLACK];

    /* Pawn shield */
    positional_score_o[WHITE] += EVAL_pawn_shield(s);

    /* Add positional scores weighted by the progress of the game */
    game_progress = EVAL_game_progress(material_score);
    score += (game_progress * (positional_score_o[WHITE] - positional_score_o[BLACK]) +
             (256 - game_progress) * (positional_score_e[WHITE] - positional_score_e[BLACK])) / 256;

    /* Invert score for black player */
    score *= sign[(int)(s->player)];

    /* Add a bonus for the side with the right to move next */
    score += TEMPO;

    return score;
}

int EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos)
{
    const int player = color;
    const int own_index = player * NUM_TYPES;
    const int opponent_index = NUM_TYPES - own_index;

    bitboard_t attackers;

    /* Is attacked by pawns */
    attackers = bitboard_pawn_capture[player][pos] & s->bitboard[opponent_index + PAWN];
    if(attackers) {
        return 1;
    }

    /* Is attacked by knights */
    attackers = bitboard_knight[pos] & s->bitboard[opponent_index + KNIGHT];
    if(attackers) {
        return 1;
    }

    /* Is attacked by sliders (bishop, rook, queen)? */
    attackers = (bitboard_bishop[pos] & (s->bitboard[opponent_index + BISHOP] | s->bitboard[opponent_index + QUEEN])) |
                (bitboard_rook[pos]   & (s->bitboard[opponent_index + ROOK]   | s->bitboard[opponent_index + QUEEN]));
    while(attackers) {
        int attack_pos = BITBOARD_find_bit(attackers);
        if((bitboard_between[attack_pos][pos] & s->bitboard[OCCUPIED]) == 0) {
            return 1;
        }
        attackers ^= BITBOARD_POSITION(attack_pos);
    }

    /* Is attacked by king */
    attackers = bitboard_king[pos] & s->bitboard[opponent_index + KING];
    if(attackers) {
        return 1;
    }

    return 0;
}

/* Returns non-zero if draw may be claimed due to insufficient material */
int EVAL_insufficient_material(const chess_state_t *s)
{
    int num_pieces = BITBOARD_count_bits(s->bitboard[OCCUPIED]);
    bitboard_t no_kings;

    switch(num_pieces){
    case 2:
        /* Only kings left */
        return 1;

    case 3:
        /* KN-K or KB-K */
        if(s->bitboard[WHITE_PIECES+KNIGHT] | s->bitboard[WHITE_PIECES+BISHOP] | s->bitboard[BLACK_PIECES+KNIGHT] | s->bitboard[BLACK_PIECES+BISHOP]) {
            return 1;
        }
        break;

    case 4:
        /* KB-KB (same color) */
        if(s->bitboard[WHITE_PIECES+BISHOP] && s->bitboard[BLACK_PIECES+BISHOP]) {
            bitboard_t bishops = s->bitboard[WHITE_PIECES+BISHOP] | s->bitboard[BLACK_PIECES+BISHOP];
            if(((bishops & BITBOARD_WHITE_SQ) == 0) || ((bishops & BITBOARD_BLACK_SQ) == 0)) {
                return 1;
            }
        }
        break;
    }

    return 0;
}

/* Returns non-zero if draw may be claimed due to the fifty move rule */
int EVAL_fifty_move_rule(const chess_state_t *s)
{
    return (s->halfmove_clock >= 100);
}
