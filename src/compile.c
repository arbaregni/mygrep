#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "regex.h"
#include "util.h"

// A new node, with no transitions, that is non-accepting
Node* make_node(Regex* regex) {
    // create the node
    Node* node = malloc(sizeof(Node));
    node->id = regex->num_nodes; // this number is updated next
    node->edges = NULL;
    node->num_edges = 0;
    node->cap_edges = 0;
    node->accepts = false;
    node->beg_capts = CAPT_NONE;
    node->end_capts = CAPT_NONE;

    // push it onto the array of all nodes
    if (regex->num_nodes >= regex->cap) {
        int new_cap = 2 * regex->cap;
        if (new_cap == 0) {
            new_cap = 1;
        }
        Node** new_data = realloc(regex->nodes, sizeof(Node*) * new_cap);
        if (new_data == NULL) {
            fprintf(stderr, "ERROR: out of memory\n");
            exit(EXIT_FAILURE);
        }
        regex->nodes = new_data;
        regex->cap = new_cap;
    }
    regex->nodes[regex->num_nodes] = node;
    regex->num_nodes += 1;

    return node;
}

void destroy_node(Node* node) {
    // we don't own those pointers, Regex does
    for (size_t i = 0; i < node->num_edges; ++i) {
        destroy_pat(&node->edges[i].pat);
    }
    free(node->edges);
    free(node);
}

void destroy_regex(const Regex* regex) {
    for (size_t i = 0; i < regex->num_nodes; ++i) {
        destroy_node(regex->nodes[i]);
    }
    free(regex->nodes);
}

// Return the flag for a new capture group
CaptureFlags new_group(Regex* regex) {
    if (regex->num_groups >= 64) {
        fprintf(stderr, "ERROR: more than 64 capture groups not supported\n");
        exit(EXIT_FAILURE);
    }
    CaptureFlags result = (1 << regex->num_groups);
    ++regex->num_groups;
    return result;
}

// Create an edge from `node` to `target`
// If the current state is `node`, then we may transition to `target` by consuming `pat`,
// and capturing that character to the groups represented by `capt_by`
void add_transition(Node* node, Node* target, Pattern pat) {
    if (node->num_edges >= node->cap_edges) {
        int new_cap = 2 * node->cap_edges;
        if (new_cap == 0) {
            new_cap = 1;
        }
        Edge* new_edges = realloc(node->edges, sizeof(Edge) * new_cap);
        if (!new_edges) {
            fprintf(stderr, "ERROR: out of memory\n");
            exit(EXIT_FAILURE);
        }
        node->edges = new_edges;
        node->cap_edges = new_cap;
    }
    node->edges[node->num_edges].target = target;
    node->edges[node->num_edges].pat = pat;
    node->num_edges += 1;
}

bool compile_nodes(Regex* regex, Node* initial, Node** final, const char** str);

bool compile_capture_group(Regex* regex, Node* initial, Node** final, const char** str) {
    const char* begin = *str;
    const char* end;
    for (end = begin; *end && *end != ')'; ++end);
    if (*end != ')') {
        fprintf(stderr, "ERROR: unclosed ( ) capturing group. expected closing `)`, found %s\n",
                *end? end : "end of input");
        return false;
    }
    *str = end + 1;
    Repition rep;
    if (!parse_repition(&rep, str)) {
        return false;
    }

    // create a new capturing group, and update the flags so that all the nodes we create pipe output to it
    CaptureFlags this_grp = new_group(regex);
    
    // Note about the compilation of a group: mostly mirrors the patterns used for the single-node variant,
    // except we call `compile_nodes` to create the sub expressions,
    // and we must join them by empty links in order to break the separate captures
    Node* curr = initial;
    int i = 0;
    for (; i < rep.lower_bound; ++i) {
        Node* next;
        const char* _s = begin;
        curr->beg_capts |= this_grp;
        // append another copy of the group
        if (!compile_nodes(regex, curr, &next, &_s)) {
            return false;
        }
        next->end_capts |= this_grp;
        curr = next;
    }
    if (rep.is_unbounded) {
        Node* next;
        const char* _s = begin;
        curr->beg_capts |= this_grp;
        // append a copy of the group
        if (!compile_nodes(regex, curr, &next, &_s)) {
            return false;
        }
        next->end_capts |= this_grp;
        Node* final = make_node(regex);
        // its possible to go back and repeat this section
        add_transition(next, curr, EMPTY_PATTERN);
        // or we can stop repeating any time
        add_transition(next, final, EMPTY_PATTERN);
        // we can also skip over it entirely
        add_transition(curr, final, EMPTY_PATTERN);
        curr = final;
    } else {
        for (; i < rep.upper_bound; ++i) {
            Node* next;
            const char* _s = begin;
            curr->beg_capts |= this_grp;
            // append another copy of the group
            if (!compile_nodes(regex, curr, &next, &_s)) {
                return false;
            }
            next->end_capts |= this_grp;
            // we could also just skip over the group
            add_transition(curr, next, EMPTY_PATTERN);
            curr = next;
        }
    }

    *final = curr;
    return true;
}

