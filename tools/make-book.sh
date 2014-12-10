#!/bin/bash

# white.pgn/black.pgn are all games from Million Base won by white/black where both players have ELO >= 2200

echo 'FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"' > startpos.txt

pgn-extract -s -C -N -V -tstartpos.txt -owhitefix.pgn white.pgn
pgn-extract -s -C -N -V -tstartpos.txt -oblackfix.pgn black.pgn

polyglot make-book -bin white.bin -pgn whitefix.pgn -only-white -max-ply 16 -min-game 10
polyglot make-book -bin black.bin -pgn blackfix.pgn -only-black -max-ply 16 -min-game 10

polyglot merge-book -out book.bin -in1 white.bin -in2 black.bin

# Thanks to Steve Maughan, http://www.chessprogramming.net/creating-opening-book-maverick/
