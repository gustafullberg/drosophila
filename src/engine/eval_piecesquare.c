const short piecesquare[7][64] = {
    {   /* PAWN */
        0,  0,  0,  0,  0,  0,  0,  0, /* N/A */
        0,  0,  0, -1, -1,  0,  0,  0,
        1,  1,  1,  2,  2,  1,  1,  1,
        2,  2,  2,  3,  3,  2,  2,  2,
        4,  4,  4,  5,  5,  4,  4,  4,
        6,  6,  6,  7,  7,  6,  6,  6,
       10, 10, 10, 10, 10, 10, 10, 10,
        0,  0,  0,  0,  0,  0,  0,  0  /* N/A */
    },
    {   /* KNIGHT */
       -1, -1,  0,  0,  0,  0, -1, -1,
        0,  1,  2,  2,  2,  2,  1,  0,
        0,  2,  3,  3,  3,  3,  2,  0,
        1,  2,  3,  4,  4,  3,  2,  1,
        1,  2,  3,  4,  4,  3,  2,  1,
        0,  2,  3,  3,  3,  3,  2,  0,
        0,  1,  2,  2,  2,  2,  1,  0,
       -1,  0,  0,  1,  1,  0,  0, -1
    },
    {   /* BISHOP */
       -4, -2, -1,  0,  0, -1, -2, -4,
       -2,  0,  0,  1,  1,  0,  0, -2,
        0,  1,  0,  2,  2,  0,  1,  0,
        0,  1,  2,  1,  1,  2,  1,  0,
        0,  1,  1,  1,  1,  1,  1,  0,
        0,  1,  1,  1,  1,  1,  1,  0,
       -2,  0,  0,  0,  0,  0,  0, -2,
       -4, -2,  0,  0,  0,  0, -2, -4
    },
    {   /* ROOK */
        0,  0,  0,  0,  0,  0,  0,  0,
       -1,  0,  0,  0,  0,  0,  0, -1,
       -1,  0,  0,  0,  0,  0,  0, -1,
       -1,  0,  0,  0,  0,  0,  0, -1,
       -1,  0,  0,  0,  0,  0,  0, -1,
       -1,  0,  0,  0,  0,  0,  0, -1,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    {   /* QUEEN */
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  1,  1,  1,  1,  1,  1,  0,
        0,  1,  2,  2,  2,  2,  1,  0,
        0,  1,  2,  3,  3,  2,  1,  0,
        0,  1,  2,  3,  3,  2,  1,  0,
        0,  1,  2,  2,  2,  2,  1,  0,
        0,  1,  1,  1,  1,  1,  1,  0,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    {   /* KING  mid-game */
        3,  6,  6,  0,  0,  0,  6,  3,
        1,  2,  2,  0,  0,  0,  2,  1,
       -4, -4, -4, -4, -4, -4, -4, -4,
       -6, -6, -6, -6, -6, -6, -6, -6,
      -10,-10,-10,-10,-10,-10,-10,-10,
      -10,-10,-10,-10,-10,-10,-10,-10,
      -10,-10,-10,-10,-10,-10,-10,-10,
      -10,-10,-10,-10,-10,-10,-10,-10
    },
    {   /* KING  end-game */
      -10,  0,  0,  0,  0,  0,  0,-10,
       -8, -4,  1,  1,  1,  1, -4, -8,
       -8, -4,  0,  2,  2,  0, -4, -8,
       -8, -4,  0,  4,  4,  0, -4, -8,
       -8, -4,  0,  4,  4,  0, -4, -8,
       -8, -4, -4,  0,  0, -4, -4, -8,
       -8, -6, -4, -4, -4, -4, -6, -8,
      -10, -8, -8, -8, -8, -8, -8,-10
    }
};

