#include "bitboard.h"
#include <immintrin.h>
#include <stdio.h>

bitboard_t bitboard_file[NUM_POSITIONS];
bitboard_t bitboard_rank[NUM_POSITIONS];
bitboard_t bitboard_left[NUM_POSITIONS];
bitboard_t bitboard_right[NUM_POSITIONS];
bitboard_t bitboard_up[NUM_POSITIONS];
bitboard_t bitboard_down[NUM_POSITIONS];
bitboard_t bitboard_up_left[NUM_POSITIONS];
bitboard_t bitboard_up_right[NUM_POSITIONS];
bitboard_t bitboard_down_left[NUM_POSITIONS];
bitboard_t bitboard_down_right[NUM_POSITIONS];
bitboard_t bitboard_between[NUM_POSITIONS][NUM_POSITIONS];
bitboard_t bitboard_king[NUM_POSITIONS];
bitboard_t bitboard_knight[NUM_POSITIONS];
bitboard_t bitboard_pawn_move[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_pawn_capture[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_ep_capture[NUM_POSITIONS];
bitboard_t bitboard_ep_capturers[NUM_COLORS][NUM_FILES];
bitboard_t bitboard_bishop[NUM_POSITIONS];
bitboard_t bitboard_rook[NUM_POSITIONS];
bitboard_t bitboard_king_castle_empty[NUM_COLORS];
bitboard_t bitboard_queen_castle_empty[NUM_COLORS];
bitboard_t bitboard_start_position[NUM_COLORS][NUM_TYPES-1];
char       distance[NUM_POSITIONS][NUM_POSITIONS];
#ifdef __BMI2__
bitboard_t bitboard_rook_inner[NUM_POSITIONS];
bitboard_t bitboard_bishop_inner[NUM_POSITIONS];
bitboard_t bitboard_rook_attacks[102400];
bitboard_t bitboard_bishop_attacks[5248];
int rook_attacks_offset[NUM_POSITIONS];
int bishop_attacks_offset[NUM_POSITIONS];
#endif

void BITBOARD_init()
{
    bitboard_t bitboard_less_than[NUM_POSITIONS];
    bitboard_t bitboard_more_than[NUM_POSITIONS];
    bitboard_t bitboard_bltr[NUM_POSITIONS];
    bitboard_t bitboard_tlbr[NUM_POSITIONS];
    int i, j, tmp, rank, file, color, offset;
    
    /* LESS THAN */
    bitboard_less_than[0] = 0;
    for(i = 1; i < NUM_POSITIONS; i++)
        bitboard_less_than[i] = bitboard_less_than[i-1] | BITBOARD_POSITION(i-1);
    
    /* MORE THAN */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_more_than[i] = ~bitboard_less_than[i] & ~BITBOARD_POSITION(i);
    
    /* RANK */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_rank[i] = BITBOARD_RANK << (8 * BITBOARD_GET_RANK(i));
    
    /* FILE */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_file[i] = BITBOARD_FILE << BITBOARD_GET_FILE(i);
    
    /* DIAG BL-TR */
    for(i = 0; i < NUM_POSITIONS; i++) {
        bitboard_bltr[i] = 0;
        tmp = i;
        while(BITBOARD_POS_VALID(BITBOARD_GET_RANK(tmp)+1, BITBOARD_GET_FILE(tmp)+1))
            tmp += 9;
        bitboard_bltr[i] |= BITBOARD_POSITION(tmp);
        while(BITBOARD_POS_VALID(BITBOARD_GET_RANK(tmp)-1, BITBOARD_GET_FILE(tmp)-1)) {
            tmp -= 9;
            bitboard_bltr[i] |= BITBOARD_POSITION(tmp);
        }
    }
    
    /* DIAG TL-BR */
    for(i = 0; i < NUM_POSITIONS; i++) {
        bitboard_tlbr[i] = 0;
        tmp = i;
        while(BITBOARD_POS_VALID(BITBOARD_GET_RANK(tmp)+1, BITBOARD_GET_FILE(tmp)-1))
            tmp += 7;
        bitboard_tlbr[i] |= BITBOARD_POSITION(tmp);
        while(BITBOARD_POS_VALID(BITBOARD_GET_RANK(tmp)-1, BITBOARD_GET_FILE(tmp)+1)) {
            tmp -= 7;
            bitboard_tlbr[i] |= BITBOARD_POSITION(tmp);
        }
    }
    
    /* LEFT */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_left[i] = bitboard_rank[i] & bitboard_less_than[i];
    
    /* RIGHT */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_right[i] = bitboard_rank[i] & bitboard_more_than[i];
    
    /* UP */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_up[i] = bitboard_file[i] & bitboard_more_than[i];
    
    /* DOWN */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_down[i] = bitboard_file[i] & bitboard_less_than[i];
    
    /* UP-LEFT */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_up_left[i] = bitboard_tlbr[i] & bitboard_more_than[i];
    
    /* UP-RIGHT */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_up_right[i] = bitboard_bltr[i] & bitboard_more_than[i];
    
    /* DOWN-LEFT */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_down_left[i] = bitboard_bltr[i] & bitboard_less_than[i];
    
    /* DOWN-RIGHT */
    for(i = 0; i < NUM_POSITIONS; i++)
        bitboard_down_right[i] = bitboard_tlbr[i] & bitboard_less_than[i];
    
    /* BETWEEN */
    for(i = 0; i < NUM_POSITIONS; i++) {
        for(j = 0; j < NUM_POSITIONS; j++) {
            bitboard_between[i][j] = 0;
            if(BITBOARD_POSITION(j) & bitboard_bltr[i]) bitboard_between[i][j] = bitboard_bltr[i];
            if(BITBOARD_POSITION(j) & bitboard_tlbr[i]) bitboard_between[i][j] = bitboard_tlbr[i];
            if(BITBOARD_POSITION(j) & bitboard_file[i]) bitboard_between[i][j] = bitboard_file[i];
            if(BITBOARD_POSITION(j) & bitboard_rank[i]) bitboard_between[i][j] = bitboard_rank[i];
            if(bitboard_between[i][j]) {
                if(i < j) {
                    bitboard_between[i][j] &= bitboard_more_than[i] & bitboard_less_than[j];
                } else {
                    bitboard_between[i][j] &= bitboard_more_than[j] & bitboard_less_than[i];
                }
            }
        }
    }

    /* KING */
    for(i = 0; i < NUM_POSITIONS; i++) {
        rank = BITBOARD_GET_RANK(i);
        file = BITBOARD_GET_FILE(i);
        bitboard_king[i] = 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank-1, file-1)) ?  BITBOARD_RANK_FILE(rank-1, file-1) : 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank-1, file  )) ?  BITBOARD_RANK_FILE(rank-1, file  ) : 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank-1, file+1)) ?  BITBOARD_RANK_FILE(rank-1, file+1) : 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank  , file-1)) ?  BITBOARD_RANK_FILE(rank  , file-1) : 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank  , file+1)) ?  BITBOARD_RANK_FILE(rank  , file+1) : 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank+1, file-1)) ?  BITBOARD_RANK_FILE(rank+1, file-1) : 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank+1, file  )) ?  BITBOARD_RANK_FILE(rank+1, file  ) : 0;
        bitboard_king[i] |= (BITBOARD_POS_VALID(rank+1, file+1)) ?  BITBOARD_RANK_FILE(rank+1, file+1) : 0;
    }
    
    /* KNIGHT */
    for(i = 0; i < NUM_POSITIONS; i++) {
        rank = BITBOARD_GET_RANK(i);
        file = BITBOARD_GET_FILE(i);
        bitboard_knight[i] = 0;
        if(BITBOARD_POS_VALID(rank-2, file-1)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank-2, file-1);
        if(BITBOARD_POS_VALID(rank-1, file-2)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank-1, file-2);
        if(BITBOARD_POS_VALID(rank-2, file+1)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank-2, file+1);
        if(BITBOARD_POS_VALID(rank-1, file+2)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank-1, file+2);
        if(BITBOARD_POS_VALID(rank+2, file-1)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank+2, file-1);
        if(BITBOARD_POS_VALID(rank+1, file-2)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank+1, file-2);
        if(BITBOARD_POS_VALID(rank+2, file+1)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank+2, file+1);
        if(BITBOARD_POS_VALID(rank+1, file+2)) bitboard_knight[i] |= BITBOARD_RANK_FILE(rank+1, file+2);
    }
    
    /* PAWN */
    for(color = 0; color < NUM_COLORS; color++) {
        if(color == 0) {
            offset = 1;
        } else {
            offset = -1;
        }
        for(i = 0; i < NUM_POSITIONS; i++) {
            rank = BITBOARD_GET_RANK(i);
            file = BITBOARD_GET_FILE(i);
            bitboard_pawn_move[color][i] = 0;
            bitboard_pawn_capture[color][i] = 0;
            if(BITBOARD_POS_VALID(rank+offset, file)) bitboard_pawn_move[color][i] = BITBOARD_RANK_FILE(rank+offset, file);
            if(BITBOARD_POS_VALID(rank+offset, file-1)) bitboard_pawn_capture[color][i] |= BITBOARD_RANK_FILE(rank+offset, file-1);
            if(BITBOARD_POS_VALID(rank+offset, file+1)) bitboard_pawn_capture[color][i] |= BITBOARD_RANK_FILE(rank+offset, file+1);
        }
    }
    
    /* EN PASSANT CAPTURE */
    for(i = 0; i < NUM_POSITIONS; i++) {
        bitboard_ep_capture[i] = 0;
        if(BITBOARD_GET_RANK(i) == 2) {
            BITBOARD_SET(bitboard_ep_capture[i], i+8);
        } else if(BITBOARD_GET_RANK(i) == 5) {
            BITBOARD_SET(bitboard_ep_capture[i], i-8);
        }
    }
    
    /* EN PASSANT CAPTURERS */
    for(color = 0; color < NUM_COLORS; color++) {
        /* WHITE => rank = 4, BLACK => rank = 3 */
        rank = 4 - color;
        for(i = 0; i < NUM_FILES; i++) {
            bitboard_ep_capturers[color][i] = 0;
            if(i > 0) {
                bitboard_ep_capturers[color][i] |= BITBOARD_RANK_FILE(rank, i-1);
            }
            if(i < 7) {
                bitboard_ep_capturers[color][i] |= BITBOARD_RANK_FILE(rank, i+1);
            }
        }
    }
    
    /* BISHOP */
    for(i = 0; i < NUM_POSITIONS; i++) {
        bitboard_bishop[i] = (bitboard_bltr[i] | bitboard_tlbr[i]) ^ BITBOARD_POSITION(i);
    }
    
    /* ROOK */
    for(i = 0; i < NUM_POSITIONS; i++) {
        bitboard_rook[i] = (bitboard_rank[i] | bitboard_file[i]) ^ BITBOARD_POSITION(i);
    }
    
    /* CASTLING */
    bitboard_king_castle_empty[WHITE]  = 0x0000000000000060;
    bitboard_king_castle_empty[BLACK]  = 0x6000000000000000;
    bitboard_queen_castle_empty[WHITE] = 0x000000000000000E;
    bitboard_queen_castle_empty[BLACK] = 0x0E00000000000000;
    
    /* START POSITIONS */
    bitboard_start_position[WHITE][PAWN]    = 0x000000000000FF00;
    bitboard_start_position[WHITE][KNIGHT]  = 0x0000000000000042;
    bitboard_start_position[WHITE][BISHOP]  = 0x0000000000000024;
    bitboard_start_position[WHITE][ROOK]    = 0x0000000000000081;
    bitboard_start_position[WHITE][QUEEN]   = 0x0000000000000008;
    bitboard_start_position[WHITE][KING]    = 0x0000000000000010;
    bitboard_start_position[BLACK][PAWN]    = 0x00FF000000000000;
    bitboard_start_position[BLACK][KNIGHT]  = 0x4200000000000000;
    bitboard_start_position[BLACK][BISHOP]  = 0x2400000000000000;
    bitboard_start_position[BLACK][ROOK]    = 0x8100000000000000;
    bitboard_start_position[BLACK][QUEEN]   = 0x0800000000000000;
    bitboard_start_position[BLACK][KING]    = 0x1000000000000000;

    for(i = 0; i < NUM_POSITIONS; i++) {
        for(j = 0; j < NUM_POSITIONS; j++) {
            int dx, dy;
            dx = BITBOARD_GET_FILE(j) - BITBOARD_GET_FILE(i);
            dy = BITBOARD_GET_RANK(j) - BITBOARD_GET_RANK(i);
            if(dx < 0) dx = -dx;
            if(dy < 0) dy = -dy;
            if(dx < dy) dx = dy;
            distance[i][j] = (char)dx;
        }
    }

