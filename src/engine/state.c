#include <stdlib.h>
#include <stdio.h>
#include "state.h"
#include "movegen.h"
#include "eval.h"
#include <stdio.h>

void STATE_reset(chess_state_t *s)
{
    int color;
    int piece;
    for(color = WHITE; color <= BLACK; color++) {
        s->bitboard[color*NUM_TYPES + ALL] = 0;
        for(piece = PAWN; piece <= KING; piece++) {
            s->bitboard[color*NUM_TYPES + piece] = bitboard_start_position[color][piece];
            s->bitboard[color*NUM_TYPES + ALL] |= bitboard_start_position[color][piece];
        }
    }
    s->bitboard[OCCUPIED] = s->bitboard[WHITE_PIECES + ALL] | s->bitboard[BLACK_PIECES + ALL];
    
    s->castling[WHITE] = STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK | STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
    s->castling[BLACK] = STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK | STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
    s->ep_file = STATE_EN_PASSANT_NONE;
    
    s->player = WHITE;
    
    STATE_compute_hash(s);
    
    s->last_move = 0;
    s->halfmove_clock = 0;
}

static void STATE_add_move_to_list(int pos_to, int pos_from, int type, int captured_type, int special, move_t *moves)
{
    *moves = (pos_from) | (pos_to << 6) | (type << 12) | (captured_type << 15) | (special << 18);
}

static int STATE_add_move_to_list_promotion(int pos_to, int pos_from, move_t *moves)
{
    move_t move = (pos_from) | (pos_to << 6) | (PAWN << 12);
    moves[0] = move | (MOVE_QUEEN_PROMOTION << 18);
    moves[1] = move | (MOVE_ROOK_PROMOTION << 18);
    moves[2] = move | (MOVE_BISHOP_PROMOTION << 18);
    moves[3] = move | (MOVE_KNIGHT_PROMOTION << 18);
    return 4;
}

static int STATE_add_move_to_list_promotion_capture(int pos_to, int pos_from, int captured_type, move_t *moves)
{
    move_t move = (pos_from) | (pos_to << 6) | (PAWN << 12) | (captured_type << 15);
    moves[0] = move | (MOVE_QUEEN_PROMOTION_CAPTURE << 18);
    moves[1] = move | (MOVE_ROOK_PROMOTION_CAPTURE << 18);
    moves[2] = move | (MOVE_BISHOP_PROMOTION_CAPTURE << 18);
    moves[3] = move | (MOVE_KNIGHT_PROMOTION_CAPTURE << 18);
    return 4;
}

static int STATE_add_moves_to_list(bitboard_t bitboard_to, int pos_from, int type, int captured_type, int special, move_t *moves)
{
    move_t move;
    int pos_to;
    int num_moves = 0;
    
    move = (pos_from) | (type << 12) | (captured_type << 15) | (special << 18);
    while(bitboard_to) {
        pos_to = BITBOARD_find_bit(bitboard_to);
        moves[num_moves] = move | (pos_to << 6);
        
        bitboard_to ^= BITBOARD_POSITION(pos_to);
        num_moves++;
    }
    
    return num_moves;
}

static int STATE_add_pawn_captures_promotions(const chess_state_t *s, bitboard_t pawn_captures_from_left, bitboard_t pawn_captures_from_right, bitboard_t pawn_promotion, bitboard_t pawn_promotion_capture_from_left, bitboard_t pawn_promotion_capture_from_right, move_t *moves)
{
    const int player = s->player;
    const int opponent = player ^ 1;
    const int opponent_index = NUM_TYPES*opponent;
    int num_moves = 0;
    int attack_from_left, attack_from_right, step;

    if(player == WHITE) {
        attack_from_left = -9;
        attack_from_right = -7;
        step = -8;
    } else {
        attack_from_left = 7;
        attack_from_right = 9;
        step = 8;
    }

    /* Captures */
    bitboard_t possible_captures = pawn_captures_from_left | pawn_captures_from_right;
    for(int opponent_type = PAWN; possible_captures; opponent_type++) {
        bitboard_t captures = possible_captures & s->bitboard[opponent_index + opponent_type];
        possible_captures ^= captures;
        bitboard_t captures_left = pawn_captures_from_left & captures;
        while(captures_left) {
            int pos_to = BITBOARD_find_bit(captures_left);
            bitboard_t pos_to_bb = BITBOARD_POSITION(pos_to);
            STATE_add_move_to_list(pos_to, pos_to + attack_from_left, PAWN, opponent_type, MOVE_CAPTURE, moves + num_moves++);
            captures_left ^= pos_to_bb;
        }
        bitboard_t captures_right = pawn_captures_from_right & captures;
        while(captures_right) {
            int pos_to = BITBOARD_find_bit(captures_right);
            bitboard_t pos_to_bb = BITBOARD_POSITION(pos_to);
            STATE_add_move_to_list(pos_to, pos_to + attack_from_right, PAWN, opponent_type, MOVE_CAPTURE, moves + num_moves++);
            captures_right ^= pos_to_bb;
        }
    }
    /* Promotion with capture */
    bitboard_t pawn_capture_promotion = pawn_promotion_capture_from_left | pawn_promotion_capture_from_right;
    for(int opponent_type = PAWN; pawn_capture_promotion; opponent_type++) {
        bitboard_t captures = pawn_capture_promotion & s->bitboard[opponent_index + opponent_type];
        pawn_capture_promotion ^= captures;
        bitboard_t captures_left = pawn_promotion_capture_from_left & captures;
        while(captures_left) {
            int pos_to = BITBOARD_find_bit(captures_left);
            bitboard_t pos_to_bb = BITBOARD_POSITION(pos_to);
            num_moves += STATE_add_move_to_list_promotion_capture(pos_to, pos_to + attack_from_left, opponent_type, moves + num_moves);
            captures_left ^= pos_to_bb;
        }
        bitboard_t captures_right = pawn_promotion_capture_from_right & captures;
        while(captures_right) {
            int pos_to = BITBOARD_find_bit(captures_right);
            bitboard_t pos_to_bb = BITBOARD_POSITION(pos_to);
            num_moves += STATE_add_move_to_list_promotion_capture(pos_to, pos_to + attack_from_right, opponent_type, moves + num_moves);
            captures_right ^= pos_to_bb;
        }
    }
    /* Promotion */
    while(pawn_promotion) {
        int pos_to = BITBOARD_find_bit(pawn_promotion);
        int pos_from = pos_to + step;
        num_moves += STATE_add_move_to_list_promotion(pos_to, pos_from, moves + num_moves);
        pawn_promotion ^= BITBOARD_POSITION(pos_to);
    }
    return num_moves;
}

