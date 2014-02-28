#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "state.h"
#include "san.h"

typedef struct _openingbook_node_t {
    char                        pos_from;
    char                        pos_to;
    char                        num_subnodes;
    struct _openingbook_node_t  **subnodes;
} node;

node *add_move(node *n, move_t move)
{
    int i;
    node *newnode;
    char move_from = MOVE_GET_POS_FROM(move);
    char move_to = MOVE_GET_POS_TO(move);
    
    for(i = 0; i < n->num_subnodes; i++) {
        if(n->subnodes[i]->pos_from == move_from && n->subnodes[i]->pos_to == move_to) {
            return n->subnodes[i];
        }
    }
    
    newnode = (node*)malloc(sizeof(node));
    newnode->pos_from = move_from;
    newnode->pos_to = move_to;
    newnode->num_subnodes = 0;
    newnode->subnodes = 0;
    n->num_subnodes++;
    n->subnodes = (node**)realloc(n->subnodes, n->num_subnodes * sizeof(node));
    n->subnodes[n->num_subnodes-1] = newnode;
    return newnode;
}

void add_line(node *n, char *line)
{
    chess_state_t s;
    move_t move;
    const char *p;
    STATE_reset(&s);
    
    p = strtok(line, " ");
    while(p) {
        move = SAN_parse_move(&s, p);
        if(move == 0) {
            printf("ERROR\n");
            exit(1);
        }
        STATE_apply_move(&s, move);
        n = add_move(n, move);
        p = strtok(NULL, " ");
    }
}

void print_tree(node *n, int depth)
{
    int i, j;
    for(i = 0; i < n->num_subnodes; i++) {
        for(j = 0; j < depth; j++) printf(" ");
        printf("%d %d\n", n->subnodes[i]->pos_from, n->subnodes[i]->pos_to);
        print_tree(n->subnodes[i], depth+1);
    }
}

void export_node(node *n, FILE *f)
{
    int i;
    fwrite(&n->pos_from, sizeof(n->pos_from), 1, f);
    fwrite(&n->pos_to, sizeof(n->pos_to), 1, f);
    fwrite(&n->num_subnodes, sizeof(n->num_subnodes), 1, f);
    
    for(i = 0; i < n->num_subnodes; i++) {
        export_node(n->subnodes[i], f);
    }
}

void export_tree(node *n)
{
    FILE *f = fopen("openingbook.dat", "w");
    export_node(n, f);
    fclose(f);
}

int main(int argc, char **argv)
{
    int i;
    FILE *f;
    unsigned char buffer[1024];
    node *n = (node*)malloc(sizeof(node));
    memset(n, 0, sizeof(node));
    
    BITBOARD_init();
    
    if(argc != 2) {
        fprintf(stderr, "Usage: %s openings.txt\n", argv[0]);
        exit(1);
    }
    
    f = fopen(argv[1], "r");
    if(!f) {
        fprintf(stderr, "Error: could not open file\n");
        exit(2);
    }
    
    while(1) {
        int c;
        for(i = 0; i < 1024; i++) {
            c = fgetc(f);
            if(c == EOF) break;
            if(c == '\n') {
                buffer[i] = '\0';
                break;
            } else {
                buffer[i] = c;
            }
        }
        if(c == EOF) break;
        printf("%s\n", buffer);
        
        add_line(n, (char*)buffer);
    }
    
    export_tree(n);
    
    return 0;
}
