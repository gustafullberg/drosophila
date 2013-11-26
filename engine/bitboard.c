#include "bitboard.h"
#include <stdio.h>

bitboard_t bitboard_less_than[NUM_POSITIONS];
bitboard_t bitboard_more_than[NUM_POSITIONS];
bitboard_t bitboard_file[NUM_POSITIONS];
bitboard_t bitboard_rank[NUM_POSITIONS];
bitboard_t bitboard_bltr[NUM_POSITIONS];
bitboard_t bitboard_tlbr[NUM_POSITIONS];
bitboard_t bitboard_left[NUM_POSITIONS];
bitboard_t bitboard_right[NUM_POSITIONS];
bitboard_t bitboard_up[NUM_POSITIONS];
bitboard_t bitboard_down[NUM_POSITIONS];
bitboard_t bitboard_up_left[NUM_POSITIONS];
bitboard_t bitboard_up_right[NUM_POSITIONS];
bitboard_t bitboard_down_left[NUM_POSITIONS];
bitboard_t bitboard_down_right[NUM_POSITIONS];
bitboard_t bitboard_king[NUM_POSITIONS];
bitboard_t bitboard_knight[NUM_POSITIONS];
bitboard_t bitboard_pawn_move[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_pawn_move2[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_pawn_capture[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_ep_capture[NUM_POSITIONS];
bitboard_t bitboard_ep_capturers[NUM_COLORS][NUM_FILES];

void bitboard_init()
{
    bitboard_t i, tmp;
    int rank, file, color, offset, base;
    
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
            base = 1;
        } else {
            offset = -1;
            base = 6;
        }
        for(i = 0; i < NUM_POSITIONS; i++) {
            rank = BITBOARD_GET_RANK(i);
            file = BITBOARD_GET_FILE(i);
            bitboard_pawn_move[color][i] = 0;
            bitboard_pawn_move2[color][i] = 0;
            bitboard_pawn_capture[color][i] = 0;
            if(BITBOARD_POS_VALID(rank+offset, file)) bitboard_pawn_move[color][i] = BITBOARD_RANK_FILE(rank+offset, file);
            if(rank == base)
                bitboard_pawn_move2[color][i] = BITBOARD_RANK_FILE(rank+2*offset, file);
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
}

void bitboard_print_debug(bitboard_t bitboard)
{
    int rank, file;
    for(rank = 7; rank >= 0; rank--) {
        for(file = 0; file < 8; file++) {
            fprintf(stdout, "%d ", (bitboard & BITBOARD_RANK_FILE(rank, file)) ? 1 : 0);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");
}