static int STATE_add_pawn_quiet(const chess_state_t *s, bitboard_t possible_moves, bitboard_t pawn_push2, move_t *moves)
{
    int num_moves = 0;
    int dir = (s->player == WHITE) ? -1 : 1;

    /* Pawn push */
    while(possible_moves) {
        int pos_to = BITBOARD_find_bit(possible_moves);
        STATE_add_move_to_list(pos_to, pos_to + 8 * dir, PAWN, 0, MOVE_QUIET, moves + num_moves++);
        possible_moves ^= BITBOARD_POSITION(pos_to);
    }
    /* Double push */
    while(pawn_push2) {
        int pos_to = BITBOARD_find_bit(pawn_push2);
        STATE_add_move_to_list(pos_to, pos_to + 16 * dir, PAWN, 0, MOVE_DOUBLE_PAWN_PUSH, moves + num_moves++);
        pawn_push2 ^= BITBOARD_POSITION(pos_to);
    }
    return num_moves;
}

int STATE_generate_moves(const chess_state_t *s, move_t *moves)
{
    int type, opponent_type, pos_from;
    bitboard_t pieces, possible_moves, possible_captures;
    bitboard_t pawn_push2, pawn_captures_from_left, pawn_captures_from_right;
    bitboard_t pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right;
    int num_moves = 0;
    const int player = s->player;
    const int opponent = player ^ 1;
    const int player_index = NUM_TYPES*player;
    const int opponent_index = NUM_TYPES*opponent;
    const bitboard_t player_pieces = s->bitboard[player_index + ALL];
    const bitboard_t opponent_pieces = s->bitboard[opponent_index + ALL];

    /* Pawns */
    type = PAWN;
    pieces = s->bitboard[player_index + type];
    MOVEGEN_all_pawns(player, pieces, player_pieces, opponent_pieces, &possible_moves, &pawn_push2, &pawn_captures_from_left, &pawn_captures_from_right, &pawn_promotion, &pawn_promotion_captures_from_left, &pawn_promotion_captures_from_right);
    num_moves += STATE_add_pawn_captures_promotions(s, pawn_captures_from_left, pawn_captures_from_right, pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right, &moves[num_moves]);
    num_moves += STATE_add_pawn_quiet(s, possible_moves, pawn_push2, &moves[num_moves]);

    for(type = KNIGHT; type < NUM_TYPES - 1; type++) { /* Loop through all types of pieces */
        pieces = s->bitboard[player_index + type];

        while(pieces) { /* Loop through all pieces of the type */
            /* Get one position from the bitboard */
            pos_from = BITBOARD_find_bit(pieces);

            /* Get all possible moves for this piece */
            MOVEGEN_piece(type, pos_from, player_pieces, opponent_pieces, &possible_moves, &possible_captures);
            
            /* Extract possible captures */
            for(opponent_type = 0; possible_captures; opponent_type++) {
                bitboard_t captures = possible_captures & s->bitboard[opponent_index + opponent_type];
                num_moves += STATE_add_moves_to_list(captures, pos_from, type, opponent_type, MOVE_CAPTURE, moves + num_moves);
                possible_captures ^= captures;
            }
        
            num_moves += STATE_add_moves_to_list(possible_moves, pos_from, type, 0, MOVE_QUIET, moves + num_moves);
            
            /* Clear position from bitboard */
            pieces ^= BITBOARD_POSITION(pos_from);
        }
    }
    
    /* En passant */
    if(s->ep_file != STATE_EN_PASSANT_NONE) {
        int file;
        bitboard_t attack_file;
        
        /* The file of the possible en passant capture */
        file = s->ep_file;
        attack_file = BITBOARD_FILE << file;
        
        /* Find pawns that can make the capture */
        pieces = bitboard_ep_capturers[player][file] & s->bitboard[player_index+PAWN];
        
        /* Loop through the found pawns */
        while(pieces) {
            /* Get one position from the bitboard */
            pos_from = BITBOARD_find_bit(pieces);
            
            possible_captures = bitboard_pawn_capture[player][pos_from] & attack_file;
            
            num_moves += STATE_add_moves_to_list(possible_captures, pos_from, PAWN, PAWN, MOVE_EP_CAPTURE, moves + num_moves);
            
            /* Clear position from bitboard */
            pieces ^= BITBOARD_POSITION(pos_from);
        }
    }

    /* King-side Castling */
    if(s->castling[player] & STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK) {
        if((bitboard_king_castle_empty[player] & s->bitboard[OCCUPIED]) == 0) {
            int king_pos = BITBOARD_find_bit(s->bitboard[player_index + KING]);
            if(EVAL_position_is_attacked(s, player, king_pos+0) == 0 && 
               EVAL_position_is_attacked(s, player, king_pos+1) == 0 &&
               EVAL_position_is_attacked(s, player, king_pos+2) == 0)
            {
                num_moves += STATE_add_moves_to_list(s->bitboard[player_index + KING] << 2, king_pos, KING, 0, MOVE_KING_CASTLE, moves + num_moves);
            }
        }
    }
    
    /* Queen-side Castling */
    if(s->castling[player] & STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK) {
        if((bitboard_queen_castle_empty[player] & s->bitboard[OCCUPIED]) == 0) {
            int king_pos = BITBOARD_find_bit(s->bitboard[player_index + KING]);
            if(EVAL_position_is_attacked(s, player, king_pos-0) == 0 && 
               EVAL_position_is_attacked(s, player, king_pos-1) == 0 &&
               EVAL_position_is_attacked(s, player, king_pos-2) == 0)
            {
                num_moves += STATE_add_moves_to_list(s->bitboard[player_index + KING] >> 2, king_pos, KING, 0, MOVE_QUEEN_CASTLE, moves + num_moves);
            }
        }
    }

    return num_moves;
}

