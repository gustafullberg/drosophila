#include "state.h"
#include "move.h"
#include "bitboard.h"
#include "eval.h"
#include <stdio.h>
#include <string.h>

void state_reset(chess_state_t *s)
{
    s->bitboard[WHITE_PIECES+PAWN]      = 0x000000000000FF00;
    s->bitboard[WHITE_PIECES+KNIGHT]    = 0x0000000000000042;
    s->bitboard[WHITE_PIECES+BISHOP]    = 0x0000000000000024;
    s->bitboard[WHITE_PIECES+ROOK]      = 0x0000000000000081;
    s->bitboard[WHITE_PIECES+QUEEN]     = 0x0000000000000008;
    s->bitboard[WHITE_PIECES+KING]      = 0x0000000000000010;
    s->bitboard[WHITE_PIECES+ALL]       = 0x000000000000FFFF;
    
    s->bitboard[BLACK_PIECES+PAWN]      = 0x00FF000000000000;
    s->bitboard[BLACK_PIECES+KNIGHT]    = 0x4200000000000000;
    s->bitboard[BLACK_PIECES+BISHOP]    = 0x2400000000000000;
    s->bitboard[BLACK_PIECES+ROOK]      = 0x8100000000000000;
    s->bitboard[BLACK_PIECES+QUEEN]     = 0x0800000000000000;
    s->bitboard[BLACK_PIECES+KING]      = 0x1000000000000000;
    s->bitboard[BLACK_PIECES+ALL]       = 0xFFFF000000000000;
    
    s->bitboard[OCCUPIED]               = 0xFFFF00000000FFFF;
    
    s->flags[WHITE] = STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK | STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
    s->flags[BLACK] = STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK | STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
    
    s->player = WHITE;
}

static int state_add_moves_to_stack(chess_state_t *s, bitboard_t bitboard_to, int pos_from, int type, int captured_type, int special, move_t *stack)
{
    move_t move;
	int pos_to;
	int num_moves = 0;
	
	while(bitboard_to) {
		pos_to = bitboard_find_bit(bitboard_to);
		move = (pos_from) | (pos_to << 6) | (type << 12) | (captured_type << 15) | (special << 18);
		stack[num_moves] = move;
		
		bitboard_to ^= BITBOARD_POSITION(pos_to);
		num_moves++;
	}
	
	return num_moves;
}

