/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include "search_alphabeta.h"
#include "search_mtdf.h"

ttable_t *ttable;
move_t stack[5000];

void test_alphabeta()
{
    chess_state_t s;
    move_t move_ab, move_mtdf;
    int s_ab, s_mtdf;
    int ply;
    int depth = 4;
    
    /* Start out with fresh state */
    STATE_reset(&s);

    for(ply = 0; ply < 20; ply++) {
        printf("\nPly: %d\n", ply);
        s_mtdf = SEARCH_mtdf(&s, stack, ttable, 4, &move_mtdf, 0);
        printf("MTD(f): %d ", s_mtdf);
        STATE_move_print_debug(move_mtdf);
        s_ab   = SEARCH_alphabeta(&s, stack, ttable, 4, &move_ab, -SHRT_MAX-depth, SHRT_MAX+depth);
        printf("AB:     %d ", s_ab);
        STATE_move_print_debug(move_ab);

        assert(s_mtdf == s_ab);
        assert(move_mtdf == move_ab);
        
        STATE_apply_move(&s, move_mtdf);
    }
}

int main()
{
    bitboard_init();
    ttable = TTABLE_create(20);
    test_alphabeta();
    TTABLE_destroy(ttable);
    return 0;
}
