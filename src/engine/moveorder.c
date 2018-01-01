#include "moveorder.h"
#include "see.h"

/* MVV-LVA: Capture bonus + Value of captured piece - Value of own piece */
static const int capture_score[6][6] = {
    { 10 + 1 - 1, 10 + 3 - 1, 10 + 4 - 1, 10 + 5 - 1, 10 + 9 - 1, 10 + 10 - 1 },
    { 10 + 1 - 3, 10 + 3 - 3, 10 + 4 - 3, 10 + 5 - 3, 10 + 9 - 3, 10 + 10 - 3 },
    { 10 + 1 - 4, 10 + 3 - 4, 10 + 4 - 4, 10 + 5 - 4, 10 + 9 - 4, 10 + 10 - 4 },
    { 10 + 1 - 5, 10 + 3 - 5, 10 + 4 - 5, 10 + 5 - 5, 10 + 9 - 5, 10 + 10 - 5 },
    { 10 + 1 - 9, 10 + 3 - 9, 10 + 4 - 9, 10 + 5 - 9, 10 + 9 - 9, 10 + 10 - 9 },
    { 10 + 1 -10, 10 + 3 -10, 10 + 4 -10, 10 + 5 -10, 10 + 9 -10, 10 + 10 -10 }
};

static int MOVEORDER_compute_score(const chess_state_t *s, const move_t move)
{
    unsigned int score = 0;
    
    const int pos_to = MOVE_GET_POS_TO(move);
    
    if(MOVE_IS_CAPTURE(move)) {
        /* MVV-LVA */
        int own_type      = MOVE_GET_TYPE(move);
        int captured_type = MOVE_GET_CAPTURE_TYPE(move);
        score += capture_score[own_type][captured_type];
        
        /* Recapture bonus */
        if(MOVE_IS_CAPTURE(s->last_move)) {
            if(pos_to == MOVE_GET_POS_TO(s->last_move)) {
                score += 5;
            }
        }
    }

    return score;
}

int MOVEORDER_rate_moves(const chess_state_t *s, move_t moves[], int num_moves, const move_t best_guess)
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score;
        if(moves[i] == best_guess) {
            /* This move is already tried. Remove it from the list */
            moves[i] = moves[num_moves-1];
            num_moves--;
        }
        score = MOVEORDER_compute_score(s, moves[i]);
        moves[i] |= score << MOVE_SCORE_SHIFT;
    }

    return num_moves;
}

int MOVEORDER_rate_moves_quiescence(const chess_state_t *s, move_t moves[], int num_moves)
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score = MOVEORDER_compute_score(s, moves[i]);
        moves[i] |= score << MOVE_SCORE_SHIFT;
    }

    return num_moves;
}

void MOVEORDER_best_move_first(move_t moves[], int num_moves)
{
    if(num_moves < 2) return;
    int best = 0;
    for(int i = 1; i < num_moves; i++) {
        if(moves[i] > moves[best]) best = i;
    }
    move_t tmp = moves[0];
    moves[0] = moves[best];
    moves[best] = tmp;
}
