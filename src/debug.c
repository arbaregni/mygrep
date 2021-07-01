#include "regex.h"
#include "pattern.h"

//
// This file contains a variety of methods intended to be useful for debugging various structs
//

void debug_pat(const Pattern* pat) {
    switch (pat->type) {
        case PAT_EMPTY:
            printf("PAT_EMPTY");
            break;
        case PAT_LITERAL:
            printf("'%c'", pat->literal);
            break;
        case PAT_ANY:
            printf("PAT_ANY");
            break;
        case PAT_SPACE:
            printf("PAT_SPACE");
            break;
        case PAT_ALPHA:
            printf("PAT_ALPHA");
            break;
        case PAT_NONALPHA:
            printf("PAT_NONALPHA");
            break;
        case PAT_DIGIT:
            printf("PAT_DIGIT");
            break;
        case PAT_NONDIGIT:
            printf("PAT_NONDIGIT");
            break;
        case PAT_SET:
            printf("PAT_SET[");
            for (size_t i = 0; i < pat->num_sub_pats; ++i) {
                if (i != 0) {
                    printf(", ");
                }
                debug_pat(&pat->sub_pats[i]);
            }
            printf("]");
            break;
        case PAT_NEG_SET:
            printf("PAT_NEG_SET");
            for (size_t i = 0; i < pat->num_sub_pats; ++i) {
                if (i != 0) {
                    printf(", ");
                }
                debug_pat(&pat->sub_pats[i]);
            }
            printf("]");
            break;
    }
}

void debug_node(const Node* node) {
    printf(" +--\n");
    printf(" | Node %ld (%s):\n", node->id, node->accepts? "accepts" : "rejects");
    printf(" |     %ld edge(s), beg_capts = %ld, end_capts = %ld\n", node->num_edges, node->beg_capts, node->end_capts);
    for (size_t i = 0; i < node->num_edges; ++i) {
        printf(" |     ");
        Edge* e = &node->edges[i];
        debug_pat(&e->pat);
        printf(" -> Node %ld\n", e->target->id);
    }
}

void debug_regex(const Regex* regex) {
    printf("----------------------------------------------------------------------------\n");
    printf("Initial: Node %ld\n", regex->initial->id);
    printf("Trap:    Node %ld\n", regex->trap->id);
    printf("Num Groups: %ld\n", regex->num_groups);
    for (size_t i = 0; i < regex->num_nodes; ++i) {
        debug_node(regex->nodes[i]);
    }
    printf("----------------------------------------------------------------------------\n");
}