static inline bitboard_t STATE_pin_mask(bitboard_t piece, int king_pos, bitboard_t pinners)
{
    while(pinners) { /* Loop through all possible pinners */
        /* Get one position from the bitboard */
        int pinner_pos = BITBOARD_find_bit(pinners);
        bitboard_t pinner_bb = BITBOARD_POSITION(pinner_pos);

        bitboard_t between = bitboard_between[pinner_pos][king_pos];
        if(between & piece) return between | pinner_bb;

        /* Clear position from bitboard */
        pinners ^= pinner_bb;
    }

    /* Never reached */
    return 0;
}

int STATE_generate_legal_moves(const chess_state_t *s, bitboard_t checkers, bitboard_t block_checker, bitboard_t pinners, bitboard_t pinned, bitboard_t king_threat, move_t *moves)
{
    int opponent_type, pos_from;
    bitboard_t pieces, possible_moves, possible_captures;
    bitboard_t pawn_push2, pawn_captures_from_left, pawn_captures_from_right;
    bitboard_t pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right;
    int num_moves = 0;
    const int player = s->player;
    const int opponent = player ^ 1;
    const int player_index = NUM_TYPES*player;
    const int opponent_index = NUM_TYPES*opponent;
    const bitboard_t player_pieces = s->bitboard[player_index + ALL];
    const bitboard_t opponent_pieces = s->bitboard[opponent_index + ALL];
    const int num_checkers = BITBOARD_count_bits(checkers);
    int king_pos = BITBOARD_find_bit(s->bitboard[player_index + KING]);

    if(num_checkers < 2) {
        bitboard_t move_mask = (bitboard_t)0xFFFFFFFFFFFFFFFF;
        bitboard_t capture_mask = (bitboard_t)0xFFFFFFFFFFFFFFFF;

        if(num_checkers) {
            move_mask = block_checker;
            capture_mask = checkers;
        }

        /* Pawns */
        {
            /* Non-pinned pawns */
            pieces = s->bitboard[player_index + PAWN] & ~pinned;
            MOVEGEN_all_pawns(player, pieces, player_pieces, opponent_pieces, &possible_moves, &pawn_push2, &pawn_captures_from_left, &pawn_captures_from_right, &pawn_promotion, &pawn_promotion_captures_from_left, &pawn_promotion_captures_from_right);
            possible_moves &= move_mask;
            pawn_push2 &= move_mask;
            pawn_promotion &= move_mask;
            pawn_captures_from_left &= capture_mask;
            pawn_captures_from_right &= capture_mask;
            pawn_promotion_captures_from_left &= capture_mask;
            pawn_promotion_captures_from_right &= capture_mask;
            num_moves += STATE_add_pawn_captures_promotions(s, pawn_captures_from_left, pawn_captures_from_right, pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right, &moves[num_moves]);
            num_moves += STATE_add_pawn_quiet(s, possible_moves, pawn_push2, &moves[num_moves]);

            /* Pinned pawns */
            if(!num_checkers) {
                pieces = s->bitboard[player_index + PAWN] & pinned;
                while(pieces) {
                    pos_from = BITBOARD_find_bit(pieces);
                    bitboard_t piece_bb = BITBOARD_POSITION(pos_from);
                    bitboard_t pin_mask = STATE_pin_mask(piece_bb, king_pos, pinners);

                    MOVEGEN_all_pawns(player, pieces, player_pieces, opponent_pieces, &possible_moves, &pawn_push2, &pawn_captures_from_left, &pawn_captures_from_right, &pawn_promotion, &pawn_promotion_captures_from_left, &pawn_promotion_captures_from_right);
                    possible_moves &= (move_mask & pin_mask);
                    pawn_push2 &= (move_mask & pin_mask);
                    pawn_promotion &= (move_mask & pin_mask);
                    pawn_captures_from_left &= (capture_mask & pin_mask);
                    pawn_captures_from_right &= (capture_mask & pin_mask);
                    pawn_promotion_captures_from_left &= (capture_mask & pin_mask);
                    pawn_promotion_captures_from_right &= (capture_mask & pin_mask);
                    num_moves += STATE_add_pawn_captures_promotions(s, pawn_captures_from_left, pawn_captures_from_right, pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right, &moves[num_moves]);
                    num_moves += STATE_add_pawn_quiet(s, possible_moves, pawn_push2, &moves[num_moves]);

                    pieces ^= piece_bb;
                }
            }

            /* En passant */
            if(s->ep_file != STATE_EN_PASSANT_NONE) {
                int file;
                bitboard_t attack_file;

                /* The file of the possible en passant capture */
                file = s->ep_file;
                attack_file = BITBOARD_FILE << file;

                /* Find pawns that can make the capture */
                pieces = bitboard_ep_capturers[player][file] & s->bitboard[player_index+PAWN];

                /* Loop through the found pawns */
                while(pieces) {
                    /* Get one position from the bitboard */
                    pos_from = BITBOARD_find_bit(pieces);
                    bitboard_t piece_bb = BITBOARD_POSITION(pos_from);

                    bitboard_t pin_mask = (bitboard_t)0xFFFFFFFFFFFFFFFF;
                    if(piece_bb & pinned) {
                        if(num_checkers) {
                            /* Pinned piece can't be moved when checked */
                            pieces ^= piece_bb;
                            continue;
                        }
                        pin_mask = STATE_pin_mask(piece_bb, king_pos, pinners);
                    }

                    possible_captures = bitboard_pawn_capture[player][pos_from] & attack_file;

                    int valid_move = (capture_mask & bitboard_ep_capture[BITBOARD_find_bit(possible_captures)] & pin_mask) || (possible_captures & move_mask & pin_mask);
                    if(valid_move && (bitboard_rank[pos_from] & s->bitboard[player_index+KING])) {
                        /* Super gotcha */
                        bitboard_t opponent_slider = s->bitboard[opponent_index+ROOK] | s->bitboard[opponent_index+QUEEN];
                        while(opponent_slider) {
                            int slider_pos = BITBOARD_find_bit(opponent_slider);
                            bitboard_t between = bitboard_between[king_pos][slider_pos];
                            if((BITBOARD_count_bits(between & s->bitboard[OCCUPIED]) == 2) && (between & piece_bb)) {
                                valid_move = 0; 
                            }
                            opponent_slider ^= BITBOARD_POSITION(slider_pos);
                        }
                        // Check if own king and opponent rook/queen is on the same rank. Check if two pieces are between those and one is one of the pawns.

                    }
                    if(valid_move) {
                        num_moves += STATE_add_moves_to_list(possible_captures, pos_from, PAWN, PAWN, MOVE_EP_CAPTURE, moves + num_moves);
                    }

                    /* Clear position from bitboard */
                    pieces ^= piece_bb;
                }
            }
        }

        /* Knights, bishops, rooks and queens */
        for(int type = KNIGHT; type <= QUEEN; type++) {
            pieces = s->bitboard[player_index + type];

            while(pieces) { /* Loop through all pieces of the type */
                /* Get one position from the bitboard */
                pos_from = BITBOARD_find_bit(pieces);
                bitboard_t piece_bb = BITBOARD_POSITION(pos_from);

                /* Take pinning into consideration */
                bitboard_t pin_mask = (bitboard_t)0xFFFFFFFFFFFFFFFF;
                if(piece_bb & pinned) {
                    if(num_checkers) {
                        /* Pinned piece can't be moved when checked */
                        pieces ^= piece_bb;
                        continue;
                    }
                    pin_mask = STATE_pin_mask(piece_bb, king_pos, pinners);
                }

                /* Get all possible moves for this piece */
                MOVEGEN_piece(type, pos_from, player_pieces, opponent_pieces, &possible_moves, &possible_captures);

                /* Limit to moves and captures that don't put own king in check */
                possible_moves &= (move_mask & pin_mask);
                possible_captures &= (capture_mask & pin_mask);

                /* Extract possible captures */
                for(opponent_type = 0; possible_captures; opponent_type++) {
                    bitboard_t captures = possible_captures & s->bitboard[opponent_index + opponent_type];
                    num_moves += STATE_add_moves_to_list(captures, pos_from, type, opponent_type, MOVE_CAPTURE, moves + num_moves);
                    possible_captures ^= captures;
                }

                num_moves += STATE_add_moves_to_list(possible_moves, pos_from, type, 0, MOVE_QUIET, moves + num_moves);

                /* Clear position from bitboard */
                pieces ^= piece_bb;
            }
        }
    }
    
    /* King */
    {
        /* Get all possible moves for this piece */
        MOVEGEN_piece(KING, king_pos, player_pieces, opponent_pieces, &possible_moves, &possible_captures);

        /* Only allow safe squares */
        possible_moves &= ~king_threat;
        possible_captures &= ~king_threat;

        /* Extract possible captures */
        for(opponent_type = 0; possible_captures; opponent_type++) {
            bitboard_t captures = possible_captures & s->bitboard[opponent_index + opponent_type];
            num_moves += STATE_add_moves_to_list(captures, king_pos, KING, opponent_type, MOVE_CAPTURE, moves + num_moves);
            possible_captures ^= captures;
        }

        /* Quiet king moves */
        num_moves += STATE_add_moves_to_list(possible_moves, king_pos, KING, 0, MOVE_QUIET, moves + num_moves);

        /* Castling */
        if(!num_checkers) {
            /* King-side */
            if(s->castling[player] & STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK) {
                bitboard_t king_bb = s->bitboard[player_index + KING];
                if((bitboard_king_castle_empty[player] & s->bitboard[OCCUPIED]) == 0 &&
                        (((king_bb << 1) | (king_bb << 2)) & king_threat) == 0) {
                    num_moves += STATE_add_moves_to_list(king_bb << 2, king_pos, KING, 0, MOVE_KING_CASTLE, moves + num_moves);
                }
            }

            /* Queen-side */
            if(s->castling[player] & STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK) {
                bitboard_t king_bb = s->bitboard[player_index + KING];
                if((bitboard_queen_castle_empty[player] & s->bitboard[OCCUPIED]) == 0 &&
                        (((king_bb >> 1) | (king_bb >> 2)) & king_threat) == 0) {
                    num_moves += STATE_add_moves_to_list(king_bb >> 2, king_pos, KING, 0, MOVE_QUEEN_CASTLE, moves + num_moves);
                }
            }
        }
    }

    return num_moves;
}

