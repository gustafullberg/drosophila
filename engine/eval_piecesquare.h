#ifndef _EVAL_PIECESQUARE_H
#define _EVAL_PIECESQUARE_H

const int piecesquare[7][64] = {
    {   /* PAWN */
        0,  0,  0,  0,  0,  0,  0,  0, /* N/A */
        0,  0,  0,  0,  0,  0,  0,  0,
        5,  5,  5, 10, 10,  5,  5,  5,
       10, 10, 10, 15, 15, 10, 10, 10,
       18, 18, 18, 23, 23, 18, 18, 18,
       30, 30, 30, 35, 35, 30, 30, 30,
       50, 50, 50, 50, 50, 50, 50, 50,
        0,  0,  0,  0,  0,  0,  0,  0  /* N/A */
    },
    {   /* KNIGHT */
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  5,  5,  5,  5,  5,  5,  0,
        5, 10, 10, 15, 15, 10, 10,  5,
       10, 15, 15, 20, 20, 15, 15, 10,
       10, 15, 15, 15, 15, 15, 15, 10,
        5, 10, 10, 10, 10, 10, 10,  5,
        0,  5,  5,  5,  5,  5,  5,  0
    },
    {   /* BISHOP */
      -20,-10,  0,  0,  0,  0,-10,-20,
      -10,  0,  0,  5,  5,  0,  0,-10,
        0,  5,  0, 10, 10,  0,  5,  0,
        0,  5, 10,  5,  5, 10,  5,  0,
        0,  5,  5,  5,  5,  5,  5,  0,
        0,  5,  5,  5,  5,  5,  5,  0,
      -10,  0,  0,  0,  0,  0,  0,-10,
      -20,-10,  0,  0,  0,  0,-10,-20
    },
    {   /* ROOK */
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    {   /* QUEEN */
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    {   /* KING  mid-game */
       15, 30, 30,  0,  0,  0, 30, 15,
        5, 10, 10,  0,  0,  0, 10,  5,
      -20,-20,-20,-20,-20,-20,-20,-20,
      -30,-30,-30,-30,-30,-30,-30,-30,
      -50,-50,-50,-50,-50,-50,-50,-50,
      -50,-50,-50,-50,-50,-50,-50,-50,
      -50,-50,-50,-50,-50,-50,-50,-50,
      -50,-50,-50,-50,-50,-50,-50,-50
    },
    {   /* KING  end-game */
      -50,  0,  0,  0,  0,  0,  0,-50,
      -40,-20,  5,  5,  5,  5,-20,-40,
      -40,-20,  0, 10, 10,  0,-20,-40,
      -40,-20,  0, 20, 20,  0,-20,-40,
      -40,-20,  0, 20, 20,  0,-20,-40,
      -40,-20,-20,  0,  0,-20,-20,-40,
      -40,-30,-20,-20,-20,-20,-30,-40,
      -50,-40,-40,-40,-40,-40,-40,-50
    }
};

#endif
