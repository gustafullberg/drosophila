#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <map>
extern "C" {
#include "engine.h"
#include "state.h"
#include "search.h"
#include "thread.h"
#include "eval.h"
}
#include "pgn.hpp"

#define NUM_THREADS 16

extern eval_param_t param;

struct Result {
    float target;
    float sum_val;
    float num;
    float result;
    bool hasResult;
    Result() : target(0.0f), sum_val(0.0f), num(0.0f), result(0.0f), hasResult(false) {}
    void update(float result) { sum_val += result; num += 1.0f; target = sum_val / num; }
};

std::map<uint64_t, Result> res; // Global

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

float computeMse()
{
    float e2_tot = 0.0;
    float num_pos = 0.0f;
    for(auto &kv : res) {
        Result &r = kv.second;
        if(r.hasResult) {
            e2_tot += error(r.target, r.result);
            num_pos++;
            r.hasResult = false;
        }
    }
    return sqrtf(e2_tot/num_pos);
}

void computeTarget(char *buf)
{
    engine_state_t *engine;
    ENGINE_create(&engine);
    PGN pgn(buf);
    int c = 0;
    while(1) {
        // New game
        ENGINE_reset(engine);

        float result = pgn.nextGame();
        if(pgn.isEOF()) break;
        if(pgn.isError()) {
            fprintf(stderr, "Error: Could not parse file\n");
            exit(1);
        }

        /* Parse game */
        char *san;
        while((san = pgn.nextMove())) {
            int r = ENGINE_apply_move_san(engine, san);
            if(r == ENGINE_RESULT_ILLEGAL_MOVE) {
                fprintf(stderr, "SAN parse error: \"%s\"\n", san);
                exit(1);
            }
            c++;

            uint64_t hash = ENGINE_get_hash(engine);
            Result &target = res[hash];
            target.update(result);
        }
    }
    ENGINE_destroy(engine);

    printf("Total positions  %d\nUnique positions %d\n", c, res.size());
}

typedef struct {
    int index;
    const char *buf;
} thread_arg_t;

void* worker_thread(void *_arg)
{
    thread_arg_t *arg = (thread_arg_t*)_arg;
    engine_state_t *engine;
    ENGINE_create(&engine);

    PGN pgn(arg->buf);

    do {
        float result = pgn.nextGame();
        if(pgn.isEOF()) break;
        if(pgn.isError()) {
            fprintf(stderr, "Error: Could not parse file\n");
            exit(1);
        }

        ENGINE_reset(engine);

        //if(pgn.gameNumber() > 1000) break;
        if(pgn.gameNumber() % NUM_THREADS != arg->index) continue;

        /* Parse game */
        int half_moves = 0;
        char *san;
        while((san = pgn.nextMove())) {
            int r = ENGINE_apply_move_san(engine, san);
            if(r == ENGINE_RESULT_ILLEGAL_MOVE) {
                fprintf(stderr, "SAN parse error: \"%s\"\n", san);
                exit(1);
            }

            half_moves++;
            if(half_moves < 20) continue;

            uint64_t hash = ENGINE_get_hash(engine);
            {
                Result &target = res[hash];
                if(target.hasResult) continue;
            }

            /* Evaluate position */
            int pos_from, pos_to, promotion_type;
            short score = ENGINE_search(engine, 1, 3600*1000, 0, 0, &pos_from, &pos_to, &promotion_type);
            if(half_moves % 2 == 1) score = -score; /* Invert score for black */

            Result &target = res[hash];
            target.result = float(score);
            target.hasResult = true;
        }
    } while(1);

    ENGINE_destroy(engine);

    return 0;
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

    for(int i = 0; i < NUM_THREADS; i++) {
        THREAD_join(t[i]);
    }

    float mse = computeMse();
    return mse;
}

float optimize(const char *buf, int *x, int idx, float mse_start, int min, int max)
{
    int max_diff = 1;
    int x_init = x[idx];
    if(min < x_init - max_diff) min = x_init - max_diff;
    if(max > x_init + max_diff) max = x_init + max_diff;

    fprintf(stderr, "[%d] = %d=>%f", idx, x[idx], mse_start);
    float mse_best = mse_start;

    // Step upwards
    while(x[idx] < max) {
        x[idx]++;
        float mse = run_test(buf);
        fprintf(stderr, ", %d=>%f", x[idx], mse);
        if(mse >= mse_best) {
            x[idx]--;
            break;
        }
        mse_best = mse;
    }

    if(mse_best < mse_start) {
        fprintf(stderr, "  CHANGED from %d to %d - MSE %f\n", x_init, x[idx], mse_best);
        return mse_best;
    }

    // Step downwards
    while(x[idx] > min) {
        x[idx]--;
        float mse = run_test(buf);
        fprintf(stderr, ", %d=>%f", x[idx], mse);
        if(mse >= mse_best) {
            x[idx]++;
            break;
        }
        mse_best = mse;
    }

    if(mse_best < mse_start) {
        fprintf(stderr, "  CHANGED from %d to %d - MSE %f\n", x_init, x[idx], mse_best);
    } else {
        fprintf(stderr, "\n");
    }
    return mse_best;
}

