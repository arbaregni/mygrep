#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "repition.h"

// parses an integer from `*str`, advancing `**str` to the first unconsumed char
unsigned int parse_int(const char** str) {
    char* endptr;
    unsigned int result = strtol(*str, &endptr, 10); // base 10
    *str = endptr;
    return result;
}


// parses an repition modifier from `*str`, advancing `**str` to the first unconsumed char
bool parse_repition(Repition* rep, const char** str) {
     switch (**str) {
        case '?':
            rep->lower_bound = 0;
            rep->upper_bound = 1;
            rep->is_unbounded = false;
            ++*str;
            break;
        case '*':
            rep->lower_bound = 0;
            rep->upper_bound = -1;
            rep->is_unbounded = true;
            ++*str;
            break;
        case '+':
            rep->lower_bound = 1;
            rep->upper_bound = -1;
            rep->is_unbounded = true;
            ++*str;
            break;
        case '{':
            ++*str;
            // the first number is a lower bound
            rep->lower_bound = parse_int(str);
            if (**str == '}') {
                ++*str;
                rep->upper_bound = rep->lower_bound;
                rep->is_unbounded = false;
            } else if (**str == ',') {
                ++*str;
                if (**str == '}') {
                    ++*str;
                    // there is no upper bound
                    rep->upper_bound = -1;
                    rep->is_unbounded = true;
                } else if (isdigit(**str)) {
                    rep->upper_bound = parse_int(str);
                    rep->is_unbounded = false;
                    if (rep->upper_bound < rep->lower_bound) {
                        fprintf(stderr, "ERROR: regex upper bound (%d) is less than lower bound (%d)\n",
                                rep->upper_bound, rep->lower_bound);
                        return false;
                    }
                    if (**str != '}') {
                        fprintf(stderr, "ERROR in regex { }, found no closing `}`, found: `%s`\n", *str);
                        return false;
                    }
                    ++*str;
                } else {
                    fprintf(stderr, "ERROR: in regex { } repition, expected `}` or number, found: `%c`\n", **str);
                    return false;
                }
            } else {
                fprintf(stderr, "ERROR: in regex { } repition, expected `,` or `}`, found: `%c`\n", **str);
                return false;
            }
            break;
        default:
            // no repition signifier means we expect the pattern exactly once
            // and we don't have to eat anything
            rep->lower_bound = 1;
            rep->upper_bound = 1;
            rep->is_unbounded = false;
            break;
    }
    return true;
}
