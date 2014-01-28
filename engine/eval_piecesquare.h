#ifndef _EVAL_PIECESQUARE_H
#define _EVAL_PIECESQUARE_H

const int piecesquare[6][64] = {
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
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0
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
    {   /* KING */
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0
    },
};

#endif