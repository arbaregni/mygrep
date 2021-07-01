#ifndef _repition_h__
#define _repition_h__

#include <stdbool.h>

// Represents how many times a pattern can be repeated
typedef struct {
    // we must match at least this many instances (inclusive)
    unsigned int lower_bound;
    // we may match at most this many instances (inclusive)
    unsigned int upper_bound;
    // if set to true, we ignore the upper bound, and instead match as
    // many as we can
    bool is_unbounded;
} Repition;


// Parse the regex repition from the string pointer, advancing it to just after the repition text
// BEFORE: we start looking at a character in the string
//   apples? they are good
//         ^
//         |
//         *str
// AFTER: *str is advanced because the '?' was matched
//   apples? they are good
//          ^
//          |
//          *str
bool parse_repition(Repition* rep, const char** str);

#endif
