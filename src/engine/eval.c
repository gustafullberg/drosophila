#include "eval.h"
#include "movegen.h"

eval_param_t param =
{
    .psq = {
        .pawn =
        {
            0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0, -2, -2,  3,  2,  0,
            1, -1,  0, -1,  0,  1,  0,  1,
            1,  2,  1,  2,  2,  0,  1,  1,
            2,  2,  2,  2,  2,  2,  2,  2,
            5,  6,  6,  6,  6,  6,  6,  5,
           10, 14, 14, 14, 14, 14, 14, 10,
            0,  0,  0,  0,  0,  0,  0,  0
        },
        .knight =
        {
           -7, -1, -4, -1,  1,  2,  1, -5,
           -4,  0, -2,  3,  4,  1,  2,  4,
           -2,  0,  1,  4,  6,  3,  4, -1,
            1,  4,  5,  3,  5,  4,  4,  1,
            4,  6,  7,  7,  5,  8,  2,  4,
            1,  4,  8,  8,  6,  7,  5,  1,
           -2,  1,  3,  3,  3,  3,  1, -2,
           -5, -2, -1,  1,  1, -1, -2, -5
        },
        .bishop =
        {
           -3, -3, -2, -4, -1, -1, -6, -5,
           -3,  0, -1, -1,  0,  0,  4, -4,
           -3,  0,  1,  2,  2,  1,  0,  1,
           -3,  1,  3,  4,  4,  1,  0,  0,
           -1,  1,  1,  4,  4,  1,  0, -1,
           -2,  2,  4,  1,  2,  5,  2,  1,
           -1, -1,  1,  1,  2,  1,  0, -6,
           -2,  2, -2,  1,  0, -2, -2, -2
        },
        .rook =
        {
           -2, -1, -1, -1,  0,  0,  1,  0,
           -3, -2, -2, -2, -2,  0, -1, -3,
           -3, -3, -1, -2, -2, -2,  1, -2,
            2,  2,  1,  0, -1,  0,  0, -1,
            4,  3,  3,  2,  1,  0,  2,  1,
            5,  4,  4,  4,  2,  4,  3,  2,
            6,  5,  5,  5,  4,  3,  4,  5,
            5,  6,  5,  3,  3,  4,  4,  4
        },
        .queen =
        {
           -1, -2, -1,  2,  0, -6, -5, -4,
            0,  2,  2,  2,  3,  1,  0, -3,
           -2,  1,  3,  0,  3,  3,  6,  4,
            0,  0,  0,  1,  0,  2,  4,  4,
           -1,  0,  0,  0,  0,  3,  1,  0,
           -2,  1,  1,  3,  4,  8,  4,  3,
            0, -3,  3,  3,  1,  1,  6,  6,
           -2,  3,  3,  3,  1,  2,  3,  4
        },
        .king_midgame =
        {
           -2,  7,  2, -6,  4, -6,  4,  1,
            7,  3,  1,  0, -2, -1,  6,  5,
            0,  2,  1, -5, -1,  1,  1, -1,
           -5, -2, -6, -4, -2, -2, -2, -8,
           -9, -6, -8, -6, -8, -5, -4, -6,
           -8, -4, -4, -5, -4, -5, -5, -5,
           -9, -6, -7, -5, -6, -7, -6, -8,
          -10, -8, -8, -8, -7, -9, -9, -9
        },
        .king_endgame =
        {
           -4, -5, -4, -6, -6, -6, -6, -9,
           -6, -2, -5, -5, -4, -1, -2, -3,
           -7, -4, -4, -3, -1,  0,  0, -4,
           -7, -2, -1,  1,  1,  2,  0, -2,
           -4, -1,  2,  2,  2,  3,  2, -2,
           -2,  0,  2,  2,  2,  2,  2, -2,
           -2,  0,  1,  0,  0,  2,  0, -2,
          -10, -4, -2, -2, -2, -2, -2,-10
        },
    },
    .mobility = {
        .knight  = { -9, -7, -2,  0,  1,  2,  2,  4,  4 },
        .bishop  = { -7, -6, -5, -3, -1,  0,  1,  2,  2,  2,  2,  2,  2,  2 },
        .rook_o  = { -6, -3, -3, -3, -3, -2, -2, -1,  0,  2,  2,  2,  3,  2,  2 },
        .rook_e  = { -6, -5, -2,  1,  4,  4,  6,  7,  8,  9, 10, 10, 10, 10, 10 },
        .queen_o = { -5, -5, -5, -5, -2, -3, -3, -3, -3, -2, -2, -1,  1,  1,  2,  3,  3,  3,  3,  5,  5,  6,  6,  6,  7,  7,  7,  7 },
        .queen_e = {-13,-13,-12,-10,-10, -9, -7, -4,  3,  3,  5,  7,  8,  8,  9, 10, 10, 10, 10, 14, 14, 14, 15, 15, 16, 16, 18, 18 },
    },
    .threat = {
        .pawn    = {  0,  1,  1,  1,  1 },
        .knight  = {  0,  0,  2,  5,  5 },
        .bishop  = {  0,  0,  0,  5,  5 },
        .rook    = {  0,  1,  0, -2,  5 },
    },
    .pressure = {
        .knight  = {  0,  3,  5,  3,  3,  0,  0,  0 },
        .bishop  = {  0,  0,  2,  1,  1,  0,  2,  1 },
        .rook    = {  0,  7,  5,  4,  4,  1,  1,  2 },
        .queen   = {  0, 15, 14,  6,  2,  1,  0,  0 },
        .scaling_midgame= {  0, 10, 26, 39, 55 },
    },
    .positional = {
        .pawn_guards_minor             = 1,
        .pawn_guards_pawn              = 1,
        .pawn_shield_1                 = 4,
        .pawn_shield_2                 = 1,
        .pawn_passed_o                 = 11,
        .pawn_passed_e                 = 10,
        .pawn_passed_dist_kings_diff_e = 7,
        .pawn_passed_dist_own_king_e   = 1,
        .pawn_passed_unblocked         = 10,
        .pawn_passed_unreachable_e     = 19,
        .pawn_isolated_o               = -3,
        .pawn_isolated_e               = -3,
        .knight_reduction              = 2,
        .rook_open_file_o              = 6,
        .rook_open_file_e              = 1,
        .rook_halfopen_file_o          = 4,
        .rook_halfopen_file_e          = 3,
        .rook_rearmost_pawn_o          = -1,
        .rook_rearmost_pawn_e          = 2,
        .tempo                         = 1,
        .pawn_passed_scaling = {  0, 13, 28, 63, 115, 177, 255,  0 },
    }
};

