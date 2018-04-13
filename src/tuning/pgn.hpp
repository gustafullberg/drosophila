#ifndef PGN_HPP
#define PGN_HPP

class PGN
{
private:
    const char *game_start;
    const char *game_end;
    bool eof = false;
    bool error = false;
    int game_number = 0;
    char *game;
    char *start_ptr;
    char *save_ptr;
    bool comment;
public:
    PGN(const char *buffer) : game_end(buffer), game(nullptr) {}
    ~PGN();
    float nextGame();
    char *nextMove();
    bool isEOF() { return eof; }
    bool isError() { return error; }
    int gameNumber() { return game_number; }
};
#endif
