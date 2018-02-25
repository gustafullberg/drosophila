#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "engine.h"
#include "state.h"
#include "search.h"
#include "thread.h"
#include "eval.h"

#define NUM_THREADS 4

float sigmoid(float x)
{
    const float K = 5.0f / 400.0f;
    return 1.0f / (1.0f + powf(10.0f, -K * x));
}

float error(float result, float score)
{
    float e = result - sigmoid(score);
    return e*e;
}

typedef struct {
    int index;
    const char *buf;
    float error;
    int num_positions;
} thread_arg_t;

void* worker_thread(void *_arg)
{
    thread_arg_t *arg = (thread_arg_t*)_arg;
    arg->error = 0;
    arg->num_positions = 0;

    engine_state_t *engine;
    ENGINE_create(&engine);

    char game[1024*100];
    const char *game_end = arg->buf;
    const char *game_start;
    int game_number = 0;
    do {
        /* Find start of game */
        game_start = strstr(game_end, "\n\n");
        if(!game_start) break;
        game_start += 2;

        /* Find end of game */
        game_end = strstr(game_start, "\n\n") + 2;

        ENGINE_reset(engine);

        /* Game result */
        float result;
        if(game_end[-3] == '0') result = 1.0f;
        else if(game_end[-3] == '1') result = 0.0f;
        else if(game_end[-3] == '2') result = 0.5f;
        else {
            fprintf(stderr, "Error: Could not parse file\n");
            exit(1);
        }

        int game_len = game_end - game_start;
        strncpy(game, game_start, game_len);
        game[game_len] = '\0';
        if(game_end - game_start - 2 > sizeof(game) ) {
            fprintf(stderr, "\ntoo small game buffer\n");
            exit(1);
        }

        ++game_number;
        //if(game_number>1000) break;
        if(game_number % NUM_THREADS != arg->index) continue;


        /* Parse game */
        char *s = game;
        char *t;
        int comment = 0;
        int half_moves = 0;
        char *save_ptr;
        while(t = strtok_r(s, " \n", &save_ptr)) {
            s = NULL;
            int len = strlen(t);
            if(len == 0) continue;
            if(t[0] == '{') {
                comment = 1;
                continue;
            }
            if(t[len-1] == '}') {
                comment = 0;
                continue;
            }
            if(comment) continue;

            if(t[0] >= '0' && t[0] <= '9') continue;

            int r = ENGINE_apply_move_san(engine, t);
            if(r == ENGINE_RESULT_ILLEGAL_MOVE) {
                break;
            }

            half_moves++;

            chess_state_t **state = (chess_state_t **)engine;

            if(half_moves < 12) continue;
            short score = ENGINE_static_evaluation(engine);
            arg->num_positions++;

            float e2 = error(result, score);
            arg->error += e2;
        }
    } while(1);

    ENGINE_destroy(engine);
}

float run_test(const char *buf)
{
    thread_arg_t t_arg[NUM_THREADS];
    thread_t t[NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++) {
        t_arg[i].index = i;
        t_arg[i].buf = buf;
        THREAD_create(&t[i], worker_thread, &t_arg[i]);
    }

    float e2_tot = 0.0f;
    int num_pos = 0;
    for(int i = 0; i < NUM_THREADS; i++) {
        THREAD_join(t[i]);
        e2_tot += t_arg[i].error;
        num_pos += t_arg[i].num_positions;
    }

    float mse = sqrtf(e2_tot/num_pos);
    printf("MSE %f\n", mse);

    return mse;
}
extern eval_param_t param;
int *c_opt = &param.mobility.queen_o[0];
int num_elem = 28;

float optimize(const char *buf, int idx, float mse_start)
{
    int c_start = c_opt[idx];
    fprintf(stderr, "Initial value for c_opt[%d] = %d => MSE %f\n", idx, c_opt[idx], mse_start);
    //float mse_start = run_test(buf);
    float mse_best = mse_start;

    // Step upwards
    while(1) {
        c_opt[idx]++;
        fprintf(stderr, "Testing for c_opt[%d] = %d => ", idx, c_opt[idx]);
        float mse = run_test(buf);
        if(mse >= mse_best) break;
        mse_best = mse;
    }
    c_opt[idx]--;

    if(mse_best < mse_start) {
        fprintf(stderr, "Coefficient optimized at value %d to MSE %f (from value %d, MSE %f)\n", c_opt[idx], mse_best, c_start, mse_start);
        return mse_best;
    }

    // Step downwards
    while(1) {
        c_opt[idx]--;
        fprintf(stderr, "Testing for c_opt[%d] = %d => ", idx, c_opt[idx]);
        float mse = run_test(buf);
        if(mse >= mse_best) break;
        mse_best = mse;
    }
    c_opt[idx]++;

    fprintf(stderr, "Coefficient optimized at value %d to MSE %f (from value %d, MSE %f)\n", c_opt[idx], mse_best, c_start, mse_start);
    return mse_best;
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "Error: No input file\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if(!f) {
        fprintf(stderr, "Error: Could not open file: %s\n", argv[1]);
        return 2;
    }

    /* File size */
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Read and close file */
    char *buf = malloc(file_size);
    fread(buf, 1, file_size, f);
    fclose(f);

    fprintf(stderr, "Initial => ");
    float mse_best = run_test(buf);
    for(int i = 0; i < num_elem; i++) {
        mse_best = optimize(buf, i, mse_best);
    }
    for(int i = 0; i < num_elem; i++) {
        fprintf(stdout, "%d, ", c_opt[i]);
    }

    free(buf);
    return 0;
}
