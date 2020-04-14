#include <string.h>
#include "san.h"
#include "search.h"

/* TODO: rewrite this dirty code */
move_t SAN_parse_move(const chess_state_t *state, const char *san)
{
    move_t moves[256];
    move_t candidate = 0;
    move_t num_candidates = 0;
    int num_moves;
    int i;
    int len;
    int is_capture = 0;
    int promotion_type = 0;
    int type = PAWN;
    int pos_to = 0;
    int rank = -1;
    int file = -1;
    
    len = (int)strlen(san);
    
    /* Generate all possible moves */
    num_moves = STATE_generate_moves_simple(state, moves);
    
    /* What type of piece is moving? */
    switch(san[0]) {
    case 'N':
        type = KNIGHT;
        break;
    case 'B':
        type = BISHOP;
        break;
    case 'R':
        type = ROOK;
        break;
    case 'Q':
        type = QUEEN;
        break;
    case 'K':
    case 'O':
        type = KING;
        break;
    default:
        type = PAWN;
        break;
    }
    
    if(type == KING) {
        /* Is this a kingside castling? */
        if(strncmp(san, "O-O-O", 5) == 0) {
            for(i = 0; i < num_moves; i++) {
                if(MOVE_GET_SPECIAL_FLAGS(moves[i]) == MOVE_QUEEN_CASTLE) {
                    return moves[i];
                }
            }
            /* Failure */
            return 0;
        }
        
        /* Is this a queenside castling? */
        if(strncmp(san, "O-O", 3) == 0) {
            for(i = 0; i < num_moves; i++) {
                if(MOVE_GET_SPECIAL_FLAGS(moves[i]) == MOVE_KING_CASTLE) {
                    return moves[i];
                }
            }
            /* Failure */
            return 0;
        }
    }
    
    /* Is this move a capture? */
    is_capture = (strchr(san, 'x') != 0);
    
    /* Is this move an en passant capture? */
    if(type == PAWN && is_capture && len > 5) {
        if(strcmp(san+len-5, " e.p.") == 0) {
            len -= 5;
        }
    }
    
    /* Is this move a check? */
    if(san[len-1] == '+') {
        len -= 1;
    }

    /* Is this move a check mate? */
    if(san[len-1] == '#') {
        len -= 1;
    }
    
    /* Is this move a pawn promotion? */
    if(type == PAWN && strchr(san, '=')) {
        /* To what piece? */
        switch(san[len-1]) {
        case 'N':
            promotion_type = KNIGHT;
            break;
        case 'B':
            promotion_type = BISHOP;
            break;
        case 'R':
            promotion_type = ROOK;
            break;
        case 'Q':
            promotion_type = QUEEN;
            break;
        default:
        /* Failure */
            return 0;
        }
        
        len -= 2;
    }
    
    /* To what square is the piece moving? */
    pos_to = (san[len-2]-'a') + (san[len-1]-'1') * 8;
    len -= 2;
    
    if(is_capture) {
        len -= 1;
    }
    
    /* Check from file/rank/square */
    if((type == PAWN && len > 0 ) || (type != PAWN && len > 1)) {
        const char *s = san;
        if(type != PAWN) {
            s += 1;
            len -= 1;
        }
        
        for(i = 0; i < len; i++) {
            if(s[i] >= 'a' && s[i] <= 'h') {
                file = s[i] - 'a';
            } else if(s[i] >= '1' && s[i] <= '8') {
                rank = s[i] - '1';
            }
        }
    }

    /* Find candidate move */
    for(i = 0; i < num_moves; i++) {
        int pos_from;
        move_t move = moves[i];
        
        if(MOVE_GET_TYPE(move) != type) continue;
        if((MOVE_IS_CAPTURE(move) != 0) != is_capture) continue;
        if(promotion_type && promotion_type != MOVE_PROMOTION_TYPE(move)) continue;
        if(MOVE_GET_POS_TO(move) != pos_to) continue;
        pos_from = MOVE_GET_POS_FROM(move);
        if(rank >= 0 && pos_from / 8 != rank) continue;
        if(file >= 0 && pos_from % 8 != file) continue;

        num_candidates++;
        candidate = move;
    }

    if(num_candidates == 1) {
        return candidate;
    } else {
        /* Failure */
        return 0;
    }
}
