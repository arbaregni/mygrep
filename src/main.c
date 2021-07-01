#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "regex.h"
#include "util.h"

#define MAX_LINE_SIZE 1024

void match_lines(const Regex* regex, FILE* file, bool trim_to_match, bool print_captures) {
    // put a null byte before the beginning of the line to help with the anchor testing
    char buf[MAX_LINE_SIZE + 1];
    buf[0] = '\0';
    char* line = buf + 1;

    while (1) {
        char* ret = fgets(line, MAX_LINE_SIZE, file);
        if (!ret) {
            break; // stop reading
        }
        trim_newline(line);

        Captures captures;
        if (!is_match(regex, line, &captures)) {
            continue;
        }
        if (trim_to_match) {
            size_t _num;
            StrView s = get_capts(&captures, 0, &_num)[0]; // capture group 0 is the whole regex
            printf("%.*s\n", (int)s.len, s.beg);
        } else {
            printf("%s\n", line);
        }
        if (print_captures) {
            for (size_t group_idx = 1; group_idx < captures.num_groups; ++group_idx) {
                size_t num;
                StrView* capts = get_capts(&captures, group_idx, &num);
                printf("    [%ld]", group_idx);
                for (size_t capt_idx = 0; capt_idx < num; ++capt_idx) {
                    StrView s = capts[capt_idx];
                    printf(" %.*s", (int)s.len, s.beg);
                }
                printf("\n");
            }
        }
    }

}

int main(int argc, char** argv) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        printf("HELP:\n");
        printf("USAGE: a.out <regex> [options] <input-file1> [ <input-file2> ... ]\n");
        printf("OPTIONS: -t, --trim reports only matched portion, instead of entire line\n");
        printf("         -c, --print-captures prints the capture ( ) groups\n");
        return EXIT_SUCCESS;
    }
    if (argc < 3) {
        fprintf(stderr, "ERROR: Invalid arguments\n");
        fprintf(stderr, "USAGE: a.out <regex> [options] <input-file1> [ <input-file2> ... ]\n");
        fprintf(stderr, "USAGE: a.out --help\n");
        return EXIT_FAILURE;
    }
    ++argv; // eat argv[0]
    Regex regex;
    compile(&regex, *argv);
    ++argv;

#ifdef DEBUG
    debug_regex(&regex);
#endif
    
    bool trim_to_match = false;
    bool print_captures = false;
    for (; *argv; ++argv) {
        if (**argv != '-') {
            break;
        }
        if (  strcmp(*argv, "-t") == 0
           || strcmp(*argv, "--trim") == 0)
        {
            trim_to_match = true;
        }
        if (  strcmp(*argv, "-c") == 0
           || strcmp(*argv, "--print-captures") == 0)
        {
            print_captures = true;
        }
    }
    int success = EXIT_SUCCESS; // set to EXIT_FAILURE if any problems occured
   
    for (; *argv; ++argv) {
        FILE* file = fopen(*argv, "r");
        if (!file) {
            fprintf(stderr, "ERROR: Can not open input file `%s` to read, skipping...\n", *argv);
            success = EXIT_FAILURE;
            continue;
        }
        match_lines(&regex, file, trim_to_match, print_captures);
    }

    destroy_regex(&regex);

    return success;
}
