#ifndef _ENGINE_H
#define _ENGINE_H

#define ENGINE_RESULT_NONE          0
#define ENGINE_RESULT_WHITE_MATES   1
#define ENGINE_RESULT_BLACK_MATES   2
#define ENGINE_RESULT_STALE_MATE    3

typedef struct engine_state engine_state_t;

void engine_create(engine_state_t **state);
void engine_destroy(engine_state_t *state);
void engine_reset(engine_state_t *state);
int  engine_opponent_move(engine_state_t *state, int pos_from, int pos_to, int promotion_type);
int  engine_ai_move(engine_state_t *state, int *pos_from, int *pos_to, int *promotion_type);
int  engine_result(engine_state_t *state);

#endif
