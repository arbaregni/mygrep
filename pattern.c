#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "pattern.h"

bool matches_in_set(const Pattern* sub_pats, int num_sub_pats, int ch) {
    for (int i = 0; i < num_sub_pats; ++i) {
        if (pattern_matches(&sub_pats[i], ch)) {
            return true;
        }
    }
    return false;
}

bool pattern_matches(const Pattern *pattern, char ch) {
    switch (pattern->type) {
        case PAT_EMPTY:      return  true; // but we don't consume anything
        case PAT_LITERAL:    return  pattern->literal == ch;
        case PAT_ANY:        return  true;
        case PAT_ALPHA:      return  isalpha(ch);
        case PAT_NONALPHA:   return !isalpha(ch);
        case PAT_SPACE:      return  isspace(ch);
        case PAT_DIGIT:      return  isdigit(ch);
        case PAT_NONDIGIT:   return !isdigit(ch);
        case PAT_SET:
            return matches_in_set(pattern->sub_pats, pattern->num_sub_pats, ch);
        case PAT_NEG_SET:
            return !matches_in_set(pattern->sub_pats, pattern->num_sub_pats, ch);
    }
    fprintf(stderr, "Unanticipated enum variant of PatternType");
    exit(EXIT_FAILURE);
}

size_t pat_size(const Pattern* pat) {
    if (pat->type == PAT_EMPTY) {
        return 0;
    } else {
        return 1;
    }
}

bool parse_escape_code(Pattern* pat, const char** str) {
    switch (**str) {
        case '\\': case '.': case '^': case '$': case '?': case '*':
        case '{': case '}': case '[': case ']': case '(': case ')':
            // an escaped literal 
            pat->type = PAT_LITERAL;
            pat->literal = **str;
            break;
        case 's':
            // \s is white space
            pat->type = PAT_SPACE;
            break;
        case 'w':
            // \w is alphabetic
            pat->type = PAT_ALPHA;
            break;
        case 'W':
            // \W is nonalphabetic
            pat->type = PAT_NONALPHA;
            break;
        case 'd':
            // \d is a digit
            pat->type = PAT_DIGIT;
            break;
        case 'D':
            // \D is a non digit
            pat->type = PAT_NONDIGIT;
            break;
        default:
            fprintf(stderr, "ERROR: unexpected escape sequence \\%c\n", **str);
            return false;
    }
    ++*str;
    return true;
}

// a matching set like [abc] or [^abc]           
bool parse_pattern_set(Pattern* pat, const char** str) {
    if (**str == '^') {
        ++*str;
        // a negated matching set [^abc]
        pat->type = PAT_NEG_SET;
    } else {
        pat->type = PAT_SET;
    }

    // find the end of this matching set
    const char* end = *str; 
    for (; *end != ']'; ++end) {
        // check that there are no illegal characters
        if (*end == '\0') {
            fprintf(stderr, "ERROR: end of input inside [ ] set\n");
            return false;
        } else if (*end == '[') {
            fprintf(stderr, "ERROR: nested [ ] sets\n");
            return false;
        }
    }
    // [^abcde]
    //   ^    ^
    //   |    |
    //  *str   end
    // there are (end - 1 - *str) characters so *at most* that many patterns
    int cap = end - *str; // note: this might allocate more than we need. oh well
    pat->sub_pats = malloc(cap * sizeof(Pattern));
    // fill the dynamically allocated array with all the patterns
    pat->num_sub_pats = 0;
    while (*str < end) {
        bool success = parse_pattern(&pat->sub_pats[pat->num_sub_pats], str);
        if (!success) {
            return false;
        }
        pat->num_sub_pats += 1;
    }
    // set the string to one character past the last char we consumed
    *str = end + 1;
    return true;
}

bool parse_pattern(Pattern* pat, const char** str) {
    pat->literal = 0; // left this way unless we are a literal expression
    pat->sub_pats = NULL;
    pat->num_sub_pats = 0;
    bool result = false;
    if (**str == '\\') {
        // escape code: inspect the next character
        ++*str;
        result = parse_escape_code(pat, str);
    } else if (**str == '[') {
        ++*str;
        result = parse_pattern_set(pat, str);
    } else if (**str == '.') {
        // match anything
        pat->type = PAT_ANY;
        ++*str;
        result = true;
    } else {
        // just a literal character
        pat->type = PAT_LITERAL;
        pat->literal = **str;
        ++*str;
        result = true;
    }
    return result;
}

void destroy_pat(const Pattern* pat) {
    for (size_t i = 0; i < pat->num_sub_pats; ++i) {
        destroy_pat(&pat->sub_pats[i]);
    }
    free(pat->sub_pats);
}