int STATE_generate_moves_quiescence(const chess_state_t *s, move_t *moves)
{
    int type, opponent_type, pos_from;
    bitboard_t pieces, possible_moves, possible_captures;
    bitboard_t pawn_push2, pawn_captures_from_left, pawn_captures_from_right;
    bitboard_t pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right;
    int num_moves = 0;
    const int player = s->player;
    const int opponent = player ^ 1;
    const int player_index = NUM_TYPES*player;
    const int opponent_index = NUM_TYPES*opponent;
    const bitboard_t player_pieces = s->bitboard[player_index + ALL];
    const bitboard_t opponent_pieces = s->bitboard[opponent_index + ALL];
    
    /* Pawns */
    type = PAWN;
    pieces = s->bitboard[player_index + type];
    MOVEGEN_all_pawns(player, pieces, player_pieces, opponent_pieces, &possible_moves, &pawn_push2, &pawn_captures_from_left, &pawn_captures_from_right, &pawn_promotion, &pawn_promotion_captures_from_left, &pawn_promotion_captures_from_right);
    num_moves += STATE_add_pawn_captures_promotions(s, pawn_captures_from_left, pawn_captures_from_right, pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right, &moves[num_moves]);
    
    for(type = KNIGHT; type < NUM_TYPES - 1; type++) { /* Loop through all types of pieces */
        pieces = s->bitboard[player_index + type];

        while(pieces) { /* Loop through all pieces of the type */
            /* Get one position from the bitboard */
            pos_from = BITBOARD_find_bit(pieces);

            /* Get all possible moves for this piece */
            MOVEGEN_piece(type, pos_from, player_pieces, opponent_pieces, &possible_moves, &possible_captures);
            
            /* Extract possible captures */
            for(opponent_type = 0; possible_captures; opponent_type++) {
                bitboard_t captures = possible_captures & s->bitboard[opponent_index + opponent_type];
                num_moves += STATE_add_moves_to_list(captures, pos_from, type, opponent_type, MOVE_CAPTURE, moves + num_moves);
                possible_captures ^= captures;
            }
            
            /* Clear position from bitboard */
            pieces ^= BITBOARD_POSITION(pos_from);
        }
    }
    
    /* En passant */
    if(s->ep_file != STATE_EN_PASSANT_NONE) {
        int file;
        bitboard_t attack_file;
        
        /* The file of the possible en passant capture */
        file = s->ep_file;
        attack_file = BITBOARD_FILE << file;
        
        /* Find pawns that can make the capture */
        pieces = bitboard_ep_capturers[player][file] & s->bitboard[player_index+PAWN];
        
        /* Loop through the found pawns */
        while(pieces) {
            /* Get one position from the bitboard */
            pos_from = BITBOARD_find_bit(pieces);
            
            possible_captures = bitboard_pawn_capture[player][pos_from] & attack_file;
            
            num_moves += STATE_add_moves_to_list(possible_captures, pos_from, PAWN, PAWN, MOVE_EP_CAPTURE, moves + num_moves);
            
            /* Clear position from bitboard */
            pieces ^= BITBOARD_POSITION(pos_from);
        }
    }

    return num_moves;
}

