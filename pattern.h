#ifndef __pattern_h__
#define __pattern_h__

#include <stdio.h>
#include <stdbool.h>

// Represent what type of pattern we are matching
enum PatternType {
    // nonconsuming, empty match
    PAT_EMPTY,
    // Match a literal character (specified by the other field in the RegexPattern struct)
    PAT_LITERAL,
    // Match any character (.)
    PAT_ANY,
    // Match whitespace (\s)
    PAT_SPACE,
    // Match alphabetic (\w)
    PAT_ALPHA,
    // Match any non alphabetic (\W)
    PAT_NONALPHA,
    // Match digit (\d)
    PAT_DIGIT,
    // Match any non digit
    PAT_NONDIGIT,
    // Match any in the set of patterns
    PAT_SET,
    // Matches anything not in the set of patterns
    PAT_NEG_SET,
};

typedef struct Pattern_s Pattern;

struct Pattern_s {
    // the type of pattern we are 
    enum PatternType type;
    // If we are a PAT_LTIERAl type, then what literal we expect. otherwise, 0
    char literal;
    // If we are a PAT_SET or PAT_NEG_SET type, then the set of subpatterns we could match, otherwise NULL
    Pattern* sub_pats;
    size_t num_sub_pats;
};

static const struct Pattern_s EMPTY_PATTERN = { PAT_EMPTY, '\0', NULL };
static const struct Pattern_s PATTERN_ANY = { PAT_ANY, '\0', NULL };

// Parse the regex pattern from the string pointer, advancing it to just after the repition text
// BEFORE: we start looking at a character in the string
//     give me \d+ apple(s)
//             ^
//             |
//             *str
// AFTER: *str is advanced because the "\d" was matched
//     give me \d+ apple(s)
//               ^
//               |
//               *str
bool parse_pattern(Pattern* pat, const char** str);

// returns the size of a match by `pat` in characters:
size_t pat_size(const Pattern* pat);

// print a debuging report
void debug_pat(const Pattern* pat);

// Returns true if `pattern` and `literal` match `ch`
// For example, '.' will match anything and 'a' will match the literal 'a'
bool pattern_matches(const Pattern *pattern, char ch);

void destroy_pat(const Pattern* pat);

#endif