static const short sign[2] = { 1, -1 };

/* Game progress: 256 = opening, 0 = endgame */
static int EVAL_game_progress(short material[2])
{
    const int m_hi = 2 * (QUEEN_VALUE + 2 * ROOK_VALUE + BISHOP_VALUE);
    const int m_lo = 2 * (2 * KNIGHT_VALUE);
    const int sum_material = material[WHITE] + material[BLACK];
    if(sum_material > m_hi) {
        return 256;
    } else if(sum_material < m_lo) {
        return 0;
    } else {
        return 256 * (sum_material - m_lo) / (m_hi - m_lo);
    }
}

void EVAL_pawn_types(const chess_state_t *s, bitboard_t attack[NUM_COLORS], bitboard_t *passedPawns, bitboard_t *isolatedPawns)
{
    bitboard_t pawns[2];
    bitboard_t frontFill[2];
    bitboard_t attackSpan[2];
    bitboard_t attackFrontFill[2];
    bitboard_t attackFileFill[2];

    /* Location of pawns */
    pawns[WHITE] = s->bitboard[WHITE_PIECES+PAWN];
    pawns[BLACK] = s->bitboard[BLACK_PIECES+PAWN];

    /* Squares attacked by pawns */
    attack[WHITE] = ((pawns[WHITE] & ~(BITBOARD_FILE<<0)) << 7) | ((pawns[WHITE] & ~(BITBOARD_FILE<<7)) << 9);
    attack[BLACK] = ((pawns[BLACK] & ~(BITBOARD_FILE<<0)) >> 9) | ((pawns[BLACK] & ~(BITBOARD_FILE<<7)) >> 7);

    /* Front-fill */
    frontFill[WHITE] = BITBOARD_fill_north(pawns[WHITE]);
    frontFill[BLACK] = BITBOARD_fill_south(pawns[BLACK]);

    /* Attack-span */
    attackSpan[WHITE] = BITBOARD_fill_north(attack[WHITE]);
    attackSpan[BLACK] = BITBOARD_fill_south(attack[BLACK]);

    /* Combined front-fill and attack-span */
    attackFrontFill[WHITE] = frontFill[WHITE] | attackSpan[WHITE];
    attackFrontFill[BLACK] = frontFill[BLACK] | attackSpan[BLACK];

    /* Fill in both directions */
    attackFileFill[WHITE] = BITBOARD_fill_south(attackSpan[WHITE]);
    attackFileFill[BLACK] = BITBOARD_fill_north(attackSpan[BLACK]);

    /* Passed pawns */
    *passedPawns = (pawns[WHITE] & ~attackFrontFill[BLACK]) |
                   (pawns[BLACK] & ~attackFrontFill[WHITE]);

    /* Isolated pawns */
    *isolatedPawns = (pawns[WHITE] & ~attackFileFill[WHITE]) |
                     (pawns[BLACK] & ~attackFileFill[BLACK]);
}