int STATE_apply_move(chess_state_t *s, const move_t move)
{
    int player = s->player;
    int opponent = player ^ 1;

    /* Reset en passant state */
    s->hash ^= bitboard_zobrist_ep[(int)s->ep_file];
    s->ep_file = STATE_EN_PASSANT_NONE;

    if(move) {
        int pos_from        = MOVE_GET_POS_FROM(move);
        int pos_to          = MOVE_GET_POS_TO(move);
        int type            = MOVE_GET_TYPE(move);
        int special         = MOVE_GET_SPECIAL_FLAGS(move);

        int player_index = player * NUM_TYPES;
        int opponent_index = NUM_TYPES - player_index;

        /* Reset castling hash */
        s->hash ^= bitboard_zobrist_castling[player][(int)s->castling[player]];
        
        /* Increment half-move clock */
        s->halfmove_clock++;
        
        /* Remove bitboard of this type from the "ALL bitboard" */
        s->bitboard[player_index + ALL] ^= s->bitboard[player_index + type];

        /* Clear "from", set "to" */
        BITBOARD_CLEAR(s->bitboard[player_index + type], pos_from);
        BITBOARD_SET(s->bitboard[player_index + type], pos_to);

        /* Update the "ALL bitboard" */
        s->bitboard[player_index + ALL] ^= s->bitboard[player_index + type];
        
        /* Update hash with normal move */
        s->hash ^= bitboard_zobrist[player][type][pos_from];
        s->hash ^= bitboard_zobrist[player][type][pos_to];
        
        /* Remove captured piece from the other side */
        if(special & MOVE_CAPTURE) {
            int opponent_type = MOVE_GET_CAPTURE_TYPE(move);
            if(special == MOVE_EP_CAPTURE) {
                int pos_capture = BITBOARD_find_bit(bitboard_ep_capture[pos_to]);
                
                /* En passant capture */
                s->bitboard[opponent_index + PAWN] ^= bitboard_ep_capture[pos_to];
                s->bitboard[opponent_index + ALL] ^= bitboard_ep_capture[pos_to];
                
                /* Update hashes with EP capture */
                s->hash ^= bitboard_zobrist[opponent][opponent_type][pos_capture];

            } else {
                /* Normal capture */
                BITBOARD_CLEAR(s->bitboard[opponent_index + opponent_type], pos_to);
                BITBOARD_CLEAR(s->bitboard[opponent_index + ALL], pos_to);
            
                /* Rook capture disables castling */
                if((opponent_type == ROOK) && (BITBOARD_POSITION(pos_to) & bitboard_start_position[opponent][ROOK])) {
                    s->hash ^= bitboard_zobrist_castling[opponent][(int)s->castling[opponent]];
                    if(bitboard_file[pos_to] == bitboard_file[0]) {
                        s->castling[opponent] &= ~STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK;
                    } else {
                        s->castling[opponent] &= ~STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
                    }
                    s->hash ^= bitboard_zobrist_castling[opponent][(int)s->castling[opponent]];
                }
                
                /* Update hash with normal capture */
                s->hash ^= bitboard_zobrist[opponent][opponent_type][pos_to];
            }
            
            /* Reset half-move clock when a piece is captured */
            s->halfmove_clock = 0;
        }
        
        if(type == KING) {
            /* Disable castling */
            s->castling[player] &= ~(STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK | STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK);
            
            /* Castling */
            if(special == MOVE_KING_CASTLE) {
                /* Move rook */
                s->bitboard[player_index + ALL] ^= s->bitboard[player_index + ROOK];
                s->bitboard[player_index + ROOK] ^= s->bitboard[player_index + KING] << 1;
                s->bitboard[player_index + ROOK] ^= s->bitboard[player_index + KING] >> 1;
                s->bitboard[player_index + ALL] ^= s->bitboard[player_index + ROOK];
                
                /* Update hash with king-side castling */
                s->hash ^= bitboard_zobrist[player][ROOK][pos_to+1];
                s->hash ^= bitboard_zobrist[player][ROOK][pos_to-1];
            }
            if(special == MOVE_QUEEN_CASTLE) {
                /* Move rook */
                s->bitboard[player_index + ALL] ^= s->bitboard[player_index + ROOK];
                s->bitboard[player_index + ROOK] ^= s->bitboard[player_index + KING] >> 2;
                s->bitboard[player_index + ROOK] ^= s->bitboard[player_index + KING] << 1;
                s->bitboard[player_index + ALL] ^= s->bitboard[player_index + ROOK];
                
                /* Update hash with king-side castling */
                s->hash ^= bitboard_zobrist[player][ROOK][pos_to-2];
                s->hash ^= bitboard_zobrist[player][ROOK][pos_to+1];
            }
        }
        
        if(type == ROOK) {
            /* Disable castling for a moved rook */
            if(BITBOARD_POSITION(pos_from) & bitboard_start_position[player][ROOK]) {
                if(bitboard_file[pos_from] == bitboard_file[0]) {
                    s->castling[player] &= ~STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK;
                } else {
                    s->castling[player] &= ~STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
                }
            }
        }
        
        if(type == PAWN) {
            /* Reset half-move clock when a pawn is moved */
            s->halfmove_clock = 0;
                  
            /* Pushing pawn 2 squares opens for en passant */
            if(special == MOVE_DOUBLE_PAWN_PUSH) {
                int file = BITBOARD_GET_FILE(pos_from);
                if(bitboard_ep_capturers[opponent][file] & s->bitboard[opponent_index+PAWN]) {
                    s->ep_file = (unsigned char)file;
                    s->hash ^= bitboard_zobrist_ep[file];
                }
            }
            
            /* Pawn promotion */
            if(MOVE_IS_PROMOTION(move)) {
                int promotion_type = MOVE_PROMOTION_TYPE(move);
                BITBOARD_CLEAR(s->bitboard[player_index + type], pos_to);
                BITBOARD_SET(s->bitboard[player_index + promotion_type], pos_to);
                
                /* Update hash with promotion */
                s->hash ^= bitboard_zobrist[player][PAWN][pos_to];
                s->hash ^= bitboard_zobrist[player][promotion_type][pos_to];
            }
        }

        /* Occupied by piece of any color */
        s->bitboard[OCCUPIED] = s->bitboard[WHITE_PIECES+ALL] | s->bitboard[BLACK_PIECES+ALL];

        /* Apply castling rights to hash */
        s->hash ^= bitboard_zobrist_castling[player][(int)s->castling[player]];
    }    
    /* Switch side to play */
    s->player = (unsigned char)opponent;
    s->hash ^= bitboard_zobrist_color;
    
    /* Store last move */
    s->last_move = move;

    return 0;
}

