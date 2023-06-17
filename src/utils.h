#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>

typedef long long int Int;

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

#define MAX_STRLEN 512

extern int  string_length(const char*);
extern bool string_equal(const char* a, const char* b);
extern void string_copy(char* dest, const char* src);

extern int  get_xdigit_value(int char_digit_literal);
extern bool is_digit(char c);
extern bool is_xdigit(char c);
extern bool is_ident(char c);
extern bool is_ident_start(char c);

extern Int to_number(const char* s);

#endif // UTILS_H
