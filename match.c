#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "regex.h"
#include "pattern.h"

typedef struct {
    Edge** edges;
    size_t len;
    size_t cap;
} Path;

void push_edge(Path* path, Edge* e) {
    if (path->len >= path->cap) {
        size_t new_cap = 2 * path->cap;
        if (new_cap == 0) {
            new_cap = 1;
        }
        Edge** new_edges = realloc(path->edges, sizeof(Edge*) * new_cap);
        if (!new_edges) {
            fprintf(stderr, "ERROR: out of memory when realloc'ing `path->edges` from %ld to %ld\n", path->cap, new_cap);
            exit(EXIT_FAILURE);
        }
        path->edges = new_edges;
        path->cap = new_cap;
    }
    path->edges[path->len] = e;
    ++path->len;
}

Edge* pop_edge(Path* path) {
    if (path->len <= 0) {
        return NULL;
    }
    --path->len;
    return path->edges[path->len];
}

// modifies captures to include all the capture groups if it successfully matches
bool search_from(const Node* node, const char* input, Path* path) {
    if (*input == '\0') {
        // it's over, now we see if we landed on an accepting node
        return node->accepts;
    }
    // try each possible path from this point
    for (size_t i = 0; i < node->num_edges; ++i) {
        Edge* e = &node->edges[i];
        Pattern* pat = &e->pat;
        if (!pattern_matches(pat, *input)) {
            continue;
        }
        push_edge(path, e);
        size_t skip = pat_size(pat);
        if (search_from(e->target, input + skip, path)) {
            // we found the end!
            return true;
        }
        pop_edge(path);
    }
    // no match possible
    return false;
}

StrView* captures_from_path(const Path* path, const char* input, size_t* match_count, CaptureFlags grp) {
    *match_count = 0;
    for (size_t i = 0; i < path->len; ++i) {
        Edge* e = path->edges[i];
        if (e->target->beg_capts & grp) {
            ++*match_count;
        }
    }
    StrView* captures = malloc(*match_count * sizeof(StrView));
    size_t capt_idx = 0;

    enum State { LOOKING_FOR_BEG, LOOKING_FOR_END };
    enum State state = LOOKING_FOR_BEG;
    const char* beg;

    for (size_t i = 0; i < path->len; ++i) {
        Edge* e = path->edges[i];
        if (state == LOOKING_FOR_BEG && (e->target->beg_capts & grp)) {
            beg = input;
            state = LOOKING_FOR_END;
        }
        // no `else if`: we want to find captures that might take place over a single node
        if (state == LOOKING_FOR_END && (e->target->end_capts & grp)) {
            captures[capt_idx].beg = beg;
            captures[capt_idx].len = (input + 1) - beg;
            ++capt_idx;
            state = LOOKING_FOR_BEG;
        }
        input += pat_size(&e->pat);
    }

    return captures;
}

bool is_match(const Regex* regex, const char* input, Captures* captures) {
    Path path;
    path.edges = NULL;
    path.len = 0;
    path.cap = 0;
    
    Edge dummy_edge;
    dummy_edge.pat = EMPTY_PATTERN;
    dummy_edge.target = regex->initial;

    push_edge(&path, &dummy_edge);

    bool success = search_from(regex->initial, input, &path);
    if (!success) {
        return false;
    }
    // now construct the capture from the path we took
    captures->group_capts = malloc(regex->num_groups * sizeof(StrView*));
    captures->num_capts   = malloc(regex->num_groups * sizeof(size_t));
    captures->num_groups  = regex->num_groups;

    for (size_t group_idx = 0; group_idx < regex->num_groups; ++group_idx) {
        size_t num;
        captures->group_capts[group_idx] = captures_from_path(&path, input, &num, 1 << group_idx);
        captures->num_capts[group_idx] = num;
    }

    free(path.edges);

    return success;
}

StrView* get_capts(const Captures* captures, size_t group_idx, size_t* match_count) {
    *match_count = captures->num_capts[group_idx];
    return captures->group_capts[group_idx];
}