int state_generate_moves(chess_state_t *s, move_t *stack)
{
	int type, opponent_type, pos_from;
	bitboard_t pieces, possible_moves, possible_captures;
	bitboard_t pawn_push2, pawn_promotion, pawn_capture_promotion;
	int num_moves = 0;
	char opponent = 1 - s->player;
	int own_index = NUM_TYPES*s->player;
	int opponent_index = NUM_TYPES*opponent;
    
	type = PAWN;
	pieces = s->bitboard[own_index + type];
	while(pieces) { /* Loop through all pieces of the type */
		pos_from = bitboard_find_bit(pieces);
		
		move_pawn(s->player, pos_from, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &possible_moves, &pawn_push2, &possible_captures, &pawn_promotion, &pawn_capture_promotion);
		
		num_moves += state_add_moves_to_stack(s, possible_moves, pos_from, type, 0, MOVE_QUIET, stack + num_moves);
		num_moves += state_add_moves_to_stack(s, pawn_push2, pos_from, type, 0, MOVE_DOUBLE_PAWN_PUSH, stack + num_moves);

		for(opponent_type = 0; opponent_type < NUM_TYPES - 1; opponent_type++) {
			num_moves += state_add_moves_to_stack(s, possible_captures & s->bitboard[opponent_index + opponent_type], pos_from, type, opponent_type, MOVE_CAPTURE, stack + num_moves);
		}
		
		num_moves += state_add_moves_to_stack(s, pawn_promotion, pos_from, type, 0, MOVE_KNIGHT_PROMOTION, stack + num_moves);
		num_moves += state_add_moves_to_stack(s, pawn_promotion, pos_from, type, 0, MOVE_BISHOP_PROMOTION, stack + num_moves);
		num_moves += state_add_moves_to_stack(s, pawn_promotion, pos_from, type, 0, MOVE_ROOK_PROMOTION, stack + num_moves);
		num_moves += state_add_moves_to_stack(s, pawn_promotion, pos_from, type, 0, MOVE_QUEEN_PROMOTION, stack + num_moves);
		
		for(opponent_type = 0; opponent_type < NUM_TYPES - 1; opponent_type++) {
			num_moves += state_add_moves_to_stack(s, pawn_capture_promotion & s->bitboard[opponent_index + opponent_type], pos_from, type, opponent_type, MOVE_KNIGHT_PROMOTION_CAPTURE, stack + num_moves);
			num_moves += state_add_moves_to_stack(s, pawn_capture_promotion & s->bitboard[opponent_index + opponent_type], pos_from, type, opponent_type, MOVE_BISHOP_PROMOTION_CAPTURE, stack + num_moves);
			num_moves += state_add_moves_to_stack(s, pawn_capture_promotion & s->bitboard[opponent_index + opponent_type], pos_from, type, opponent_type, MOVE_ROOK_PROMOTION_CAPTURE, stack + num_moves);
			num_moves += state_add_moves_to_stack(s, pawn_capture_promotion & s->bitboard[opponent_index + opponent_type], pos_from, type, opponent_type, MOVE_QUEEN_PROMOTION_CAPTURE, stack + num_moves);
		}
		
		pieces ^= BITBOARD_POSITION(pos_from);
	}
	
	for(type = KNIGHT; type < NUM_TYPES - 1; type++) { /* Loop through all types of pieces */
		pieces = s->bitboard[own_index + type];

		while(pieces) { /* Loop through all pieces of the type */
			/* Get one position from the bitboard */
			pos_from = bitboard_find_bit(pieces);

			/* Get all possible moves for this piece */
			move_piece(s->player, type, pos_from, s->bitboard[own_index + ALL], s->bitboard[opponent_index + ALL], &possible_moves, &possible_captures);
			
			/* Extract possible captures */
			for(opponent_type = 0; opponent_type < NUM_TYPES - 1; opponent_type++) {
				num_moves += state_add_moves_to_stack(s, possible_captures & s->bitboard[opponent_index + opponent_type], pos_from, type, opponent_type, MOVE_CAPTURE, stack + num_moves);
			}

			num_moves += state_add_moves_to_stack(s, possible_moves, pos_from, type, 0, MOVE_QUIET, stack + num_moves);
            
			/* Clear position from bitboard */
			pieces ^= BITBOARD_POSITION(pos_from);
		}
	}
    
    /* En passant */
    if(s->flags[(int)s->player] & STATE_FLAGS_EN_PASSANT_POSSIBLE_MASK) {
        int file;
        bitboard_t attack_file;
        
        /* The file of the possible en passant capture */
        file = (s->flags[(int)s->player] & STATE_FLAGS_EN_PASSANT_FILE_MASK) >> STATE_FLAGS_EN_PASSANT_FILE_SHIFT;
        attack_file = BITBOARD_FILE << file;
        
        /* Find pawns that can make the capture */
        pieces = bitboard_ep_capturers[(int)s->player][file] & s->bitboard[own_index+PAWN];
        
        /* Loop through the found pawns */
        while(pieces) {
            /* Get one position from the bitboard */
			pos_from = bitboard_find_bit(pieces);
            
            possible_captures = bitboard_pawn_capture[(int)s->player][pos_from] & attack_file;
            
            num_moves += state_add_moves_to_stack(s, possible_captures, pos_from, PAWN, PAWN, MOVE_EP_CAPTURE, stack + num_moves);
            
            /* Clear position from bitboard */
			pieces ^= BITBOARD_POSITION(pos_from);
        }
    }

    /* King-side Castling */
    if(s->flags[(int)s->player] & STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK) {
        int king_pos = bitboard_find_bit(s->bitboard[own_index + KING]);
        if((bitboard_king_castle_empty[(int)s->player] & (s->bitboard[WHITE_PIECES + ALL] | s->bitboard[BLACK_PIECES + ALL])) == 0) {
            if(EVAL_position_is_attacked(s, s->player, king_pos+0) == 0 && 
               EVAL_position_is_attacked(s, s->player, king_pos+1) == 0 &&
               EVAL_position_is_attacked(s, s->player, king_pos+2) == 0)
            {
                num_moves += state_add_moves_to_stack(s, s->bitboard[own_index + KING] << 2, king_pos, KING, 0, MOVE_KING_CASTLE, stack + num_moves);
            }
        }
    }
    
    /* Queen-side Castling */
    if(s->flags[(int)s->player] & STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK) {
        int king_pos = bitboard_find_bit(s->bitboard[own_index + KING]);
        if((bitboard_queen_castle_empty[(int)s->player] & (s->bitboard[WHITE_PIECES + ALL] | s->bitboard[BLACK_PIECES + ALL])) == 0) {
            if(EVAL_position_is_attacked(s, s->player, king_pos-0) == 0 && 
               EVAL_position_is_attacked(s, s->player, king_pos-1) == 0 &&
               EVAL_position_is_attacked(s, s->player, king_pos-2) == 0)
            {
                num_moves += state_add_moves_to_stack(s, s->bitboard[own_index + KING] >> 2, king_pos, KING, 0, MOVE_QUEEN_CASTLE, stack + num_moves);
            }
        }
    }

	return num_moves;
}

