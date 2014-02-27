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
typedef void (*thinking_output_cb)(int, int);

void ENGINE_create(engine_state_t **state);
void ENGINE_destroy(engine_state_t *state);
void ENGINE_reset(engine_state_t *state);
int  ENGINE_apply_move(engine_state_t *state, const int pos_from, const int pos_to, const int promotion_type);
int  ENGINE_apply_move_san(engine_state_t *state, const char *san);
void ENGINE_think(engine_state_t *state, const int moves_left_in_period, const int time_left_ms, const int time_incremental_ms, int *pos_from, int *pos_to, int *promotion_type, const unsigned char max_depth);
int  ENGINE_result(const engine_state_t *state);
void ENGINE_register_thinking_output_cb(engine_state_t *state, thinking_output_cb think_cb);
void ENGINE_resize_hashtable(engine_state_t *state, const int size_mb);

#endif
