#ifndef ENGINE_H
#define ENGINE_H

#define ENGINE_RESULT_NONE                          0
#define ENGINE_RESULT_ILLEGAL_MOVE                 -1

#define ENGINE_PROMOTION_NONE       0
#define ENGINE_PROMOTION_KNIGHT     1
#define ENGINE_PROMOTION_BISHOP     2
#define ENGINE_PROMOTION_ROOK       3
#define ENGINE_PROMOTION_QUEEN      4

#define ENGINE_SEARCH_NONE          0
#define ENGINE_SEARCH_RUNNING       1
#define ENGINE_SEARCH_COMPLETED     2

typedef struct engine_state engine_state_t;
typedef void (*thinking_output_cb)(int ply, int score, int time_ms, unsigned int nodes, int pv_length, int *pos_from, int *pos_to, int *promotion_type);

void ENGINE_create(engine_state_t **state);
void ENGINE_destroy(engine_state_t *state);
void ENGINE_reset(engine_state_t *state);
int  ENGINE_apply_move(engine_state_t *state, const int pos_from, const int pos_to, const int promotion_type);
int  ENGINE_apply_move_san(engine_state_t *state, const char *san);
int  ENGINE_search(engine_state_t *state, const int moves_left_in_period, const int time_left_ms, const int time_incremental_ms, const unsigned char max_depth, int *pos_from, int *pos_to, int *promotion_type);
void ENGINE_search_stop(engine_state_t *state);
void ENGINE_register_search_output_cb(engine_state_t *state, thinking_output_cb think_cb);
void ENGINE_resize_hashtable(engine_state_t *state, const int size_mb);
int  ENGINE_set_board(engine_state_t *state, const char *fen);
int  ENGINE_playing_side(engine_state_t *state);

#endif
