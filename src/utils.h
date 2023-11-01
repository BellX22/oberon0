#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>

typedef long long int Int;

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))
#define MAX_STRLEN 512

int  string_length(const char*);
bool string_equal(const char* a, const char* b);
void string_copy(char* dest, const char* src);
bool is_digit(char c);
bool is_xdigit(char c);
bool is_ident(char c);
bool is_ident_start(char c);
int  get_xdigit_value(int char_digit_literal);
Int  to_number(const char* s);

#endif // UTILS_H
