#ifndef __util_h__
#define __util_h__

int min(int a, int b);

// dynamically allocate a copy of `str`
char* make_copy(const char* str);

// dynamically allocate a copy the portion of the string between
// `match_begin` (inclusive) and `match_end` (exclusive)
char* copy_between(const char* begin, const char* end);

// replace first instance of a newline with a null byte
// returns a pointer to that instance, or NULL if none exist
char* trim_newline(char* str);


#endif