// Given an initial node, append a chain of nodes after that one, based on the current state of `*str`
// Overwrites `*final` to be the last node we create,
// and `*str` to noe character after the last consumed one
//
// Stops parsing at end of string or a parenthesis
bool compile_nodes(Regex* regex, Node* initial, Node** final, const char** str) {
    Node* curr = initial;
    while (**str != '\0' && **str != ')') {
        if (**str == '$' && *(*str + 1) == '\0') {
            break;
        }
        if (**str == '(') {
            ++*str;
            Node* next;
            if (!compile_capture_group(regex, curr, &next, str)) {
                return false;
            }
            curr = next;
            continue;
        }

        Pattern pat;
        Repition rep;
        if (!parse_pattern(&pat, str)) {
            return false;
        }
        if (!parse_repition(&rep, str)) {
            return false;
        }
        // a straightforward chain of required nodes
        // if we have `A{3}`:
        //
        //  +---+      +---+      +---+      +---+
        //  |   | ---> |   | ---> |   | ---> |   |
        //  +---+  A   +---+  A   +---+  A   +---+
        //   ^                                 ^
        //   |                                 |
        //   initial                          *final
        //  
        int i = 0;
        for (; i < rep.lower_bound; ++i) {
            Node* next = make_node(regex);
            add_transition(curr, next, pat);
            curr = next;
        }
        if (rep.is_unbounded) {
            // we want to loop back around any time we like by matching another
            // if we have `A*`:
            //
            //   +---+
            //   |   | A
            //   v   |
            //  +---+         +---+  
            //  |   | ------> |   | 
            //  +---+  empty  +---+  
            //   ^           ^                      ^
            //   |           |     
            //   initial     *final 
            //  
            // we can keep going back to `curr` as many times as we like if we match `pat`
            Node* next = make_node(regex);
            add_transition(curr, curr, pat);
            // or we can stop any time
            add_transition(curr, next, EMPTY_PATTERN);
        } else {
            // a straightforward chain of required nodes, except we can skip any link if we want
            // if we have `A{0,3}`:
            //
            //        empty        empty         empty
            //  +---+ -----> +---+ -----> +---+ -----> +---+
            //  |   |        |   |        |   |        |   |
            //  +---+ -----> +---+ -----> +---+ -----> +---+
            //   ^      A            A            A      ^
            //   |                                       |
            //   initial                              *final
            //  
            for (; i < rep.upper_bound; ++i) {
                Node* next = make_node(regex);
                // we have a choice: we can match another 'pat'
                add_transition(curr, next, pat);
                // but we don't have to, instead we can bypass it
                add_transition(curr, next, EMPTY_PATTERN);
                curr = next;
            }
        }
    }
    *final = curr;
    return true;
}
// Parses regex until we hit a closing paren or final '$' anchor, or null byte
// `*str` is avanced to the last unconsumed byte (which will be one of those 3)
bool compile(Regex* regex, const char* str) {
#ifdef DEBUG
    printf("compiling `%s`...\n", str);
#endif
    regex->nodes = NULL;
    regex->num_nodes = 0;
    regex->cap = 0;
    regex->num_groups = 0;
    
    regex->trap = make_node(regex);
    add_transition(regex->trap, regex->trap, PATTERN_ANY);

    regex->initial = make_node(regex);
    CaptureFlags group0 = new_group(regex);
    regex->initial->beg_capts |= group0;

    if (*str == '^') {
        ++str;
        // must match from beginning of line
    } else {
        // we can accept any amount of input before the match
        add_transition(regex->initial, regex->initial, PATTERN_ANY);
    }

    Node* final;
    const char* advance_to = str;
    if (!compile_nodes(regex, regex->initial, &final, &advance_to)) {
        return false;
    }
    final->accepts = true;
    final->end_capts |= group0;

    if (*advance_to == '$') {
        ++advance_to;
        // any extra input causes us to reject
        add_transition(final, regex->trap, PATTERN_ANY);
    } else {
        // we can stay at the match forever
        add_transition(final, final, PATTERN_ANY);
    }

    if (*advance_to != '\0') {
        fprintf(stderr, "ERROR: unanticipated extra characters after parsing was finished: `%s`\n.       Was there an unclosed `)`?\n", advance_to);
        return false;
    }

    return true;
}
