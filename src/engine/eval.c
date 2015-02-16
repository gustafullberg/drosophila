#include "eval.h"

/* Material */
#define PAWN_VALUE      20
#define KNIGHT_VALUE    65
#define BISHOP_VALUE    65
#define ROOK_VALUE     100
#define QUEEN_VALUE    195
#define BISHOP_PAIR     10

/* Positional */
#define PAWN_GUARDS_MINOR   3
#define PAWN_GUARDS_PAWN    1
#define PAWN_SHIELD_1       5
#define PAWN_SHIELD_2       2
#define PAWN_BACKWARD       0
#define PAWN_PASSED         0
#define PAWN_DOUBLED        0
#define PAWN_ISOLATED       0
#define TEMPO               2

static const int king_queen_tropism[8]  = { 0,  7, 7, 5, 0, 0, 0, 0 };
static const int king_rook_tropism[8]   = { 0,  5, 5, 3, 0, 0, 0, 0 };
static const int king_bishop_tropism[8] = { 0,  3, 3, 2, 0, 0, 0, 0 };
static const int king_knight_tropism[8] = { 0,  0, 0, 2, 0, 0, 0, 0 };

static const short sign[2] = { 1, -1 };

/* Game progress: 256 = opening, 0 = endgame */
int EVAL_game_progress(short material[2])
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