void STATE_compute_hash(chess_state_t *s)
{
    int color;
    int type;
    int pos;
    bitboard_t pieces;
    
    s->hash = 0;
    
    for(color = 0; color < NUM_COLORS; color++) {
        for(type = 0; type < NUM_TYPES - 1; type++) {
            pieces = s->bitboard[color*NUM_TYPES + type];

            while(pieces) {
                pos = BITBOARD_find_bit(pieces);

                /* Update the hash for each piece on the board */
                s->hash ^= bitboard_zobrist[color][type][pos];
                
                pieces ^= BITBOARD_POSITION(pos);
            }
        }
    }

    /* En passant */
    s->hash ^= bitboard_zobrist_ep[(int)s->ep_file];

    /* Castling rights */
    s->hash ^= bitboard_zobrist_castling[WHITE][(int)s->castling[WHITE]];
    s->hash ^= bitboard_zobrist_castling[BLACK][(int)s->castling[BLACK]];

    if(s->player == WHITE) {
        s->hash ^= bitboard_zobrist_color;
    }
}

int STATE_risk_zugzwang(const chess_state_t *s)
{
    /* Risk of zugzwang if only king and pawns remain on the playing side */
    const int player_index = NUM_TYPES*(int)s->player;
    return s->bitboard[player_index+ALL] == (s->bitboard[player_index+PAWN] | s->bitboard[player_index+KING]);
}