static short EVAL_pawn_shield(const chess_state_t *s)
{
    const bitboard_t white_queenside = 0x000000000000000E;
    const bitboard_t white_kingside  = 0x00000000000000E0;
    const bitboard_t black_queenside = 0x0E00000000000000;
    const bitboard_t black_kingside  = 0xE000000000000000;
    short score = 0;

    if(s->bitboard[WHITE_PIECES+KING] & white_queenside) {
        score += BITBOARD_count_bits((white_queenside <<  8) & s->bitboard[WHITE_PIECES+PAWN]) * param.positional.pawn_shield_1;
        score += BITBOARD_count_bits((white_queenside << 16) & s->bitboard[WHITE_PIECES+PAWN]) * param.positional.pawn_shield_2;
    } else if(s->bitboard[WHITE_PIECES+KING] & white_kingside) {
        score += BITBOARD_count_bits((white_kingside <<  8) & s->bitboard[WHITE_PIECES+PAWN]) * param.positional.pawn_shield_1;
        score += BITBOARD_count_bits((white_kingside << 16) & s->bitboard[WHITE_PIECES+PAWN]) * param.positional.pawn_shield_2;
    }

    if(s->bitboard[BLACK_PIECES+KING] & black_queenside) {
        score -= BITBOARD_count_bits((black_queenside >>  8) & s->bitboard[BLACK_PIECES+PAWN]) * param.positional.pawn_shield_1;
        score -= BITBOARD_count_bits((black_queenside >> 16) & s->bitboard[BLACK_PIECES+PAWN]) * param.positional.pawn_shield_2;
    } else if(s->bitboard[BLACK_PIECES+KING] & black_kingside) {
        score -= BITBOARD_count_bits((black_kingside >>  8) & s->bitboard[BLACK_PIECES+PAWN]) * param.positional.pawn_shield_1;
        score -= BITBOARD_count_bits((black_kingside >> 16) & s->bitboard[BLACK_PIECES+PAWN]) * param.positional.pawn_shield_2;
    }

    return score;
}

