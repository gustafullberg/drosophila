set(files
    ../engine/engine.h
    bitboard.c
    bitboard.h
    defines.h
    engine.c
    engine.h
    eval.c
    eval.h
    movegen.c
    movegen.h
    search.c
    search.h
    search_alphabeta.c
    search_alphabeta.h
    search_minimax.c
    search_minimax.h
    state.c
    state.h
)

add_library(
    pawnedengine
    ${files}
)