bitboard_t STATE_opponent_threat_to_king(const chess_state_t *s)
{
    int type, pos_from;
    bitboard_t pieces, possible_moves, possible_captures;
    bitboard_t pawn_push2, pawn_captures_from_left, pawn_captures_from_right;
    bitboard_t pawn_promotion, pawn_promotion_captures_from_left, pawn_promotion_captures_from_right;
    const int player = s->player;
    const int opponent = player ^ 1;
    const int player_index = NUM_TYPES*player;
    const int opponent_index = NUM_TYPES*opponent;
    const bitboard_t player_pieces = s->bitboard[player_index + ALL];
    const bitboard_t opponent_pieces = s->bitboard[opponent_index + ALL];
    const bitboard_t occupied = s->bitboard[OCCUPIED] ^ s->bitboard[player_index+KING];
    bitboard_t threat = 0;

    /* Pawns */
    type = PAWN;
    pieces = s->bitboard[opponent_index + type];
    if(opponent == WHITE) {
	    threat |= (pieces & ~(BITBOARD_FILE << 7)) << 9;
	    threat |= (pieces & ~(BITBOARD_FILE << 0)) << 7;
    } else {
	    threat |= (pieces & ~(BITBOARD_FILE << 7)) >> 7;
	    threat |= (pieces & ~(BITBOARD_FILE << 0)) >> 9;
    }

    for(type = KNIGHT; type < NUM_TYPES - 1; type++) { /* Loop through all types of pieces */
        pieces = s->bitboard[opponent_index + type];

        while(pieces) { /* Loop through all pieces of the type */
            /* Get one position from the bitboard */
            pos_from = BITBOARD_find_bit(pieces);

            /* Get all possible moves for this piece */
            MOVEGEN_piece(type, pos_from, 0, occupied, &possible_moves, &possible_captures);

            threat |= possible_moves | possible_captures;

            /* Clear position from bitboard */
            pieces ^= BITBOARD_POSITION(pos_from);
        }
    }

    return threat;
}

