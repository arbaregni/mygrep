#ifndef __regex_h__
#define __regex_h__

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "pattern.h"
#include "repition.h"
#include "str_view.h"

// use the bits inside a 64-bit integer to represent a set capture groups indices
// i.e. (1 << n) captures only group `n`,
// and (cap_flags1 | cap_flags2) captures `cap_flags1` union `cap_flags2`
typedef uint64_t CaptureFlags;

// 0 captures no groups since all bit fields are set to 0
#define CAPT_NONE 0

typedef struct Node_s Node;

// Represents transition to `target`, by consuming 0 or 1 characters that match `pat`
typedef struct {
    Pattern pat;
    Node* target;
} Edge;

// a node/state in the NFA
struct Node_s {
    size_t id;
    // dynamically allocated array of out-edges
    Edge* edges;
    size_t num_edges;
    size_t cap_edges;
    // whether or not this accepts the input string if we stop here
    bool accepts;
    CaptureFlags beg_capts;
    CaptureFlags end_capts;
};

// Prints a debug report to stdout
void debug_node(const Node* node);

// Frees the memory associated with the node.
// Also free's the pointer itself
void destroy_node(Node* node);

typedef struct {
    // the node to start at
    Node* initial;
    // a common node to trap ireedemable failures
    Node* trap;
    // dynamically allocated array of Node pointers
    Node** nodes;
    size_t num_nodes;
    size_t cap;
    // how many capturing groups we have
    size_t num_groups;
} Regex;


// Attempts to compile `regex` from the input string `str`
// Returns true if this was successful.
// Otherwise, returns false and prints a message to stderr
bool compile(Regex* regex, const char* str);

// Prints a debug report to stdout
void debug_regex(const Regex* regex);

typedef struct {
    // The first char in the string view
    const char* beg;
    // How many characters we view
    size_t len;
} StrView;

typedef struct {
    // matches[i] is all of the matches for group i
    StrView** group_capts;
    size_t* num_capts;
    size_t num_groups;
} Captures;

// Matches `regex` against `str`.
// Returns true if the entire string matched
bool is_match(const Regex* regex, const char* str, Captures* captures);

StrView* get_capts(const Captures* captures, size_t group_idx, size_t* num_capts);

// Free the memory alloc'd by `regex`
void destroy_regex(const Regex* regex);

#endif
