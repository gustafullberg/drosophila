#ifndef _HISTORY_H
#define _HISTORY_H

#include "bitboard.h"

typedef struct _history_t history_t;

history_t *HISTORY_create();
void HISTORY_destroy(history_t *h);
void HISTORY_reset(history_t *h);
void HISTORY_push(history_t *h, bitboard_t hash);
void HISTORY_pop(history_t *h);
int HISTORY_is_repetition(history_t *h, int halfmove_clock);

#endif
