#include "eval.h"
#include "movegen.h"

#define PAWN_RANK0 0
#define PAWN_RANK1 0
#define PAWN_RANK2 5
#define PAWN_RANK3 10
#define PAWN_RANK4 18
#define PAWN_RANK5 30
#define PAWN_RANK6 50
#define PAWN_RANK7 0

#define PAWN_GUARDED_BY_PAWN 10

static int sign[2] = { 1, -1 };

static int pawn_rank_bonus[2][8] = {
    { PAWN_RANK0, PAWN_RANK1, PAWN_RANK2, PAWN_RANK3, PAWN_RANK4, PAWN_RANK5, PAWN_RANK6, PAWN_RANK7 },
    { PAWN_RANK7, PAWN_RANK6, PAWN_RANK5, PAWN_RANK4, PAWN_RANK3, PAWN_RANK2, PAWN_RANK1, PAWN_RANK0 }
};

int EVAL_evaluate_board(chess_state_t *s)
{
    int color;
    int score = 0;

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
                        
    for(color = WHITE; color <= BLACK; color++) {
        int opponent_color = color ^ 1;
        int pawn_assessment = 0;
        bitboard_t pawns = s->bitboard[color*NUM_TYPES + PAWN];
        while(pawns) {
            /* Get one position from the bitboard */
            int pos = bitboard_find_bit(pawns);
            int rank = BITBOARD_GET_RANK(pos);
            
            /* Bonus for advancing pawns to high ranks */
            pawn_assessment += pawn_rank_bonus[color][rank];
            
            /* Bonus for being guarded by other pawn */
            if(bitboard_pawn_capture[opponent_color][pos] & s->bitboard[color*NUM_TYPES + PAWN]) {
                pawn_assessment += PAWN_GUARDED_BY_PAWN;
            }

            /* Clear position from bitboard */
            pawns ^= BITBOARD_POSITION(pos);
        }
        score += pawn_assessment * sign[color];
    }
    
    /* Switch sign of evaluation if it is black's turn */
    return score * sign[(int)(s->player)];
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
    MOVEGEN_bishop(player, pos, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &dummy, &attackers);
    attackers &= (s->bitboard[opponent_index + BISHOP] | s->bitboard[opponent_index + QUEEN]);
    if(attackers) {
        return 1;
    }

    /* Is attacked by straight sliders (rook, queen)? */
    MOVEGEN_rook(player, pos, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &dummy, &attackers);
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
