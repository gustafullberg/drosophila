#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "engine.h"
#include "state.h"
#include "search.h"
#include "thread.h"
#include "eval.h"

#define NUM_THREADS 16

extern eval_param_t param;

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

float optimize(const char *buf, int *c_opt, int idx, float mse_start, int min, int max)
{
    int c_start = c_opt[idx];
    fprintf(stderr, "[%d] = %d => MSE %f\n", idx, c_opt[idx], mse_start);
    float mse_best = mse_start;

    // Step upwards
    while(c_opt[idx] < max) {
        c_opt[idx]++;
        fprintf(stderr, "[%d] = %d => ", idx, c_opt[idx]);
        float mse = run_test(buf);
        if(mse >= mse_best) {
            c_opt[idx]--;
            break;
        }
        mse_best = mse;
    }

    if(mse_best < mse_start) {
        fprintf(stderr, "   [%d] = %d best (previous %d)\n", idx, c_opt[idx], c_start);
        return mse_best;
    }

    // Step downwards
    while(c_opt[idx] > min) {
        c_opt[idx]--;
        fprintf(stderr, "[%d] = %d => ", idx, c_opt[idx]);
        float mse = run_test(buf);
        if(mse >= mse_best) {
            c_opt[idx]++;
            break;
        }
        mse_best = mse;
    }

    fprintf(stderr, "   [%d] = %d best (previous %d)\n", idx, c_opt[idx], c_start);
    return mse_best;
}

void print_value(int val)
{
    if(val >= 0) printf(" ");
    if(val < 10 && val > -10) printf(" ");
    printf("%d", val);
}

void print_psq(char *name, int psq[64])
{
    printf("        .%s =\n", name);
    printf("        {\n");
    for(int i = 0; i < 8; i++) {
        printf("          ");
        for(int j = 0; j < 8; j++) {
            print_value(psq[8*i+j]);
            if(i != 7 || j != 7) printf(",");
        }
        printf("\n");
    }
    printf("        },\n");
}

void print_mob(char *name, int *mob, int size)
{
    int len = size / sizeof(int);
    printf("        .%s", name);
    for(int i = 0; i < 8 - (int)strlen(name); i++) printf(" ");
    printf("= {");
    for(int i = 0; i < len; i++) {
        print_value(mob[i]);
        if(i < len - 1) printf(",");
    }
    printf(" },\n");
}

void print_params()
{
    printf("eval_param_t param =\n{\n");
    printf("    .psq = {\n");
    print_psq("pawn", param.psq.pawn);
    print_psq("knight", param.psq.knight);
    print_psq("bishop", param.psq.bishop);
    print_psq("rook", param.psq.rook);
    print_psq("queen", param.psq.queen);
    print_psq("king_midgame", param.psq.king_midgame);
    print_psq("king_endgame", param.psq.king_endgame);
    printf("    },\n");
    printf("    .mobility = {\n");
    print_mob("knight", param.mobility.knight, sizeof(param.mobility.knight));
    print_mob("bishop", param.mobility.bishop, sizeof(param.mobility.bishop));
    print_mob("rook_o", param.mobility.rook_o, sizeof(param.mobility.rook_o));
    print_mob("rook_e", param.mobility.rook_e, sizeof(param.mobility.rook_e));
    print_mob("queen_o", param.mobility.queen_o, sizeof(param.mobility.queen_o));
    print_mob("queen_e", param.mobility.queen_e, sizeof(param.mobility.queen_e));
    printf("    },\n");
    printf("    .tropism = {\n");
    print_mob("knight", param.tropism.knight, sizeof(param.tropism.knight));
    print_mob("bishop", param.tropism.bishop, sizeof(param.tropism.bishop));
    print_mob("rook", param.tropism.rook, sizeof(param.tropism.rook));
    print_mob("queen", param.tropism.queen, sizeof(param.tropism.queen));
    printf("    },\n");
    printf("    .positional = {\n");
    printf("        .pawn_guards_minor             = %d,\n", param.positional.pawn_guards_minor);
    printf("        .pawn_guards_pawn              = %d,\n", param.positional.pawn_guards_pawn);
    printf("        .pawn_shield_1                 = %d,\n", param.positional.pawn_shield_1);
    printf("        .pawn_shield_2                 = %d,\n", param.positional.pawn_shield_2);
    printf("        .pawn_passed_o                 = %d,\n", param.positional.pawn_passed_o);
    printf("        .pawn_passed_e                 = %d,\n", param.positional.pawn_passed_e);
    printf("        .pawn_passed_dist_kings_diff_e = %d,\n", param.positional.pawn_passed_dist_kings_diff_e);
    printf("        .pawn_passed_dist_own_king_e   = %d,\n", param.positional.pawn_passed_dist_own_king_e);
    printf("        .pawn_passed_unblocked         = %d,\n", param.positional.pawn_passed_unblocked);
    printf("        .pawn_passed_unreachable_e     = %d,\n", param.positional.pawn_passed_unreachable_e);
    printf("        .pawn_isolated_o               = %d,\n", param.positional.pawn_isolated_o);
    printf("        .pawn_isolated_e               = %d,\n", param.positional.pawn_isolated_e);
    printf("        .knight_reduction              = %d,\n", param.positional.knight_reduction);
    printf("        .rook_open_file_o              = %d,\n", param.positional.rook_open_file_o);
    printf("        .rook_open_file_e              = %d,\n", param.positional.rook_open_file_e);
    printf("        .rook_halfopen_file_o          = %d,\n", param.positional.rook_halfopen_file_o);
    printf("        .rook_halfopen_file_e          = %d,\n", param.positional.rook_halfopen_file_e);
    printf("        .rook_rearmost_pawn_o          = %d,\n", param.positional.rook_rearmost_pawn_o);
    printf("        .rook_rearmost_pawn_e          = %d,\n", param.positional.rook_rearmost_pawn_e);
    printf("        .tempo                         = %d,\n", param.positional.tempo);
    print_mob("pawn_passed_scaling ", param.positional.pawn_passed_scaling, sizeof(param.positional.pawn_passed_scaling));
    printf("    }\n");
    printf("};\n");
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

    fprintf(stderr, "Optimizing PSQ pawn\n");
    for(int i = 0; i < 64; i++) mse_best = optimize(buf, param.psq.pawn, i, mse_best, -10, 20);
    fprintf(stderr, "Optimizing PSQ knight\n");
    for(int i = 0; i < 64; i++) mse_best = optimize(buf, param.psq.knight, i, mse_best, -10, 10);
    fprintf(stderr, "Optimizing PSQ bishop\n");
    for(int i = 0; i < 64; i++) mse_best = optimize(buf, param.psq.bishop, i, mse_best, -10, 10);
    fprintf(stderr, "Optimizing PSQ rook\n");
    for(int i = 0; i < 64; i++) mse_best = optimize(buf, param.psq.rook, i, mse_best, -10, 10);
    fprintf(stderr, "Optimizing PSQ queen\n");
    for(int i = 0; i < 64; i++) mse_best = optimize(buf, param.psq.queen, i, mse_best, -10, 10);
    fprintf(stderr, "Optimizing PSQ king_midgame\n");
    for(int i = 0; i < 64; i++) mse_best = optimize(buf, param.psq.king_midgame, i, mse_best, -10, 10);
    fprintf(stderr, "Optimizing PSQ king_endgame\n");
    for(int i = 0; i < 64; i++) mse_best = optimize(buf, param.psq.king_endgame, i, mse_best, -10, 10);

    free(buf);
    print_params();
    return 0;
}
