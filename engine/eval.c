#include "eval.h"

int EVAL_evaluate_board(chess_state_t *s)
{
    int score = 0;
    score += 100000 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+KING])   - bitboard_count_bits(s->bitboard[BLACK_PIECES+KING]));
    score +=    900 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+QUEEN])  - bitboard_count_bits(s->bitboard[BLACK_PIECES+QUEEN]));
    score +=    500 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+ROOK])   - bitboard_count_bits(s->bitboard[BLACK_PIECES+ROOK]));
    score +=    300 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+BISHOP]) - bitboard_count_bits(s->bitboard[BLACK_PIECES+BISHOP]));
    score +=    300 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+KNIGHT]) - bitboard_count_bits(s->bitboard[BLACK_PIECES+KNIGHT]));
    score +=    100 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+PAWN])   - bitboard_count_bits(s->bitboard[BLACK_PIECES+PAWN]));
    
    score -=      5 * (bitboard_count_bits(bitboard_bad_pawn[WHITE] & s->bitboard[WHITE_PIECES+PAWN]) - 
                        bitboard_count_bits(bitboard_bad_pawn[BLACK] & s->bitboard[BLACK_PIECES+PAWN]));
    score -=      5 * (bitboard_count_bits(bitboard_bad_knight[WHITE] & s->bitboard[WHITE_PIECES+KNIGHT]) - 
                        bitboard_count_bits(bitboard_bad_knight[BLACK] & s->bitboard[BLACK_PIECES+KNIGHT]));
    score -=      5 * (bitboard_count_bits(bitboard_bad_bishop[WHITE] & s->bitboard[WHITE_PIECES+BISHOP]) - 
                        bitboard_count_bits(bitboard_bad_bishop[BLACK] & s->bitboard[BLACK_PIECES+BISHOP]));
    
    /* Switch sign of evaluation if it is black's turn */
    return score - 2 * score * (int)(s->player);
}

int EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos)
{
    const int player = color;
    const int own_index = player * NUM_TYPES;
    const int opponent_index = NUM_TYPES - own_index;
    
    bitboard_t attackers, dummy;
    
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
    
    /* Is attacked by diagonal sliders (bishop, queen)? */
    move_bishop(player, pos, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &dummy, &attackers);
    attackers &= (s->bitboard[opponent_index + BISHOP] | s->bitboard[opponent_index + QUEEN]);
    if(attackers) {
        return 1;
    }

    /* Is attacked by straight sliders (rook, queen)? */
    move_rook(player, pos, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &dummy, &attackers);
    attackers &= (s->bitboard[opponent_index + ROOK] | s->bitboard[opponent_index + QUEEN]);
    if(attackers) {
        return 1;
    }
    
    /* Is attacked by king */
    attackers = bitboard_king[pos] & s->bitboard[opponent_index + KING];
    if(attackers) {
        return 1;
    }
    
    return 0;
}
