#include "eval.h"
#include "movegen.h"

#define PAWN_GUARDED_BY_PAWN 4

#define PAWN_DOUBLE_PAWN -10
#define PAWN_TRIPLE_PAWN -20

#define PAWN_SHIELD_1 5
#define PAWN_SHIELD_2 2

const short piece_value[NUM_TYPES] = { 20, 60, 64, 100, 180, 0 };
static const short sign[2] = { 1, -1 };

static const short pawn_double_pawn_penalty[8] = {
    0, 0, PAWN_DOUBLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN, PAWN_TRIPLE_PAWN
};

#if 0
short EVAL_pawn_structure(const chess_state_t *s)
{
    short score = 0;
    int color;
    int file_number;
    
    for(color = WHITE; color <= BLACK; color++) {
        int opponent_color = color ^ 1;
        short pawn_structure_score = 0;
        
        bitboard_t own_pawns = s->bitboard[color*NUM_TYPES + PAWN];
        
        bitboard_t pawns = own_pawns;
        while(pawns) {
            /* Get one position from the bitboard */
            int pos = BITBOARD_find_bit(pawns);

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
        
        int num_white_pawns = BITBOARD_count_bits(s->bitboard[WHITE_PIECES + PAWN] & file);
        int num_black_pawns = BITBOARD_count_bits(s->bitboard[BLACK_PIECES + PAWN] & file);
        
        score += pawn_double_pawn_penalty[num_white_pawns];
        score -= pawn_double_pawn_penalty[num_black_pawns];
    }
    
    return score;
}

short EVAL_get_pawn_score(const chess_state_t *s, hashtable_t *hashtable)
{
    short score = 0;

    /* Query pawn hash table */
    if(HASHTABLE_pawn_retrieve(hashtable, s->pawn_hash, &score)) {
        return score;
    }

    /* Not found: evaluate pawn structure */
    score = EVAL_pawn_structure(s);
    HASHTABLE_pawn_store(hashtable, s->pawn_hash, score);
    return score;
}
#endif

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

short EVAL_pawn_shield(const chess_state_t *s)
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
    short score = 0;
    int is_endgame = STATE_is_endgame(s);

    /* Pawn score */
    //score = s->score_pawn;
    
    /* Pawn shield */
    if(!is_endgame) {
        score += EVAL_pawn_shield(s);
    }
    
    score *= sign[(int)(s->player)];
    
    /* Material score */
    score += s->score_material;
    
    /* Adjust material score for endgame */
    if(is_endgame) {
        int white_king_pos = BITBOARD_find_bit(s->bitboard[WHITE_PIECES+KING]);
        int black_king_pos = BITBOARD_find_bit(s->bitboard[BLACK_PIECES+KING]) ^ 0x38;
        score += (
            -piecesquare[KING][white_king_pos]
            +piecesquare[KING+1][white_king_pos]
            +piecesquare[KING][black_king_pos]
            -piecesquare[KING+1][black_king_pos]
        ) * sign[(int)(s->player)];
    }
    
    return score;
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
    if(bitboard_bishop[pos] & (s->bitboard[opponent_index + BISHOP] | s->bitboard[opponent_index + QUEEN])) {
        MOVEGEN_bishop(pos, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &dummy, &attackers);
        attackers &= (s->bitboard[opponent_index + BISHOP] | s->bitboard[opponent_index + QUEEN]);
        if(attackers) {
            return 1;
        }
    }

    /* Is attacked by straight sliders (rook, queen)? */
    if(bitboard_rook[pos] & (s->bitboard[opponent_index + ROOK] | s->bitboard[opponent_index + QUEEN])) {
        MOVEGEN_rook(pos, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &dummy, &attackers);
        attackers &= (s->bitboard[opponent_index + ROOK] | s->bitboard[opponent_index + QUEEN]);
        if(attackers) {
            return 1;
        }
    }
    
    /* Is attacked by king */
    attackers = bitboard_king[pos] & s->bitboard[opponent_index + KING];
    if(attackers) {
        return 1;
    }
    
    return 0;
}
