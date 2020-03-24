#include <stdio.h>
#include <stdlib.h>
#include "openingbook.h"
#include "clock.h"

typedef struct _openingbook_node_t {
    uint64_t    hash;
    uint16_t    move;
} openingbook_node_t;

struct _openingbook_t {
    openingbook_node_t  *nodes;
    int                 num_nodes;
};

static void OPENINGBOOK_read_node(FILE *f, openingbook_node_t *node);
static int OPENINGBOOK_find_node(const openingbook_t *o, uint64_t hash, int *first_node_index);
static move_t OPENINGBOOK_translate_move(const chess_state_t *s, uint16_t m);

openingbook_t *OPENINGBOOK_create(const char *filename)
{
    int i;

    /* Allocate memory for opening book */
    openingbook_t *o = (openingbook_t*)calloc(1, sizeof(openingbook_t));
    
    /* Open file for reading */
    FILE *f = fopen(filename, "rb");
    if(f) {
        /* Go to end of file */
        fseek(f, 0, SEEK_END);

        /* Get number of elements from position at end of file */
        o->num_nodes = (int)(ftell(f) / 16);

        /* Go to beginning of file */
        rewind(f);

        /* Allocate memory for nodes */
        o->nodes = (openingbook_node_t*)malloc(o->num_nodes * sizeof(openingbook_node_t));

        /* Read nodes */
        for(i = 0; i < o->num_nodes; i++) {
            OPENINGBOOK_read_node(f, &o->nodes[i]);
        }

        fclose(f);
    }


    /* Seed random number generator */
    srand(CLOCK_random_seed());
    
    return o;
}

void OPENINGBOOK_destroy(openingbook_t *o)
{
    /* Free all memory consumed by the openingbook */
    if(o) {
        if(o->nodes) free(o->nodes);
        free(o);
    }
}

move_t OPENINGBOOK_get_move(const openingbook_t *o, const chess_state_t *s)
{
    move_t move = 0;
    int first_node_index;
    int num_nodes;

    /* Find matching nodes */
    num_nodes = OPENINGBOOK_find_node(o, s->hash, &first_node_index);

    if(num_nodes) {
        /* Select one random node */
        int offset = rand() % num_nodes;

        /* Translate it to engines move syntax */
        move = OPENINGBOOK_translate_move(s, o->nodes[first_node_index+offset].move);
    }

    return move;
}

static void OPENINGBOOK_read_node(FILE *f, openingbook_node_t *node)
{
    uint8_t buffer[16];
    int i;

    /* Read the 16 bytes that contain an entry */
    fread(buffer, 16, 1, f);

    /* Parse hash */
    node->hash = 0;
    for(i = 0; i < 8; i++) {
        node->hash <<= 8;
        node->hash |= buffer[i];
    }

    /* Parse move */
    node->move = 0;
    for(i = 8; i < 10; i++) {
        node->move <<= 8;
        node->move |= buffer[i];
    }
}

static int OPENINGBOOK_find_node(const openingbook_t *o, uint64_t hash, int *first_node_index)
{
    int index[2];
    int guess;
    int num_nodes = 0;

    /* Binary search for ONE matching node */
    index[0] = 0;
    index[1] = o->num_nodes - 1;
    *first_node_index = -1;

    while(index[0] <= index[1]) {
        guess = (index[0] + index[1]) >> 1;
        if(o->nodes[guess].hash < hash)         index[0] = guess + 1;
        else if(o->nodes[guess].hash > hash)    index[1] = guess - 1;
        else if(o->nodes[guess].hash == hash)   break;
    }

    if(index[0] > index[1]) {
        /* Hash not found */
        return num_nodes;
    }

    /* Find FIRST matching node */
    num_nodes = 1;
    while(guess > 0 && o->nodes[guess-1].hash == hash) {
        guess--;
        num_nodes++;
    }
    *first_node_index = guess;

    /* Find NUMBER of matching nodes */
    guess += num_nodes;
    while(guess < o->num_nodes && o->nodes[guess].hash == hash) {
        guess++;
        num_nodes++;
    }

    return num_nodes;
}

static move_t OPENINGBOOK_translate_move(const chess_state_t *s, uint16_t m)
{
    int i;
    int num_moves;
    move_t moves[256];
    int pos_to = m & 0x3F;
    int pos_from = (m >> 6) & 0x3F;
    int promotion_type = (m >> 12) & 7;

    /* Generate all possible moves */
    num_moves = STATE_generate_moves_simple(s, moves);

    /* Loop through all generated moves to find the right one */
    for(i = 0; i < num_moves; i++) {
        int move = moves[i];

        /* Position from has to match */
        if(MOVE_GET_POS_FROM(move) != pos_from) continue;

        /* Promotion type has to match */
        if(MOVE_PROMOTION_TYPE(move) != promotion_type) continue;

        /* Position to has to match... */
        if(MOVE_GET_POS_TO(move) == pos_to) {
            return move;
        }

        /* ...unless it is castling */
        if(MOVE_GET_TYPE(move) == KING) {
            if((MOVE_GET_POS_TO(move) == pos_from + 2 && pos_to == pos_from + 3) /* King side */
                || (MOVE_GET_POS_TO(move) == pos_from - 2 && pos_to == pos_from - 4)) /* Queen side */
            {
                return move;
            }
        }
    }

    /* No valid move found */
    return 0;
}