short EVAL_evaluate_board(const chess_state_t *s)
{
    short pawn_material_score[NUM_COLORS] = { 0, 0 };
    short material_score[NUM_COLORS]      = { 0, 0 };
    short positional_score[NUM_COLORS]    = { 0, 0 };
    short positional_score_o[NUM_COLORS]  = { 0, 0 };
    short positional_score_e[NUM_COLORS]  = { 0, 0 };
    bitboard_t pieces;
    bitboard_t pos_bitboard;

    if(EVAL_draw(s)) return 0;

    int rearmost_pawn[NUM_COLORS];
    rearmost_pawn[WHITE] = s->bitboard[WHITE_PIECES+PAWN] ? BITBOARD_GET_RANK(BITBOARD_find_bit(s->bitboard[WHITE_PIECES+PAWN])) : -1;
    rearmost_pawn[BLACK] = s->bitboard[BLACK_PIECES+PAWN] ? BITBOARD_GET_RANK(BITBOARD_find_bit_reversed(s->bitboard[BLACK_PIECES+PAWN])) : -1;

    bitboard_t pawnAttacks[2], passedPawns, isolatedPawns;
    EVAL_pawn_types(s, pawnAttacks, &passedPawns, &isolatedPawns);

    /* Kings */
    int king_pos[NUM_COLORS];
    bitboard_t king_zone[NUM_COLORS];
    king_pos[WHITE] = BITBOARD_find_bit(s->bitboard[WHITE_PIECES + KING]);
    king_pos[BLACK] = BITBOARD_find_bit(s->bitboard[BLACK_PIECES + KING]);
    king_zone[WHITE] = bitboard_king[king_pos[WHITE]];
    king_zone[WHITE] |= king_zone[WHITE] << 8;
    king_zone[BLACK] = bitboard_king[king_pos[BLACK]];
    king_zone[BLACK] |= king_zone[BLACK] >> 8;

    for(int color = WHITE; color <= BLACK; color++) {
        int pos_mask = color * 0x38;
        const bitboard_t *own = &s->bitboard[NUM_TYPES*color];
        const bitboard_t *opp = &s->bitboard[NUM_TYPES*(color^1)];
        bitboard_t own_pieces = own[ALL];
        bitboard_t opp_pieces = opp[ALL];
        bitboard_t diagonal_sliders = own[BISHOP] | own[QUEEN];
        bitboard_t straight_sliders = own[ROOK] | own[QUEEN];
        int num_king_attackers = 0;
        int king_pressure = 0;

        /* Pawns */
        pieces = own[PAWN];
        while(pieces) {
            int rank;
            int pos = BITBOARD_find_bit(pieces);
            pos_bitboard = BITBOARD_POSITION(pos);
            rank = BITBOARD_GET_RANK(pos^pos_mask);
            pawn_material_score[color] += PAWN_VALUE;
            positional_score[color] += param.psq.pawn[pos^pos_mask];
            positional_score_o[color] += (pos_bitboard & pawnAttacks[color]) ? param.positional.pawn_guards_pawn : 0; /* Guarded by other pawn */
            for(int type = PAWN; type <= QUEEN; type++) {
                positional_score[color] += param.threat.pawn[type] * BITBOARD_count_bits(pawnAttacks[color] & opp[type]);
            }

            /* Passed pawn */
            if(pos_bitboard & passedPawns) {
                /* Initial bonus for passed pawn */
                short bonus_o = (short)param.positional.pawn_passed_o;
                short bonus_e = (short)param.positional.pawn_passed_e;

                /* Distance to kings */
                int dist_own_king = distance[king_pos[color]][pos];
                int dist_opp_king = distance[king_pos[color^1]][pos];
                bonus_e += (dist_opp_king - dist_own_king) * param.positional.pawn_passed_dist_kings_diff_e;
                bonus_e += dist_own_king * param.positional.pawn_passed_dist_own_king_e;

                /* Unblocked? */
                if((bitboard_pawn_move[color][pos] & s->bitboard[OCCUPIED]) == 0) {
                    bonus_e += param.positional.pawn_passed_unblocked;

                    /* Unreachable by opponent king? */
                    int dist_prom = 7 - rank;
                    int prom_pos = (pos^pos_mask) + dist_prom * 8;
                    int dist_prom_opp_king = distance[king_pos[color^1]^pos_mask][prom_pos] - (color != s->player);
                    bonus_e += param.positional.pawn_passed_unreachable_e * (dist_prom < dist_prom_opp_king);
                }

                /* Scale bonus with rank */
                int scale_factor = param.positional.pawn_passed_scaling[rank];
                positional_score_o[color] += (short)((int)bonus_o * scale_factor >> 8);
                positional_score_e[color] += (short)((int)bonus_e * scale_factor >> 8);
            }

            /* Isolated pawn */
            if(pos_bitboard & isolatedPawns) {
                positional_score_o[color] += param.positional.pawn_isolated_o;
                positional_score_e[color] += param.positional.pawn_isolated_e;
            }

            pieces ^= pos_bitboard;
        }

        /* Knights */
        pieces = own[KNIGHT];
        int num_opp_pawns = BITBOARD_count_bits(opp[PAWN]);
        while(pieces) {
            int pos = BITBOARD_find_bit(pieces);
            pos_bitboard = BITBOARD_POSITION(pos);
            material_score[color] += KNIGHT_VALUE - (8 - num_opp_pawns) * param.positional.knight_reduction;
            positional_score[color] += param.psq.knight[pos^pos_mask];
            bitboard_t mobility_moves = bitboard_knight[pos] & ~(own_pieces | pawnAttacks[color^1]);
            int piece_mobility = BITBOARD_count_bits(mobility_moves);
            positional_score[color] += param.mobility.knight[piece_mobility];
            positional_score_o[color] += (pos_bitboard & pawnAttacks[color]) ? param.positional.pawn_guards_minor : 0; /* Guarded by pawn */
            for(int type = PAWN; type <= QUEEN; type++) {
                positional_score[color] += param.threat.knight[type] * BITBOARD_count_bits(bitboard_knight[pos] & opp[type]);
            }
            if(mobility_moves & king_zone[color^1]) {
                king_pressure += param.pressure.knight[(int)distance[king_pos[color^1]][pos]];
                ++num_king_attackers;
            }
            pieces ^= pos_bitboard;
        }

        /* Bishops */
        pieces = own[BISHOP];
        if((pieces & BITBOARD_BLACK_SQ) && (pieces & BITBOARD_WHITE_SQ)) {
            /* Bishops on white/black squares => pair bonus */
            material_score[color] += BISHOP_PAIR;
        }
        while(pieces) {
            int pos = BITBOARD_find_bit(pieces);
            pos_bitboard = BITBOARD_POSITION(pos);
            material_score[color] += BISHOP_VALUE;
            positional_score[color] += param.psq.bishop[pos^pos_mask];
            bitboard_t moves, captures;
            MOVEGEN_bishop(pos, own_pieces & ~diagonal_sliders, opp_pieces, &moves, &captures);
            bitboard_t mobility_moves = (moves | captures) & ~pawnAttacks[color^1];
            int piece_mobility = BITBOARD_count_bits(mobility_moves);
            positional_score[color] += param.mobility.bishop[piece_mobility];
            positional_score_o[color] += (pos_bitboard & pawnAttacks[color]) ? param.positional.pawn_guards_minor : 0; /* Guarded by pawn */
            for(int type = PAWN; type <= QUEEN; type++) {
                positional_score[color] += param.threat.bishop[type] * BITBOARD_count_bits(captures & opp[type]);
            }
            if(mobility_moves & king_zone[color^1]) {
                king_pressure += param.pressure.bishop[(int)distance[king_pos[color^1]][pos]];
                ++num_king_attackers;
            }
            pieces ^= pos_bitboard;
        }

        /* Rooks */
        pieces = own[ROOK];
        while(pieces) {
            bitboard_t file;
            int pos = BITBOARD_find_bit(pieces);
            material_score[color] += ROOK_VALUE;
            positional_score[color] += param.psq.rook[pos^pos_mask];
            bitboard_t moves, captures;
            MOVEGEN_rook(pos, own_pieces & ~straight_sliders, opp_pieces, &moves, &captures);
            bitboard_t mobility_moves = (moves | captures) & ~pawnAttacks[color^1];
            int piece_mobility = BITBOARD_count_bits(mobility_moves);
            positional_score_o[color] += param.mobility.rook_o[piece_mobility];
            positional_score_e[color] += param.mobility.rook_e[piece_mobility];
            for(int type = PAWN; type <= QUEEN; type++) {
                positional_score[color] += param.threat.rook[type] * BITBOARD_count_bits(captures & opp[type]);
            }
            if(mobility_moves & king_zone[color^1]) {
                king_pressure += param.pressure.rook[(int)distance[king_pos[color^1]][pos]];
                ++num_king_attackers;
            }

            /* Check for (half-)open files */
            file = (BITBOARD_FILE << BITBOARD_GET_FILE(pos));
            if((file & own[PAWN]) == 0) {
                if((file & opp[PAWN]) == 0) {
                    /* Open file */
                    positional_score_o[color] += param.positional.rook_open_file_o;
                    positional_score_e[color] += param.positional.rook_open_file_e;
                } else {
                    /* Half-open file */
                    positional_score_o[color] += param.positional.rook_halfopen_file_o;
                    positional_score_e[color] += param.positional.rook_halfopen_file_e;
                }
            }

            /* Rooks on the rank of the opponents rearmost pawns */
            if(BITBOARD_GET_RANK(pos) == rearmost_pawn[color^1]) {
                positional_score_o[color] += param.positional.rook_rearmost_pawn_o;
                positional_score_e[color] += param.positional.rook_rearmost_pawn_e;
            }

            pieces ^= BITBOARD_POSITION(pos);
        }

        /* Queens */
        pieces = own[QUEEN];
        while(pieces) {
            bitboard_t moves_b, captures_b, moves_r, captures_r;
            int pos = BITBOARD_find_bit(pieces);
            material_score[color] += QUEEN_VALUE;
            positional_score[color] += param.psq.queen[pos^pos_mask];
            MOVEGEN_bishop(pos, own_pieces & ~diagonal_sliders, opp_pieces, &moves_b, &captures_b);
            MOVEGEN_rook(pos, own_pieces & ~straight_sliders, opp_pieces, &moves_r, &captures_r);
            bitboard_t mobility_moves = (moves_b | captures_b | moves_r | captures_r) & ~pawnAttacks[color^1];
            int piece_mobility = BITBOARD_count_bits(mobility_moves);
            positional_score_o[color] += param.mobility.queen_o[piece_mobility];
            positional_score_e[color] += param.mobility.queen_e[piece_mobility];
            if(mobility_moves & king_zone[color^1]) {
                king_pressure += param.pressure.queen[(int)distance[king_pos[color^1]][pos]];
                ++num_king_attackers;
            }
            pieces ^= BITBOARD_POSITION(pos);
        }

        /* King */
        positional_score_o[color] += param.psq.king_midgame[king_pos[color]^pos_mask];
        positional_score_e[color] += param.psq.king_endgame[king_pos[color]^pos_mask];

        if(num_king_attackers > 4) num_king_attackers = 4;
        positional_score_o[color] += king_pressure * param.pressure.scaling_midgame[num_king_attackers] >> 4;
    }

    short score = 0;
    score += pawn_material_score[WHITE] - pawn_material_score[BLACK];
    score += material_score[WHITE] - material_score[BLACK];
    score += positional_score[WHITE] - positional_score[BLACK];

    /* Pawn shield */
    positional_score_o[WHITE] += EVAL_pawn_shield(s);

    /* Add positional scores weighted by the progress of the game */
    int game_progress = EVAL_game_progress(material_score);
    score += (game_progress * (positional_score_o[WHITE] - positional_score_o[BLACK]) +
             (256 - game_progress) * (positional_score_e[WHITE] - positional_score_e[BLACK])) / 256;

    /* Invert score for black player */
    score *= sign[(int)(s->player)];

    /* Add a bonus for the side with the right to move next */
    score += param.positional.tempo;

    return score;
}

int EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos)
{
    const int player = color;
    const int own_index = player * NUM_TYPES;
    const int opponent_index = NUM_TYPES - own_index;

    bitboard_t attackers;

    /* Is attacked by pawns */
    attackers = bitboard_pawn_capture[player][pos] & s->bitboard[opponent_index + PAWN];
    if(attackers) {
        return 1;
    }

    /* Is attacked by knights */
    attackers = bitboard_knight[pos] & s->bitboard[opponent_index + KNIGHT];
    if(attackers) {
        return 1;
    }

    /* Is attacked by sliders (bishop, rook, queen)? */
    bitboard_t occupied = s->bitboard[OCCUPIED] ^ s->bitboard[own_index + KING];
    attackers = (bitboard_bishop[pos] & (s->bitboard[opponent_index + BISHOP] | s->bitboard[opponent_index + QUEEN])) |
                (bitboard_rook[pos]   & (s->bitboard[opponent_index + ROOK]   | s->bitboard[opponent_index + QUEEN]));
    while(attackers) {
        int attack_pos = BITBOARD_find_bit(attackers);
        if((bitboard_between[attack_pos][pos] & occupied) == 0) {
            return 1;
        }
        attackers ^= BITBOARD_POSITION(attack_pos);
    }

    /* Is attacked by king */
    attackers = bitboard_king[pos] & s->bitboard[opponent_index + KING];
    if(attackers) {
        return 1;
    }

    return 0;
}

/* Returns non-zero if draw may be claimed due to insufficient material or fifty move rule */
int EVAL_draw(const chess_state_t *s)
{
    int num_pieces = BITBOARD_count_bits(s->bitboard[OCCUPIED]);

    switch(num_pieces){
    case 2:
        /* Only kings left */
        return 1;

    case 3:
        /* KN-K or KB-K */
        if(s->bitboard[WHITE_PIECES+KNIGHT] | s->bitboard[WHITE_PIECES+BISHOP] | s->bitboard[BLACK_PIECES+KNIGHT] | s->bitboard[BLACK_PIECES+BISHOP]) {
            return 1;
        }
        break;

    case 4:
        /* KB-KB (same color) */
        if(s->bitboard[WHITE_PIECES+BISHOP] && s->bitboard[BLACK_PIECES+BISHOP]) {
            bitboard_t bishops = s->bitboard[WHITE_PIECES+BISHOP] | s->bitboard[BLACK_PIECES+BISHOP];
            if(((bishops & BITBOARD_WHITE_SQ) == 0) || ((bishops & BITBOARD_BLACK_SQ) == 0)) {
                return 1;
            }
        }
        break;
    }

    return (s->halfmove_clock >= 100);
}
