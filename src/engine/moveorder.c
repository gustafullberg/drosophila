#include "moveorder.h"

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
    const unsigned int max = (((unsigned int)MOVE_SCORE_MASK) >> MOVE_SCORE_SHIFT);
    unsigned int score = 0;
    
    const int pos_to = MOVE_GET_POS_TO(move);
    
    if(MOVE_IS_PROMOTION(move)) {
        score += 20;
    }
    
    if(MOVE_IS_CAPTURE(move)) {
        /* MVV-LVA */
        int own_type      = MOVE_GET_TYPE(move);
        int captured_type = MOVE_GET_CAPTURE_TYPE(move);
        score += capture_score[own_type][captured_type];
        
        /* Recapture bonus */
        if(MOVE_IS_CAPTURE(s->last_move)) {
            if(pos_to == MOVE_GET_POS_TO(s->last_move)) {
                score += 10;
            }
        }
    }

    if(score > max) {
        score = max;
    }
    
    return score;
}

static void MOVEORDER_sort(move_t moves[], const int num_moves)
{
#define MAX_NUM_MOVE_TO_SORT 4
    int i, j;
    move_t temp;
    int num_to_sort = num_moves;
    if(num_to_sort > MAX_NUM_MOVE_TO_SORT) {
        num_to_sort = MAX_NUM_MOVE_TO_SORT;
    }
    
    /* Selection sort (greatest first) */
    for(i = 0; i < num_to_sort-1; i++) {
        int index_highest = i;
        for(j = i+1; j < num_moves; j++) {
            if(moves[j] > moves[index_highest]) {
                index_highest = j;
            }
        }
        temp = moves[i];
        moves[i] = moves[index_highest];
        moves[index_highest] = temp;
    }
}

int MOVEORDER_order_moves(const chess_state_t *s, move_t moves[], int num_moves, const move_t best_guess)
{
    int i;
    
    /* Get score for each move */
    for(i = 0; i < num_moves; i++) {
        int score;
        if(moves[i] == best_guess) {
            /* This move is already tried. Remove it from the list */
            moves[i] = moves[num_moves-1];
            num_moves--;
        }
        score = MOVEORDER_compute_score(s, moves[i]);
        moves[i] |= score << MOVE_SCORE_SHIFT;
    }
    
    /* Sort moves by score */
    MOVEORDER_sort(moves, num_moves);
    
    return num_moves;
}
