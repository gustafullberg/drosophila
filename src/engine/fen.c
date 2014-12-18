#include <string.h>
#include "fen.h"

/* Create a state from Forsyth–Edwards Notation */
int FEN_read(chess_state_t *state, const char *fen)
{
    int i = 0, j;
    int rank = 7, file = 0;
    char c;

    /* Clear the state */
    memset(state, 0, sizeof(chess_state_t));
    
    /* Read positions of all pieces */
    while(!(rank == 0 && file > 7)) {
        bitboard_t pos = BITBOARD_RANK_FILE(rank, file);
        c = fen[i++];
        
        if(c >= '1' && c <= '8') {
            file += c - '0';
        }
        
        else if(c == '/') {
            file = 0;
            rank--;
        }
        
        else if(c >= 'B' && c <= 'r') {
            int bitboard_number = 0;
            if(c == 'P') bitboard_number = WHITE_PIECES + PAWN;
            else if(c == 'N') bitboard_number = WHITE_PIECES + KNIGHT;
            else if(c == 'B') bitboard_number = WHITE_PIECES + BISHOP;
            else if(c == 'R') bitboard_number = WHITE_PIECES + ROOK;
            else if(c == 'Q') bitboard_number = WHITE_PIECES + QUEEN;
            else if(c == 'K') bitboard_number = WHITE_PIECES + KING;
            else if(c == 'p') bitboard_number = BLACK_PIECES + PAWN;
            else if(c == 'n') bitboard_number = BLACK_PIECES + KNIGHT;
            else if(c == 'b') bitboard_number = BLACK_PIECES + BISHOP;
            else if(c == 'r') bitboard_number = BLACK_PIECES + ROOK;
            else if(c == 'q') bitboard_number = BLACK_PIECES + QUEEN;
            else if(c == 'k') bitboard_number = BLACK_PIECES + KING;
            else { return 0; }
            
            state->bitboard[bitboard_number] |= pos;
            file++;
        }
        
        else { return 0; }
    }

    /* Update the "ALL" bitboards */
    for(j = PAWN; j <= KING; j++) {
        state->bitboard[WHITE_PIECES+ALL] |= state->bitboard[WHITE_PIECES+j];
        state->bitboard[BLACK_PIECES+ALL] |= state->bitboard[BLACK_PIECES+j];
    }
    state->bitboard[OCCUPIED] = state->bitboard[WHITE_PIECES+ALL] | state->bitboard[BLACK_PIECES+ALL];
    
    /* Expect space */
    if(fen[i++] != ' ') { return 0; }
    
    /* Color to move */
    c = fen[i++];
    if(c == 'w') state->player = WHITE;
    else if(c == 'b') state->player = BLACK;
    else { return 0; }
    
    /* Excpect space */
    if(fen[i++] != ' ') { return 0; }
    
    /* Castling rights */
    while((c = fen[i++]) != ' ') {
        if(c == 'K') state->castling[WHITE] |= STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
        else if(c == 'Q') state->castling[WHITE] |= STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK;
        else if(c == 'k') state->castling[BLACK] |= STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK;
        else if(c == 'q') state->castling[BLACK] |= STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK;
        else if(c != '-') { return 0; }
    }

    /* En passant file */
    c = fen[i++];
    if(c != '-') {
        int ep_file = c - 'a';
        i++;
        if(bitboard_ep_capturers[(int)state->player][ep_file] & state->bitboard[state->player*NUM_TYPES+PAWN]) {
            state->ep_file = ep_file;
        } else {
            state->ep_file = STATE_EN_PASSANT_NONE;
        }
    } else {
        state->ep_file = STATE_EN_PASSANT_NONE;
    }

    /* Compute the hash of the state */
    STATE_compute_hash(state);
    state->last_move = 0;
    
    /* Success */
    return 1;
}
