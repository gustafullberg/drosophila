#include "bitboard.h"
#include <stdlib.h>
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
bitboard_t bitboard_between[NUM_POSITIONS][NUM_POSITIONS];
bitboard_t bitboard_king[NUM_POSITIONS];
bitboard_t bitboard_knight[NUM_POSITIONS];
bitboard_t bitboard_pawn_move[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_pawn_move2[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_pawn_capture[NUM_COLORS][NUM_POSITIONS];
bitboard_t bitboard_ep_capture[NUM_POSITIONS];
bitboard_t bitboard_ep_capturers[NUM_COLORS][NUM_FILES];
bitboard_t bitboard_bishop[NUM_POSITIONS];
bitboard_t bitboard_rook[NUM_POSITIONS];
bitboard_t bitboard_king_castle_empty[NUM_COLORS];
bitboard_t bitboard_queen_castle_empty[NUM_COLORS];
bitboard_t bitboard_start_position[NUM_COLORS][NUM_TYPES-1];
bitboard_t bitboard_zobrist[NUM_COLORS][NUM_TYPES-1][NUM_POSITIONS];
bitboard_t bitboard_zobrist_color;
bitboard_t bitboard_zobrist_ep[NUM_FILES];
uint32_t   bitboard_zobrist_pawn[NUM_COLORS][NUM_POSITIONS];

static bitboard_t BITBOARD_random();
static uint32_t BITBOARD_random_uint32();

void BITBOARD_init()
{
    int i, j, tmp, rank, file, color, offset, base;
    
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
    
    /* ZOBRIST KEYS */
    for(color = 0; color < NUM_COLORS; color++) {
        int type;
        for(type = 0; type < NUM_TYPES - 1; type++) {
            for(i = 0; i < NUM_POSITIONS; i++) {
                bitboard_zobrist[color][type][i] = BITBOARD_random();
            }
        }
    }
    bitboard_zobrist_color = BITBOARD_random();

    for(i = 0; i < NUM_FILES; i++) {
        bitboard_zobrist_ep[i] = BITBOARD_random();
    }

    /* ZORBIST KEYS FOR PAWN HASH TABLE */
    for(color = 0; color < NUM_COLORS; color++) {
        for(i = 0; i < NUM_POSITIONS; i++) {
            bitboard_zobrist_pawn[color][i] = BITBOARD_random_uint32();
        }
    }
}

static bitboard_t BITBOARD_random()
{
    bitboard_t b = 0;
    int i;
    for(i = 0; i < 4; i++) {
        b <<= 16;
        b |= (bitboard_t)(rand() & 0xFFFF);
    }
    return b;
}

static uint32_t BITBOARD_random_uint32()
{
    uint32_t b;
    b = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
    return b;
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

