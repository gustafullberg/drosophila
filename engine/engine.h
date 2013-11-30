#ifndef _ENGINE_H
#define _ENGINE_H

#define ENGINE_RESULT_NONE          0
#define ENGINE_RESULT_WHITE_MATES   1
#define ENGINE_RESULT_BLACK_MATES   2
#define ENGINE_RESULT_STALE_MATE    3
#define ENGINE_RESULT_ILLEGAL_MOVE -1

#define ENGINE_PROMOTION_NONE       0
#define ENGINE_PROMOTION_KNIGHT     1
#define ENGINE_PROMOTION_BISHOP     2
#define ENGINE_PROMOTION_ROOK       3
#define ENGINE_PROMOTION_QUEEN      4

typedef struct engine_state engine_state_t;

void ENGINE_create(engine_state_t **state);
void ENGINE_destroy(engine_state_t *state);
void ENGINE_reset(engine_state_t *state);
int  ENGINE_apply_move(engine_state_t *state, int pos_from, int pos_to, int promotion_type);
int  ENGINE_think_and_move(engine_state_t *state, int *pos_from, int *pos_to, int *promotion_type);
int  ENGINE_result(engine_state_t *state);

#endif
