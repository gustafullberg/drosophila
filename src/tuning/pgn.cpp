#include <cstring>
#include "pgn.hpp"

float PGN::nextGame()
{
    float result = 0.0f;

    /* Find start of game */
    game_start = strstr(game_end, "\n\n");
    if(!game_start) {
        eof = true;
        return result;
    }
    game_start += 2;

    /* Find end of game */
    game_end = strstr(game_start, "\n\n") + 2;

    /* Game result */
    if(game_end[-3] == '0') result = 1.0f;
    else if(game_end[-3] == '1') result = 0.0f;
    else if(game_end[-3] == '2') result = 0.5f;
    else {
        error = true;
        return result;
    }

    if(game) delete[] game;
    int game_len = game_end - game_start;
    game = new char[game_len+1];
    strncpy(game, game_start, game_len);
    game[game_len] = '\0';

    start_ptr = game;
    save_ptr = game;
    comment = false;

    game_number++;
    return result;
}

char *PGN::nextMove()
{
    char *t;
    while((t = strtok_r(start_ptr, " \n", &save_ptr))) {
        start_ptr = NULL;
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
        return t;
    }

    start_ptr = nullptr;
    return nullptr;
}

PGN::~PGN()
{
    if(game) delete[] game;
}
