set(files
    ../engine/engine.h
    bitboard.c
    bitboard.h
    defines.h
    engine.c
    engine.h
    eval.c
    eval.h
    move.c
    move.h
    search.c
    search.h
    search_alphabeta.c
    search_alphabeta.h
    search_quiescence.c
    search_quiescence.h
    state.c
    state.h
)

add_library(
    pawnedengine
    ${files}
)

