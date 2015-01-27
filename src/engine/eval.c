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
#define TEMPO               2

static const short sign[2] = { 1, -1 };

static int EVAL_is_endgame(const chess_state_t *s)
{
    /* Endgame defined as kings, pawns and up to 4 other pieces on the board */
    bitboard_t pieces = s->bitboard[OCCUPIED];
    pieces ^= s->bitboard[WHITE_PIECES+PAWN];
    pieces ^= s->bitboard[BLACK_PIECES+PAWN];
    return BITBOARD_count_bits(pieces) <= 6;
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
    short score;

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
    score = PAWN_GUARDS_MINOR * (BITBOARD_count_bits(white_guard & white_minor) - BITBOARD_count_bits(black_guard & black_minor));

    /* Number of pawns guarded by pawns */
    score += PAWN_GUARDS_PAWN  * (BITBOARD_count_bits(white_guard & white_pawn) - BITBOARD_count_bits(black_guard & black_pawn));

    return score;
}

short EVAL_evaluate_board(const chess_state_t *s)
{
    short material_score[NUM_COLORS] = { 0, 0 };
    short positional_score[NUM_COLORS] = { 0, 0 };
    int king_pos[NUM_COLORS];
    short score = 0;
    int is_endgame = EVAL_is_endgame(s);
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
            material_score[color] += PAWN_VALUE;
            positional_score[color] += piecesquare[PAWN][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }
        
        /* Knights */
        pieces = s->bitboard[NUM_TYPES*color + KNIGHT];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += KNIGHT_VALUE;
            positional_score[color] += piecesquare[KNIGHT][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }
        
        /* Bishops */
        pieces = s->bitboard[NUM_TYPES*color + BISHOP];
        if((pieces & BITBOARD_BLACK_SQ) && (pieces & BITBOARD_WHITE_SQ)) {
            /* Bishops on white/black squares => pair bonus */
            material_score[color] += BISHOP_PAIR;
        }
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += BISHOP_VALUE;
            positional_score[color] += piecesquare[BISHOP][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }

        /* Rooks */
        pieces = s->bitboard[NUM_TYPES*color + ROOK];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += ROOK_VALUE;
            positional_score[color] += piecesquare[ROOK][pos^pos_mask];
            pieces ^= BITBOARD_POSITION(pos);
        }
        
        /* Queens */
        pieces = s->bitboard[NUM_TYPES*color + QUEEN];
        while(pieces) {
            pos = BITBOARD_find_bit(pieces);
            material_score[color] += QUEEN_VALUE;
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

    if(score > 0) {
        if(material_score[WHITE] <= 2*BISHOP_VALUE && (!(s->bitboard[WHITE_PIECES+PAWN] | s->bitboard[WHITE_PIECES+ROOK]))) {
            score = score >> 3;
        }
    } else {
        if(material_score[BLACK] <= 2*BISHOP_VALUE && (!(s->bitboard[BLACK_PIECES+PAWN] | s->bitboard[BLACK_PIECES+ROOK]))) {
            score = -(-score >> 3);
        }
    }

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