float tune_array(const char *buf, int *x, int len, float mse, int min, int max)
{
    int *tuned = (int*)calloc(len, sizeof(int));
    int num_left = len;
    int idx;

    while(num_left) {
        do {
            idx = rand() % len;
        } while(tuned[idx] != 0);
        num_left--;
        tuned[idx] = 1;
        mse = optimize(buf, x, idx, mse, min, max);
    }

    return mse;
}

void print_value(int val)
{
    if(val >= 0) printf(" ");
    if(val < 10 && val > -10) printf(" ");
    printf("%d", val);
}

void print_psq(const char *name, int psq[64])
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

void print_mob(const char *name, int *mob, int size)
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
    printf("    .pressure = {\n");
    print_mob("knight", param.pressure.knight, sizeof(param.pressure.knight));
    print_mob("bishop", param.pressure.bishop, sizeof(param.pressure.bishop));
    print_mob("rook", param.pressure.rook, sizeof(param.pressure.rook));
    print_mob("queen", param.pressure.queen, sizeof(param.pressure.queen));
    print_mob("scaling_midgame", param.pressure.scaling_midgame, sizeof(param.pressure.scaling_midgame));
    print_mob("scaling_endgame", param.pressure.scaling_endgame, sizeof(param.pressure.scaling_endgame));
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
    fflush(stdout);
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
    char *buf = (char*)malloc(file_size);
    fread(buf, 1, file_size, f);
    fclose(f);

    computeTarget(buf);

    float mse_initial = run_test(buf);
    float mse_start = mse_initial;
    float mse_best = mse_start;
    fprintf(stderr, "Initial => %f\n", mse_initial);

    for(int iter = 0; iter < 10; iter++) {
#if 1
        fprintf(stderr, "Optimizing mobility\n");
        mse_best = tune_array(buf, (int*)&param.mobility, sizeof(param.mobility)/sizeof(int), mse_best, -10, 10);
#endif
#if 1
        fprintf(stderr, "Optimizing king pressure\n");
        mse_best = tune_array(buf, (int*)&param.pressure.knight, sizeof(param.pressure.knight)/sizeof(int), mse_best, 0, 15);
        mse_best = tune_array(buf, (int*)&param.pressure.bishop, sizeof(param.pressure.bishop)/sizeof(int), mse_best, 0, 15);
        mse_best = tune_array(buf, (int*)&param.pressure.rook, sizeof(param.pressure.rook)/sizeof(int), mse_best, 0, 15);
        mse_best = tune_array(buf, (int*)&param.pressure.queen, sizeof(param.pressure.queen)/sizeof(int), mse_best, 0, 15);
        mse_best = tune_array(buf, (int*)&param.pressure.scaling_midgame, sizeof(param.pressure.scaling_midgame)/sizeof(int), mse_best, 0, 64);
        mse_best = tune_array(buf, (int*)&param.pressure.scaling_endgame, sizeof(param.pressure.scaling_endgame)/sizeof(int), mse_best, 0, 64);
#endif
#if 1
        fprintf(stderr, "Optimizing positional parameters\n");
        mse_best = tune_array(buf, (int*)&param.positional, sizeof(param.positional)/sizeof(int), mse_best, -25, 300);
#endif
#if 1
        fprintf(stderr, "Optimizing PSQ pawn\n");
        mse_best = tune_array(buf, param.psq.pawn, 64, mse_best, -10, 20);
        fprintf(stderr, "Optimizing PSQ knight\n");
        mse_best = tune_array(buf, param.psq.knight, 64, mse_best, -10, 10);
        fprintf(stderr, "Optimizing PSQ bishop\n");
        mse_best = tune_array(buf, param.psq.bishop, 64, mse_best, -10, 10);
        fprintf(stderr, "Optimizing PSQ rook\n");
        mse_best = tune_array(buf, param.psq.rook, 64, mse_best, -10, 10);
        fprintf(stderr, "Optimizing PSQ queen\n");
        mse_best = tune_array(buf, param.psq.queen, 64, mse_best, -10, 10);
        fprintf(stderr, "Optimizing PSQ king_midgame\n");
        mse_best = tune_array(buf, param.psq.king_midgame, 64, mse_best, -10, 10);
        fprintf(stderr, "Optimizing PSQ king_endgame\n");
        mse_best = tune_array(buf, param.psq.king_endgame, 64, mse_best, -10, 10);
#endif
        print_params();
        fprintf(stderr, "\nMSE reduction in iteration %d: %f. Total reduction: %f.\n\n", iter, mse_start - mse_best, mse_initial - mse_best);
        if(mse_best >= mse_start) break;
        mse_start = mse_best;
    }
    free(buf);

    return 0;
}
