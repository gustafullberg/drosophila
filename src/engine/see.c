#include "see.h"
#include "eval.h"

static short piece_value[] = { 1, 3, 3, 5, 9, 20 };

static bitboard_t SEE_find_all_attackers(const chess_state_t *s, const bitboard_t occupied, const int pos, bitboard_t *blocked_attackers)
{
    /* Create a bitboard containing all pieces of both sides attacking a certain square */
    bitboard_t blocked = 0;
    bitboard_t potential_attackers;
    bitboard_t attackers = 0;
    const bitboard_t bishops  = s->bitboard[WHITE_PIECES + BISHOP] | s->bitboard[BLACK_PIECES + BISHOP];
    const bitboard_t rooks    = s->bitboard[WHITE_PIECES + ROOK]   | s->bitboard[BLACK_PIECES + ROOK];
    const bitboard_t queens   = s->bitboard[WHITE_PIECES + QUEEN]  | s->bitboard[BLACK_PIECES + QUEEN];
    const bitboard_t kings    = s->bitboard[WHITE_PIECES + KING]   | s->bitboard[BLACK_PIECES + KING];
    
    /* Attacking pawns */
    attackers |= bitboard_pawn_capture[BLACK][pos] & s->bitboard[WHITE_PIECES + PAWN];
    attackers |= bitboard_pawn_capture[WHITE][pos] & s->bitboard[BLACK_PIECES + PAWN];
    
    /* Attacking knights */
    attackers |= bitboard_knight[pos] & (s->bitboard[WHITE_PIECES + KNIGHT] | s->bitboard[BLACK_PIECES + KNIGHT]);
    
    /* Attacking bishops, rooks and queens */
    potential_attackers =
        (bitboard_bishop[pos] & (bishops | queens)) |
        (bitboard_rook[pos]   & (rooks   | queens));
    while(potential_attackers) {
        int attacker_pos = BITBOARD_find_bit(potential_attackers);
        if(!(bitboard_between[attacker_pos][pos] & occupied)) {
            attackers |= BITBOARD_POSITION(attacker_pos);
        } else {
            blocked |= BITBOARD_POSITION(attacker_pos);
        }
        potential_attackers ^= BITBOARD_POSITION(attacker_pos);
    }
    
    /* Attacking kings */
    attackers |= bitboard_king[pos] & kings;
    
    *blocked_attackers = blocked;
    attackers &= occupied;
    return attackers;
}

static int SEE_find_least_attacker(const chess_state_t *s, bitboard_t *occupied, bitboard_t *attackers, bitboard_t *blocked_attackers, const int pos, const int color)
{
    int attacker_pieces = color*NUM_TYPES;
    bitboard_t attackers_side = (*attackers & s->bitboard[attacker_pieces + ALL]);

    /* Check if there are any attackers left */
    if(attackers_side) {
        int type;
        int attacker_pos = 0;
        
        /* Find least valuable attacker */
        for(type = PAWN; type <= KING; type++) {
            bitboard_t attackers_of_type = s->bitboard[attacker_pieces + type] & attackers_side;
            if(attackers_of_type) {
                attacker_pos = BITBOARD_find_bit(attackers_of_type);
                break;
            }
        }
        
        /* Remove attacker from bitboard */
        *occupied  ^= BITBOARD_POSITION(attacker_pos);
        *attackers &= *occupied;
        
        /* Add "hidden" attacker */
        if(type == PAWN || type == BISHOP || type == QUEEN) {
            /* Hidden bishop or queen? */
            bitboard_t potential_attackers = bitboard_bishop[pos] & *blocked_attackers;
            while(potential_attackers) {
                int potential_attacker_pos = BITBOARD_find_bit(potential_attackers);
                bitboard_t bit = BITBOARD_POSITION(potential_attacker_pos);
                if(!(bitboard_between[potential_attacker_pos][pos] & *occupied)) {
                    *blocked_attackers ^= bit;
                    *attackers |= bit;
                }
                potential_attackers ^= bit;
            }
        }
        
        if(type == ROOK || type == QUEEN) {
            /* Hidden rook or queen? */
            bitboard_t potential_attackers = bitboard_rook[pos] & *blocked_attackers;
            while(potential_attackers) {
                int potential_attacker_pos = BITBOARD_find_bit(potential_attackers);
                bitboard_t bit = BITBOARD_POSITION(potential_attacker_pos);
                if(!(bitboard_between[potential_attacker_pos][pos] & *occupied)) {
                    *blocked_attackers ^= bit;
                    *attackers |= bit;
                }
                potential_attackers ^= bit;
            }
        }

        return type;
    }
    
    return -1;
}

short see(const chess_state_t *s, const move_t move)
{
    bitboard_t attackers, blocked_attackers;
    int type, last_type;
    short swap_list[32];
    int swap_idx = 2;
    int pos = MOVE_GET_POS_TO(move);
    bitboard_t occupied = s->bitboard[OCCUPIED];
    int color = s->player;
    
    /* Remove initial capturer */
    occupied ^= BITBOARD_POSITION(MOVE_GET_POS_FROM(move));
    
    /* Remove captured piece if en passant */
    if(MOVE_GET_SPECIAL_FLAGS(move) == MOVE_EP_CAPTURE) {
        occupied ^= bitboard_ep_capture[pos];
    }
    
    /* Add first captured piece to swap list */
    if(MOVE_IS_CAPTURE(move)) {
        type = MOVE_GET_CAPTURE_TYPE(move);
        swap_list[0] = piece_value[type];
    } else {
        swap_list[0] = 0;
    }
    
    /* Add first capturing piece to swap list */
    last_type = type = MOVE_GET_TYPE(move);
    swap_list[1] = piece_value[type] - swap_list[0];
    
    /* Get pieces that can attack the square in question */
    attackers = SEE_find_all_attackers(s, occupied, pos, &blocked_attackers);

    /* Fill the swap list */
    color ^= 1;
    while((type = SEE_find_least_attacker(s, &occupied, &attackers, &blocked_attackers, pos, color)) >= 0) {
        swap_list[swap_idx] = piece_value[type] - swap_list[swap_idx-1];
        swap_idx++;

        /* Stop swapping pieces when a king is captured */
        if(last_type == KING) {
            break;
        }

        last_type = type;
        color ^= 1;
    }
    
    /* Last piece in the list is never captured */
    swap_idx -= 1;

    /* swap_list[i] = -MAX(-swap_list[i], swap_list[i+1]) */
    while(--swap_idx) {
        swap_list[swap_idx-1] = (-swap_list[swap_idx-1] > swap_list[swap_idx]) ? swap_list[swap_idx-1] : -swap_list[swap_idx];
    }
    
    return swap_list[0];
}

int SEE_capture_less_valuable(const move_t move)
{
    return piece_value[MOVE_GET_CAPTURE_TYPE(move)] < piece_value[MOVE_GET_TYPE(move)];
}
