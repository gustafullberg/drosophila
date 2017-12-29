#include "eval.h"

/* Material */
#define PAWN_VALUE      20
#define KNIGHT_VALUE    65
#define BISHOP_VALUE    65
#define ROOK_VALUE     100
#define QUEEN_VALUE    195
#define BISHOP_PAIR     10

/* Positional */
#define PAWN_GUARDS_MINOR                3
#define PAWN_GUARDS_PAWN                 1
#define PAWN_SHIELD_1                    5
#define PAWN_SHIELD_2                    2
#define PAWN_PASSED_O                   10
#define PAWN_PASSED_E                   10
#define PAWN_PASSED_DIST_OPP_KING_E      5
#define PAWN_PASSED_DIST_OWN_KING_E     -5
#define PAWN_PASSED_UNBLOCKED_E         10
#define PAWN_PASSED_UNREACHABLE_E       25
#define PAWN_ISOLATED_O                 -3
#define PAWN_ISOLATED_E                 -2
#define ROOK_OPEN_FILE_O                 8
#define ROOK_OPEN_FILE_E                 4
#define ROOK_HALFOPEN_FILE_O             4
#define ROOK_HALFOPEN_FILE_E             2
#define TEMPO                            2

static const int passed_pawn_scaling[8] = { 0, 7, 28, 64, 114, 178, 256, 0}; // Q8
static const int king_queen_tropism[8]  = { 0,  7, 7, 5, 0, 0, 0, 0 };
static const int king_rook_tropism[8]   = { 0,  5, 5, 3, 0, 0, 0, 0 };
static const int king_bishop_tropism[8] = { 0,  3, 3, 2, 0, 0, 0, 0 };
static const int king_knight_tropism[8] = { 0,  0, 0, 2, 0, 0, 0, 0 };

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
    short score = 0;
    int   game_progress;
    int   color;
    int   pos;
    bitboard_t pieces;
    bitboard_t pos_bitboard;
    bitboard_t pawnAttacks[2], passedPawns, isolatedPawns;

    EVAL_pawn_types(s, pawnAttacks, &passedPawns, &isolatedPawns);
    
    /* Kings */
    king_pos[WHITE] = BITBOARD_find_bit(s->bitboard[WHITE_PIECES + KING]);
    king_pos[BLACK] = BITBOARD_find_bit(s->bitboard[BLACK_PIECES + KING]);
    
    for(color = WHITE; color <= BLACK; color++) {
        int pos_mask = color * 0x38;
        
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
                bonus_e += distance[king_pos[color^1]][pos] * PAWN_PASSED_DIST_OPP_KING_E;
                bonus_e += distance[king_pos[color]][pos] * PAWN_PASSED_DIST_OWN_KING_E;

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
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            pos_bitboard = BITBOARD_POSITION(pos);
            material_score[color] += KNIGHT_VALUE;
            positional_score[color] += piecesquare[KNIGHT][pos^pos_mask];
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
            
            pieces ^= BITBOARD_POSITION(pos);
        }
        
        /* Queens */
        pieces = s->bitboard[NUM_TYPES*color + QUEEN];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += QUEEN_VALUE;
            positional_score[color] += piecesquare[QUEEN][pos^pos_mask];
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
        /* KNN-K */
        no_kings = s->bitboard[OCCUPIED] ^ (s->bitboard[WHITE_PIECES+KING] | s->bitboard[BLACK_PIECES+KING]);
        if(s->bitboard[WHITE_PIECES+KNIGHT] == no_kings || s->bitboard[BLACK_PIECES+KNIGHT] == no_kings) {
            return 1;
        }

        /* KB-KB (same color) */
        if(s->bitboard[WHITE_PIECES+BISHOP] && s->bitboard[BLACK_PIECES+BISHOP]) {
            bitboard_t bishops = s->bitboard[WHITE_PIECES+BISHOP] & s->bitboard[BLACK_PIECES+BISHOP];
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