void EVAL_pawn_types(const chess_state_t *s, bitboard_t attack[2], bitboard_t *backwardPawns, bitboard_t *passedPawns, bitboard_t *doubledPawns, bitboard_t *isolatedPawns)
{
    bitboard_t pawns[2];
    bitboard_t stop[2];
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

    /* Stop squares */
    stop[WHITE] = pawns[WHITE] << 8;
    stop[BLACK] = pawns[BLACK] >> 8;

    /* Front-fill */
    frontFill[WHITE] = BITBOARD_fillNorth(pawns[WHITE]);
    frontFill[BLACK] = BITBOARD_fillSouth(pawns[BLACK]);

    /* Attack-span */
    attackSpan[WHITE] = BITBOARD_fillNorth(attack[WHITE]);
    attackSpan[BLACK] = BITBOARD_fillSouth(attack[BLACK]);

    /* Combined front-fill and attack-span */
    attackFrontFill[WHITE] = frontFill[WHITE] | attackSpan[WHITE];
    attackFrontFill[BLACK] = frontFill[BLACK] | attackSpan[BLACK];

    /* Fill in both directions */
    attackFileFill[WHITE] = BITBOARD_fillSouth(attackSpan[WHITE]);
    attackFileFill[BLACK] = BITBOARD_fillNorth(attackSpan[BLACK]);

    /* Backward pawns */
    *backwardPawns = ((stop[WHITE] & attack[BLACK] & ~attackSpan[WHITE]) >> 8) |
                     ((stop[BLACK] & attack[WHITE] & ~attackSpan[BLACK]) << 8);

    /* Passed pawns */
    *passedPawns = (pawns[WHITE] & ~attackFrontFill[BLACK]) |
                   (pawns[BLACK] & ~attackFrontFill[WHITE]);

    /* Doubled pawns */
    *doubledPawns = ((frontFill[WHITE] << 8) & pawns[WHITE]) |
                    ((frontFill[BLACK] >> 8) & pawns[BLACK]);

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

static inline int EVAL_pawn_promotion_defended_by_king(const int color, const bitboard_t pos_pawn_bitboard, const int pos_king)
{
    bitboard_t frontFill;
    if(color == WHITE) frontFill = BITBOARD_fillNorth(pos_pawn_bitboard);
    else               frontFill = BITBOARD_fillNorth(pos_pawn_bitboard);

    if(((bitboard_king[pos_king] & frontFill) == frontFill) && ((BITBOARD_POSITION(pos_king) & frontFill) == 0)) {
        /* Own king defends full path to promotion */
        return 1;
    }

    return 0;
}

static inline int EVAL_pawn_promotion_unstoppable_by_king(const int color, const int pos_pawn, const int rank, const int pos_opponent_king)
{
    int promotion_sq;
    int dist_opponent_king_to_promotion_sq;
    int dist_to_promotion_sq = 7 - rank;

    if(color == WHITE) promotion_sq = pos_pawn + 8 * dist_to_promotion_sq;
    else promotion_sq = pos_pawn - 8 * dist_to_promotion_sq;

    dist_opponent_king_to_promotion_sq = distance[promotion_sq][pos_opponent_king];

    if(dist_to_promotion_sq < dist_opponent_king_to_promotion_sq) {
        return 1;
    }

    return 0;
}

short EVAL_evaluate_board(const chess_state_t *s)
{
    short pawn_material_score[NUM_COLORS] = { 0, 0 };
    short material_score[NUM_COLORS] = { 0, 0 };
    short positional_score[NUM_COLORS] = { 0, 0 };
    short positional_score_o[NUM_COLORS] = { 0, 0 };
    short positional_score_e[NUM_COLORS] = { 0, 0 };
    int king_pos[NUM_COLORS];
    short score = 0;
    int game_progress;
    int color;
    bitboard_t pieces;
    int pos;
    bitboard_t pos_bitboard;
    bitboard_t pawnAttacks[2], backwardPawns, passedPawns, doubledPawns, isolatedPawns;

    EVAL_pawn_types(s, pawnAttacks, &backwardPawns, &passedPawns, &doubledPawns, &isolatedPawns);
    
    /* Kings */
    king_pos[WHITE] = BITBOARD_find_bit(s->bitboard[WHITE_PIECES + KING]);
    king_pos[BLACK] = BITBOARD_find_bit(s->bitboard[BLACK_PIECES + KING]);
    positional_score_o[WHITE] += piecesquare[KING][king_pos[WHITE]];
    positional_score_o[BLACK] += piecesquare[KING][king_pos[BLACK]^0x38];
    positional_score_e[WHITE] += piecesquare[KING+1][king_pos[WHITE]];
    positional_score_e[BLACK] += piecesquare[KING+1][king_pos[BLACK]^0x38];
    
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
                positional_score_o[color] += 2;
                positional_score_e[color] += 4;

                /* Defended by own king */
                if(bitboard_king[pos] & BITBOARD_POSITION(king_pos[color])) {
                    positional_score_e[color] += 10;
                    if(EVAL_pawn_promotion_defended_by_king(color, pos_bitboard, king_pos[color])) {
                        positional_score_e[color] += 10;
                    }
                }

                /* Threatened by opponent king */
                if(bitboard_king[pos] & BITBOARD_POSITION(king_pos[color^1])) {
                    positional_score_e[color] -= 10;
                } else {
                    if(EVAL_pawn_promotion_unstoppable_by_king(color, pos, rank, king_pos[color^1])) {
                        positional_score_e[color] += 20;
                    }
                }
            }

            /* Isolated pawn */
            if(pos_bitboard & isolatedPawns) {
                positional_score_o[color] += -1;
                positional_score_e[color] += -2;
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
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += ROOK_VALUE;
            positional_score[color] += piecesquare[ROOK][pos^pos_mask];
            positional_score[color] += king_rook_tropism[(int)distance[king_pos[color^1]][pos]];
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
    }
    
    score += pawn_material_score[WHITE] - pawn_material_score[BLACK];
    score += material_score[WHITE] - material_score[BLACK];
    score += positional_score[WHITE] - positional_score[BLACK];


    /* Pawn shield (TODO: Refactor) */
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
    /* Only kings left */
    if(s->bitboard[OCCUPIED] == (s->bitboard[WHITE_PIECES+KING] | s->bitboard[BLACK_PIECES+KING])) {
        return 1;
    }
    
    return 0;
}

/* Returns non-zero if draw may be claimed due to the fifty move rule */
int EVAL_fifty_move_rule(const chess_state_t *s)
{
    return (s->halfmove_clock >= 100);
}