int STATE_checkers_and_pinners(const chess_state_t *s, bitboard_t *checkers, bitboard_t *block_checker, bitboard_t *pinners, bitboard_t *pinned)
{
    const int player = s->player;
    const int opponent = player ^ 1;
    const bitboard_t * player_bb = &s->bitboard[player * NUM_TYPES];
    const bitboard_t * opponent_bb = &s->bitboard[opponent * NUM_TYPES];
    int king_pos = BITBOARD_find_bit(player_bb[KING]);
    *checkers = 0;
    *block_checker = 0;
    *pinners = 0;
    *pinned = 0;

    /* Attacking pawns */
    *checkers |= bitboard_pawn_capture[player][king_pos] & opponent_bb[PAWN];

    /* Attacking knights */
    *checkers |= bitboard_knight[king_pos] & opponent_bb[KNIGHT];

    /* Attacking sliders */
    bitboard_t attackers = (bitboard_bishop[king_pos] & (opponent_bb[BISHOP] | opponent_bb[QUEEN])) |
                           (bitboard_rook[king_pos] & (opponent_bb[ROOK] | opponent_bb[QUEEN]));
    while(attackers) {
        int attack_pos = BITBOARD_find_bit(attackers);
        bitboard_t attacker_bb = BITBOARD_POSITION(attack_pos);
        bitboard_t between = bitboard_between[attack_pos][king_pos];
        bitboard_t own_between = between & player_bb[ALL];
        bitboard_t opponent_between = between & opponent_bb[ALL];

        if(opponent_between == 0) {
            if(own_between == 0) {
                *checkers |= attacker_bb;
                *block_checker |= between;
            } else if(BITBOARD_count_bits(own_between) == 1) {
                *pinners |= attacker_bb;
                *pinned |= own_between;
            }
        }

        attackers ^= attacker_bb;
    }

    return *checkers != 0;
}

void STATE_move_print_debug(const move_t move)
{
    int pos_from = MOVE_GET_POS_FROM(move);
    int pos_to   = MOVE_GET_POS_TO(move);
    fprintf(stdout, "#%c%c -> %c%c\n", BITBOARD_GET_FILE(pos_from)+'A', BITBOARD_GET_RANK(pos_from)+'1', BITBOARD_GET_FILE(pos_to)+'A', BITBOARD_GET_RANK(pos_to)+'1');
}

void STATE_board_print_debug(const chess_state_t *s)
{
    int rank, file, color;
    for(rank = 7; rank >= 0; rank--) {
        fprintf(stdout, "#%c ", rank + '1');
        for(file = 0; file < 8; file++) {
            char c = '-';
            for(color = WHITE; color <= BLACK; color++) {
                if(s->bitboard[color*NUM_TYPES+ALL] & BITBOARD_RANK_FILE(rank, file)) {
                    if(s->bitboard[color*NUM_TYPES+PAWN] & BITBOARD_RANK_FILE(rank, file)) c = 'P';
                    if(s->bitboard[color*NUM_TYPES+KNIGHT] & BITBOARD_RANK_FILE(rank, file)) c = 'N';
                    if(s->bitboard[color*NUM_TYPES+BISHOP] & BITBOARD_RANK_FILE(rank, file)) c = 'B';
                    if(s->bitboard[color*NUM_TYPES+ROOK] & BITBOARD_RANK_FILE(rank, file)) c = 'R';
                    if(s->bitboard[color*NUM_TYPES+QUEEN] & BITBOARD_RANK_FILE(rank, file)) c = 'Q';
                    if(s->bitboard[color*NUM_TYPES+KING] & BITBOARD_RANK_FILE(rank, file)) c = 'K';
                    if(color == BLACK) c += 32;
                }
            }
            fprintf(stdout, "%c ", c);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "   A B C D E F G H\n");
}
