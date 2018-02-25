set(files
    bitboard.c
    bitboard.h
    bitboard_zobrist.c
    clock.c
    clock.h
    defines.h
    engine.c
    engine.h
    eval.c
    eval.h
    fen.c
    fen.h
    hashtable.c
    hashtable.h
    history.c
    history.h
    movegen.c
    movegen.h
    moveorder.c
    moveorder.h
    openingbook.c
    openingbook.h
    san.c
    san.h
    search.c
    search.h
    search_mtdf.c
    search_mtdf.h
    search_nullwindow.c
    search_nullwindow.h
    see.c
    see.h
    state.c
    state.h
    thread.c
    thread.h
)

add_library(
    ${LIB_NAME}
    ${files}
)

target_link_libraries(${LIB_NAME} ${CMAKE_THREAD_LIBS_INIT})

