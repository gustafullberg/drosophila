#include "eval.h"
#include "movegen.h"

#define PAWN_GUARDS_MINOR 3
#define PAWN_GUARDS_PAWN 4

#define PAWN_DOUBLE_PAWN -10
#define PAWN_TRIPLE_PAWN -20

#define PAWN_SHIELD_1 5
#define PAWN_SHIELD_2 2

const short piece_value[NUM_TYPES] = { 20, 60, 64, 100, 180, 0 };
static const short sign[2] = { 1, -1 };

static const short pawn_double_pawn_penalty[8] = {
    0, 0, PAWN_DOUBLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN
};

short EVAL_material_midgame(const chess_state_t *s)
{
    bitboard_t pieces;
    int type;
    short result = 0;
    
    for(type = PAWN; type <= KING; type++) {
        const short *psq = piecesquare[type];
        pieces = s->bitboard[WHITE_PIECES+type];
        while(pieces) {
            int pos = BITBOARD_find_bit(pieces);
            result += piece_value[type] + psq[pos];
            pieces ^= BITBOARD_POSITION(pos);
        }
        pieces = s->bitboard[BLACK_PIECES+type];
        while(pieces) {
            int pos = BITBOARD_find_bit(pieces);
            result -= piece_value[type] + psq[pos^0x38];
            pieces ^= BITBOARD_POSITION(pos);
        }
    }
    
    return result;
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

static inline short EVAL_pawn_guards_minor_piece(const chess_state_t *s)
{
    /* Bitboards with knights and bishops */
    bitboard_t white_minor = s->bitboard[WHITE_PIECES + KNIGHT] | s->bitboard[WHITE_PIECES + BISHOP];
    bitboard_t black_minor = s->bitboard[BLACK_PIECES + KNIGHT] | s->bitboard[BLACK_PIECES + BISHOP];
    
    /* Bitboards with pawns */
    bitboard_t white_pawn = s->bitboard[WHITE_PIECES + PAWN];
    bitboard_t black_pawn = s->bitboard[BLACK_PIECES + PAWN];
    
    /* Pawn guard squares */
    bitboard_t white_guard = (white_pawn & ~(BITBOARD_FILE << 0)) << 7 |
                             (white_pawn & ~(BITBOARD_FILE << 7)) << 9;
    bitboard_t black_guard = (black_pawn & ~(BITBOARD_FILE << 0)) >> 9 |
                             (black_pawn & ~(BITBOARD_FILE << 7)) >> 7;
    
    /* Number of minor pieces guarded by pawns */
    return PAWN_GUARDS_MINOR * (BITBOARD_count_bits(white_guard & white_minor) - BITBOARD_count_bits(black_guard & black_minor));
}

short EVAL_evaluate_board(const chess_state_t *s)
{
    short material_score[NUM_COLORS] = { 0, 0 };
    short positional_score[NUM_COLORS] = { 0, 0 };
    int king_pos[NUM_COLORS];
    short score = 0;
    int is_endgame = STATE_is_endgame(s);
    int color;
    bitboard_t pieces;
    int pos;
    
    /* Kings */
    king_pos[WHITE] = BITBOARD_find_bit(s->bitboard[WHITE_PIECES + KING]);
    king_pos[BLACK] = BITBOARD_find_bit(s->bitboard[BLACK_PIECES + KING]);
    if(is_endgame) {
        positional_score[WHITE] += piecesquare[KING+1][king_pos[WHITE]];
        positional_score[BLACK] += piecesquare[KING+1][king_pos[BLACK]^0x38];
    } else {
        positional_score[WHITE] += piecesquare[KING][king_pos[WHITE]];
        positional_score[BLACK] += piecesquare[KING][king_pos[BLACK]^0x38];
    }
    
    for(color = WHITE; color <= BLACK; color++) {
        int pos_mask = color * 0x38;
        
        /* Pawns */
        pieces = s->bitboard[NUM_TYPES*color + PAWN];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += 20;
            positional_score[color] += piecesquare[PAWN][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }
        
        /* Knights */
        pieces = s->bitboard[NUM_TYPES*color + KNIGHT];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += 60;
            positional_score[color] += piecesquare[KNIGHT][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }
        
        /* Bishops */
        pieces = s->bitboard[NUM_TYPES*color + BISHOP];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += 64;
            positional_score[color] += piecesquare[BISHOP][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }

        /* Rooks */
        pieces = s->bitboard[NUM_TYPES*color + ROOK];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += 100;
            positional_score[color] += piecesquare[ROOK][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }
        
        /* Queens */
        pieces = s->bitboard[NUM_TYPES*color + QUEEN];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += 180;
            positional_score[color] += piecesquare[QUEEN][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }
    }
    
    score += material_score[WHITE] - material_score[BLACK];
    score += positional_score[WHITE] - positional_score[BLACK];
    
    if(!is_endgame) {
        /* Pawn shield */
        score += EVAL_pawn_shield(s);

        /* Pawns guarding minor pieces */
        score += EVAL_pawn_guards_minor_piece(s);
    }

    /* Invert score for black player */
    score *= sign[(int)(s->player)];
    
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
