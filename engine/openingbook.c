#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "openingbook.h"

typedef struct _openingbook_node_t {
    char                        pos_from;
    char                        pos_to;
    char                        num_subnodes;
    struct _openingbook_node_t  **subnodes;
} openingbook_node_t;

struct _openingbook_t {
    openingbook_node_t  *tree;
    openingbook_node_t  *current_node;
};

static openingbook_node_t *OPENINGBOOK_read_node(FILE *f);
static openingbook_node_t *OPENINGBOOK_read(const char *filename);
static void OPENINGBOOK_free_node(openingbook_node_t *n);

openingbook_t *OPENINGBOOK_create(const char *filename)
{
    /* Allocate memory for opening book */
    openingbook_t *o = malloc(sizeof(openingbook_t));
    
    /* Read opening book file */
    o->tree = OPENINGBOOK_read(filename);
    OPENINGBOOK_reset(o);
    
    /* Seed random number generator */
    srand(time(NULL));
    
    return o;
}

void OPENINGBOOK_destroy(openingbook_t *o)
{
    /* Free all memory consumed by the openingbook */
    OPENINGBOOK_free_node(o->tree);
    free(o);
}

void OPENINGBOOK_reset(openingbook_t *o)
{
    /* The current node is the root node */
    o->current_node = o->tree;
}

move_t OPENINGBOOK_get_move(openingbook_t *o, chess_state_t *s)
{
    move_t move = 0;
    
    if(o->current_node) {
        if(o->current_node->num_subnodes) {
            move_t moves[256];
            int i;
            int index = rand() % o->current_node->num_subnodes;
            int pos_from = o->current_node->subnodes[index]->pos_from;
            int pos_to = o->current_node->subnodes[index]->pos_to;
            
            /* Generate all possible moves and find one with matching pos_from and pos_to */
            int num_moves = STATE_generate_moves(s, moves);
            for(i = 0; i < num_moves; i++) {
                if(MOVE_GET_POS_FROM(moves[i]) == pos_from && MOVE_GET_POS_TO(moves[i]) == pos_to) {
                    move = moves[i];
                    break;
                }
            }
        } else {
            o->current_node = NULL;
        }
    }
    
    return move;
}

void OPENINGBOOK_apply_move(openingbook_t *o, move_t move)
{
    if(o->current_node) {
        if(o->current_node->num_subnodes) {
            int i;
            for(i = 0; i < o->current_node->num_subnodes; i++) {
                if(o->current_node->subnodes[i]->pos_from == MOVE_GET_POS_FROM(move) &&
                    o->current_node->subnodes[i]->pos_to == MOVE_GET_POS_TO(move))
                {
                    /* Update the state of the opening book according to the played move */
                    o->current_node = o->current_node->subnodes[i];
                    return;
                }
            }
        }
    }
    
    /* A position outside the opening book is reached.
     * Do not use the opening book anymore this game */
    o->current_node = NULL;
}

static openingbook_node_t *OPENINGBOOK_read_node(FILE *f)
{
    int i;
    
    /* Allocate memory for the node */
    openingbook_node_t *n = malloc(sizeof(openingbook_node_t));
    
    /* Read positions (from and to) */
    fread(&n->pos_from, sizeof(n->pos_from), 1, f);
    fread(&n->pos_to, sizeof(n->pos_to), 1, f);
    
    /* Read the number of subnodes */
    fread(&n->num_subnodes, sizeof(n->num_subnodes), 1, f);
    
    /* Allocate memory for subnodes array */
    if(n->num_subnodes) {
        n->subnodes = malloc(n->num_subnodes * sizeof(openingbook_node_t*));
        
        /* Recursively read subnodes */
        for(i = 0; i < n->num_subnodes; i++) {
            n->subnodes[i] = OPENINGBOOK_read_node(f);
        }
    } else {
        n->subnodes = NULL;
    }
    
    return n;
}

static openingbook_node_t *OPENINGBOOK_read(const char *filename)
{
    openingbook_node_t *n = NULL;
    
    /* Open file for reading */
    FILE *f = fopen(filename, "r");
    if(f) {
        /* Recursively create tree nodes from the file */
        n = OPENINGBOOK_read_node(f);
        fclose(f);
    }
    return n;
}

static void OPENINGBOOK_free_node(openingbook_node_t *n)
{
    int i;
    
    /* Recursively free all nodes in the tree */
    if(n) {
        for(i = 0; i < n->num_subnodes; i++) {
            OPENINGBOOK_free_node(n->subnodes[i]);
        }
        
        free(n);
    }
}