#ifdef __BMI2__
    for(i = 0; i < NUM_POSITIONS; i++) {
        bitboard_rook_inner[i] = (bitboard_up[i] & ~bitboard_rank[63]) |
            (bitboard_down[i] & ~bitboard_rank[0]) |
            (bitboard_left[i] & ~bitboard_file[0]) |
            (bitboard_right[i] & ~bitboard_file[63]);
    }

    for(i = 0; i < NUM_POSITIONS; i++) {
        bitboard_bishop_inner[i] = bitboard_bishop[i] & 0x007E7E7E7E7E7E00;
    }

    int rook_offset = 0;
    for(i = 0; i < NUM_POSITIONS; i++) {
        rook_attacks_offset[i] = rook_offset;
        const int num_inner_bits = BITBOARD_count_bits(bitboard_rook_inner[i]);
        const int num_permutations = 1 << num_inner_bits;

        for(j = 0; j < num_permutations; j++) {
            const bitboard_t occ = _pdep_u64(j, bitboard_rook_inner[i]);
            bitboard_t moves_to_test = bitboard_rook[i];
            bitboard_t possible_moves = 0;
            while(moves_to_test) {
                const int pos = BITBOARD_find_bit(moves_to_test);
                const bitboard_t pos_bitboard = BITBOARD_POSITION(pos);
                moves_to_test ^= pos_bitboard;
                if((bitboard_between[i][pos] & occ) == 0) possible_moves |= pos_bitboard;
            }
            bitboard_rook_attacks[rook_offset++] = possible_moves;
        }
    }

    int bishop_offset = 0;
    for(i = 0; i < NUM_POSITIONS; i++) {
        bishop_attacks_offset[i] = bishop_offset;
        const int num_inner_bits = BITBOARD_count_bits(bitboard_bishop_inner[i]);
        const int num_permutations = 1 << num_inner_bits;

        for(j = 0; j < num_permutations; j++) {
            const bitboard_t occ = _pdep_u64(j, bitboard_bishop_inner[i]);
            bitboard_t moves_to_test = bitboard_bishop[i];
            bitboard_t possible_moves = 0;
            while(moves_to_test) {
                const int pos = BITBOARD_find_bit(moves_to_test);
                const bitboard_t pos_bitboard = BITBOARD_POSITION(pos);
                moves_to_test ^= pos_bitboard;
                if((bitboard_between[i][pos] & occ) == 0) possible_moves |= pos_bitboard;
            }
            bitboard_bishop_attacks[bishop_offset++] = possible_moves;
        }
    }
#endif
}

void BITBOARD_print_debug(const bitboard_t bitboard)
{
    int rank, file;
    for(rank = 7; rank >= 0; rank--) {
        fprintf(stdout, "#%c ", rank + '1');
        for(file = 0; file < 8; file++) {
            fprintf(stdout, "%d ", (bitboard & BITBOARD_RANK_FILE(rank, file)) ? 1 : 0);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "   A B C D E F G H\n");
}

