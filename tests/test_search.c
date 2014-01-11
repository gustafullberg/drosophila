/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include "search_alphabeta.h"
#include "search_mtdf.h"

ttable_t *ttable1;
ttable_t *ttable2;

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
        s_mtdf = SEARCH_mtdf(&s, ttable1, depth, &move_mtdf, 0);
        printf("MTD(f): %d ", s_mtdf);
        STATE_move_print_debug(move_mtdf);
        s_ab   = SEARCH_alphabeta(&s, ttable2, depth, &move_ab, -SHRT_MAX-depth, SHRT_MAX+depth);
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
    ttable1 = TTABLE_create(20);
    ttable2 = TTABLE_create(20);
    test_alphabeta();
    TTABLE_destroy(ttable1);
    TTABLE_destroy(ttable2);
    return 0;
}