void state_clone(chess_state_t *s_dst, const chess_state_t *s_src)
{
	memcpy(s_dst, s_src, sizeof(chess_state_t));
}

int state_apply_move(chess_state_t *s, const move_t move)
{
    int own_index, opponent_index;
    
	int pos_from        = (move & 0x3F);
	int pos_to          = ((move >> 6) & 0x3F);
	int type            = ((move >> 12) & 0x7);
    int opponent_type   = ((move >> 15) & 0x7);
	int special         = ((move >> 18) & 0xF);

    own_index = s->player * NUM_TYPES;
    opponent_index = NUM_TYPES - own_index;
    
	/* Remove bitboard of this type from the "ALL bitboard" */
	s->bitboard[own_index + ALL] ^= s->bitboard[own_index + type];

	/* Clear "from", set "to" */
	BITBOARD_CLEAR(s->bitboard[own_index + type], pos_from);
	BITBOARD_SET(s->bitboard[own_index + type], pos_to);

	/* Update the "ALL bitboard" */
	s->bitboard[own_index + ALL] ^= s->bitboard[own_index + type];
    
    /* Remove captured piece from the other side */
    if(special & MOVE_CAPTURE) {
        /* Normal capture */
        BITBOARD_CLEAR(s->bitboard[opponent_index + opponent_type], pos_to);
        BITBOARD_CLEAR(s->bitboard[opponent_index + ALL], pos_to);
        
        /* En passant capture */
        if(special == MOVE_EP_CAPTURE) {
            s->bitboard[opponent_index + PAWN] ^= bitboard_ep_capture[pos_to];
        }
    }
    
    /* Pushing pawn 2 squares opens for en passant */
    if(special == MOVE_DOUBLE_PAWN_PUSH) {
        s->flags[s->player^1] |= STATE_FLAGS_EN_PASSANT_POSSIBLE_MASK;
        s->flags[s->player^1] &= ~STATE_FLAGS_EN_PASSANT_FILE_MASK;
        s->flags[s->player^1] |= BITBOARD_GET_FILE(pos_from) << STATE_FLAGS_EN_PASSANT_FILE_SHIFT;
    }
    
    /* Pawn promotion */
    if(MOVE_IS_PROMOTION(move)) {
        int promotion_type = MOVE_PROMOTION_TYPE(move);
        BITBOARD_CLEAR(s->bitboard[own_index + type], pos_to);
        BITBOARD_SET(s->bitboard[own_index + promotion_type], pos_to);
    }
    
    /* Castling */
    if(special == MOVE_KING_CASTLE) {
        /* Move rook */
        s->bitboard[own_index + ALL] ^= s->bitboard[own_index + ROOK];
        s->bitboard[own_index + ROOK] ^= s->bitboard[own_index + KING] << 1;
        s->bitboard[own_index + ROOK] ^= s->bitboard[own_index + KING] >> 1;
        s->bitboard[own_index + ALL] ^= s->bitboard[own_index + ROOK];
    }
    if(special == MOVE_QUEEN_CASTLE) {
        /* Move rook */
        s->bitboard[own_index + ALL] ^= s->bitboard[own_index + ROOK];
        s->bitboard[own_index + ROOK] ^= s->bitboard[own_index + KING] >> 2;
        s->bitboard[own_index + ROOK] ^= s->bitboard[own_index + KING] << 1;
        s->bitboard[own_index + ALL] ^= s->bitboard[own_index + ROOK];
    }
    
    if(type == KING) {
        /* Disable castling */
        s->flags[(int)s->player] &= ~(STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK | STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK);
    }
    
    if(type == ROOK) {
        /* The rank number of the rook */
        int rank_number = BITBOARD_GET_RANK(pos_from);
        
        /* A bitboard representing the rank of the rook */
        bitboard_t rank = BITBOARD_RANK << (rank_number << 3);
        
        /* Check if this is the rank of the king */
        if(rank & s->bitboard[own_index + KING]) {
            int file = BITBOARD_GET_FILE(pos_from);
            if(file == 0) {
                /* Disable queen side castling */
                s->flags[(int)s->player] &= ~STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK;
            } else if(file == 7) {
                /* Disable king side castling */
                s->flags[(int)s->player] &= ~STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
            }
        }
    }

	/* Occupied by piece of any color */
	s->bitboard[OCCUPIED] = s->bitboard[WHITE_PIECES+ALL] | s->bitboard[BLACK_PIECES+ALL];

    /* This side can't play en passant next move (unless enabled by opponent's next move) */
    s->flags[(int)s->player] &= ~STATE_FLAGS_EN_PASSANT_POSSIBLE_MASK;
        
    /* Switch side to play */
    s->player = 1 - s->player;

	return 0;
}
