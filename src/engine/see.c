#include "see.h"
#include "eval.h"

static short piece_value[] = { PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, 10*QUEEN_VALUE };

static short SEE_least_valuable_attacker(const chess_state_t *s, const int color, bitboard_t own_pieces, bitboard_t opponent_pieces, const int pos, int *attacker_pos)
{
    const int player = color;
    const int own_index = player * NUM_TYPES;
    const int opponent_index = NUM_TYPES - own_index;
    
    bitboard_t attackers;
    bitboard_t occupied = own_pieces & opponent_pieces;
    
    /* Is attacked by pawns? */
    attackers = bitboard_pawn_capture[player][pos] & s->bitboard[opponent_index + PAWN] & opponent_pieces;
    if(attackers) {
        *attacker_pos = BITBOARD_find_bit(attackers);
        return piece_value[PAWN];
    }
    
    /* Is attacked by knights? */
    attackers = bitboard_knight[pos] & s->bitboard[opponent_index + KNIGHT] & opponent_pieces;
    if(attackers) {
        *attacker_pos = BITBOARD_find_bit(attackers);
        return piece_value[KNIGHT];
    }
        
    /* Is attacked by bishops? */
    attackers = bitboard_bishop[pos] & s->bitboard[opponent_index + BISHOP] & opponent_pieces;
    while(attackers) {
        *attacker_pos = BITBOARD_find_bit(attackers);
        if((bitboard_between[*attacker_pos][pos] & occupied) == 0) {
            return piece_value[BISHOP];
        }
        attackers ^= BITBOARD_POSITION(*attacker_pos);
    }

    /* Is attacked by rooks? */
    attackers = bitboard_rook[pos] & s->bitboard[opponent_index + ROOK] & opponent_pieces;
    while(attackers) {
        *attacker_pos = BITBOARD_find_bit(attackers);
        if((bitboard_between[*attacker_pos][pos] & occupied) == 0) {
            return piece_value[ROOK];
        }
        attackers ^= BITBOARD_POSITION(*attacker_pos);
    }
    
    /* Is attacked by queens? */
    attackers = (bitboard_bishop[pos] | bitboard_rook[pos]) & s->bitboard[opponent_index + QUEEN] & opponent_pieces;
    while(attackers) {
        *attacker_pos = BITBOARD_find_bit(attackers);
        if((bitboard_between[*attacker_pos][pos] & occupied) == 0) {
            return piece_value[QUEEN];
        }
        attackers ^= BITBOARD_POSITION(*attacker_pos);
    }

    /* Is attacked by king? */
    attackers = bitboard_king[pos] & s->bitboard[opponent_index + KING] & opponent_pieces;
    if(attackers) {
        *attacker_pos = BITBOARD_find_bit(attackers);
        return piece_value[KING];
    }
    
    return 0;
}


short see(const chess_state_t *s, const move_t move)
{
    int attacked_pos = MOVE_GET_POS_TO(move);
    int score = piece_value[MOVE_GET_CAPTURE_TYPE(move)];
    int victim_value = piece_value[MOVE_GET_TYPE(move)];
    int attacker_pos = MOVE_GET_POS_FROM(move);
    int attacker_value = 0;
    
    bitboard_t pieces[2];
    /* Own pieces */
    pieces[0] = s->bitboard[(int)s->player*NUM_TYPES + ALL];
    /* Opponent pieces */
    pieces[1] = s->bitboard[(int)(s->player^1)*NUM_TYPES + ALL];
    
    while(1) {
        /* Clear attacker position */
        pieces[0] ^= BITBOARD_POSITION(attacker_pos);
        
        if(score < 0) {
            return score;
        }
        
        /* Find least valuable attacker */
        attacker_value = SEE_least_valuable_attacker(s, s->player, pieces[0], pieces[1], attacked_pos, &attacker_pos);
        if(!attacker_value) {
            return score;
        }
        score -= victim_value;
        victim_value = attacker_value;
        
        /* Clear attacker position */
        pieces[1] ^= BITBOARD_POSITION(attacker_pos);
        
        if(score > 0) {
            return score;
        }
        
        /* Find least valuable attacker */
        attacker_value = SEE_least_valuable_attacker(s, s->player^1, pieces[1], pieces[0], attacked_pos, &attacker_pos);
        if(!attacker_value) {
            return score;
        }
        score += victim_value;
        victim_value = attacker_value;
    }
    
    return score;
}
