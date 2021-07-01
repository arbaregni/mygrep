#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int min(int a, int b) {
    if (a < b) return a;
    else       return b;
}

char* make_copy(const char* str) {
    int len = strlen(str);
    char* copy = malloc(len + 1); // one extra for the null byte
    strcpy(copy, str);
    return copy;
}

char* copy_between(const char* begin, const char* end) {
    size_t len = end - begin;
    char* copy = malloc(len + 1); // one extra for the null byte
    strncpy(copy, begin, len);
    copy[len] = '\0';
    return copy;
}
char* trim_newline(char* str) {
    for (; *str; ++str) {
        if (*str == '\n' || *str == '\r') {
            *str = '\0';
            return str;
        }
    }
    return NULL;
}

