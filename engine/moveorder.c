#include "moveorder.h"

static int MOVEORDER_compute_score(move_t move)
{
    int score;
    int max;
    
    score = 0;
    max = MOVE_SCORE_MASK >> MOVE_SCORE_SHIFT;
    
    if(MOVE_IS_PROMOTION(move)) {
        score += 1;
    }
    
    if(MOVE_IS_CAPTURE(move)) {
        score += 1;
    }
    
    if(score > max) {
        score = max;
    }
    
    return score;
}

static void MOVERORDER_sort(move_t moves[], int num_moves)
{
    int i, j;
    move_t temp;
    
    /* Selection sort (greatest first) */
    for(i = 0; i < num_moves-1; i++) {
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

void MOVEORDER_order_moves(move_t moves[], int num_moves)
{
    int i;
    
    /* Get score for each move */
    for(i = 0; i < num_moves; i++) {
        int score = MOVEORDER_compute_score(moves[i]);
        moves[i] |= score << MOVE_SCORE_SHIFT;
    }
    
    /* Sort moves by score */
    MOVERORDER_sort(moves, num_moves);
}
