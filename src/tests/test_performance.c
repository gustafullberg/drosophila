/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "engine.h"

#define DEPTH 11

void move(engine_state_t *engine, const char *move_white, const char *move_black)
{
    int result, pos_from, pos_to, promotion_type;
    
    ENGINE_think_start(engine, 1, 1000000, 0, DEPTH);
    ENGINE_think_get_result(engine, &pos_from, &pos_to, &promotion_type);
    
    if(move_white) {
        result = ENGINE_apply_move_san(engine, move_white);
        assert(result==0);
    }
    
    if(move_black) {
        result = ENGINE_apply_move_san(engine, move_black);
        assert(result==0);
    }
    
    fprintf(stderr, ".");
}

int main()
{
    engine_state_t *engine;
    ENGINE_create(&engine);
    srand(0); /* Force opening book to always choose the same moves */
    
    move(engine, "d4", "Nf6");
    move(engine, "c4", "e6");
    move(engine, "Nc3", "Bb4");
    move(engine, "e3", "O-O");
    move(engine, "Nge2", "d5");
    move(engine, "a3", "Be7");
    move(engine, "cxd5", "Nxd5");
    move(engine, "Bd2", "Nd7");
    move(engine, "g3", "b6");
    move(engine, "Nxd5", "exd5");
    move(engine, "Bg2", "Bb7");
    move(engine, "Bb4", "Nf6");
    move(engine, "O-O", "Re8");
    move(engine, "Rc1", "c6");
    move(engine, "Bxe7", "Rxe7");
    move(engine, "Re1", "Qd6");
    move(engine, "Nf4", "Bc8");
    move(engine, "Qa4", "Rc7");
    move(engine, "f3", "Be6");
    move(engine, "e4", "dxe4");
    move(engine, "fxe4", "Qd7");
    move(engine, "d5", "cxd5");
    move(engine, "Qxd7", "Rxd7");
    move(engine, "Nxe6", "fxe6");
    move(engine, "Bh3", "Kh8");
    move(engine, "e5", "Ng8");
    move(engine, "Bxe6", "Rdd8");
    move(engine, "Rc7", "d4");
    move(engine, "Bd7", NULL);
    
    ENGINE_destroy(engine);
    fprintf(stderr, "\n");
    return 0;
}
