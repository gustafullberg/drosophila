#include "eval.h"
#include "eval_piecesquare.h"
#include "movegen.h"

#define PAWN_RANK0 0
#define PAWN_RANK1 0
#define PAWN_RANK2 5
#define PAWN_RANK3 10
#define PAWN_RANK4 18
#define PAWN_RANK5 30
#define PAWN_RANK6 50
#define PAWN_RANK7 0

#define PAWN_GUARDED_BY_PAWN 20

#define PAWN_DOUBLE_PAWN -50
#define PAWN_TRIPLE_PAWN -100

static const int sign[2] = { 1, -1 };

static const int pawn_double_pawn_penalty[8] = {
    0, 0, PAWN_DOUBLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN
};

static int pawn_structure_assessment(const chess_state_t *s)
{
    int score = 0;
    int color;
    int file_number;
    
    for(color = WHITE; color <= BLACK; color++) {
        int opponent_color = color ^ 1;
        int pawn_structure_score = 0;
        
        bitboard_t own_pawns = s->bitboard[color*NUM_TYPES + PAWN];
        /*bitboard_t opponent_pawns = s->bitboard[opponent_color*NUM_TYPES + PAWN];*/
        
        bitboard_t pawns = own_pawns;
        while(pawns) {
            /* Get one position from the bitboard */
            int pos = bitboard_find_bit(pawns);
            /*int file = BITBOARD_GET_RANK(pos);*/
            /*int rank = BITBOARD_GET_RANK(pos);*/
            
            /* Bonus for advancing pawns to high ranks */
            /*pawn_structure_score += pawn_rank_bonus[color][rank];*/

            /* Bonus for being guarded by other pawn */
            if(bitboard_pawn_capture[opponent_color][pos] & own_pawns) {
                pawn_structure_score += PAWN_GUARDED_BY_PAWN;
            }
            
            /* Clear position from bitboard */
            pawns ^= BITBOARD_POSITION(pos);
        }
        
        /* Sign of score depends on color */
        score += pawn_structure_score * sign[color];
    }
    
    /* Penalty for double/triple pawns */
    for(file_number = 0; file_number < 8; file_number++) {
        bitboard_t file = BITBOARD_FILE << file_number;
        
        int num_white_pawns = bitboard_count_bits(s->bitboard[WHITE_PIECES + PAWN] & file);
        int num_black_pawns = bitboard_count_bits(s->bitboard[BLACK_PIECES + PAWN] & file);
        
        score += pawn_double_pawn_penalty[num_white_pawns];
        score -= pawn_double_pawn_penalty[num_black_pawns];
    }
    
    return score;
}

static int EVAL_piecesquare(const chess_state_t *s)
{
    bitboard_t pieces;
    int type;
    int result = 0;
    
    /* White pawns */
    for(type = PAWN; type < KING; type++) {
        const int *psq = piecesquare[type];
        pieces = s->bitboard[WHITE_PIECES+type];
        while(pieces) {
            int pos = bitboard_find_bit(pieces);
            result += psq[pos];
            pieces ^= BITBOARD_POSITION(pos);
        }
        pieces = s->bitboard[BLACK_PIECES+type];
        while(pieces) {
            int pos = bitboard_find_bit(pieces);
            result -= psq[pos^0x38];
            pieces ^= BITBOARD_POSITION(pos);
        }
    }
    
    {
        const int *psq = piecesquare[KING + STATE_is_endgame(s)];
        result += psq[bitboard_find_bit(s->bitboard[WHITE_PIECES+KING])];
        result -= psq[bitboard_find_bit(s->bitboard[BLACK_PIECES+KING])^0x38];
    }

    return result;
}



int EVAL_evaluate_board(const chess_state_t *s)
{
    int score = 0;

    score +=    900 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+QUEEN])  - bitboard_count_bits(s->bitboard[BLACK_PIECES+QUEEN]));
    score +=    500 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+ROOK])   - bitboard_count_bits(s->bitboard[BLACK_PIECES+ROOK]));
    score +=    320 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+BISHOP]) - bitboard_count_bits(s->bitboard[BLACK_PIECES+BISHOP]));
    score +=    300 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+KNIGHT]) - bitboard_count_bits(s->bitboard[BLACK_PIECES+KNIGHT]));
    score +=    100 * (bitboard_count_bits(s->bitboard[WHITE_PIECES+PAWN])   - bitboard_count_bits(s->bitboard[BLACK_PIECES+PAWN]));
    
    score += EVAL_piecesquare(s);
    score += pawn_structure_assessment(s);
    
    /* Down-sample score for faster MTD(f) convergence */
    score = score >> 2;
    
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
