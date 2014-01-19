#ifndef _OPENINGBOOK_H
#define _OPENINGBOOK_H

#include "state.h"

struct _openingbook_t;
typedef struct _openingbook_t openingbook_t;

openingbook_t *OPENINGBOOK_create(const char *filename);
void OPENINGBOOK_destroy(openingbook_t *o);
void OPENINGBOOK_reset(openingbook_t *o);
move_t OPENINGBOOK_get_move(openingbook_t *o, chess_state_t *s);
void OPENINGBOOK_apply_move(openingbook_t *o, move_t move);

#endif

