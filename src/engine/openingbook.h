#ifndef _OPENINGBOOK_H
#define _OPENINGBOOK_H

#include "state.h"

struct _openingbook_t;
typedef struct _openingbook_t openingbook_t;

openingbook_t *OPENINGBOOK_create(const char *filename);
void OPENINGBOOK_destroy(openingbook_t *o);
move_t OPENINGBOOK_get_move(const openingbook_t *o, const chess_state_t *s);
uint64_t OPENINGBOOK_get_hash(const chess_state_t *state);

#endif

