#include "utils.h"
#include <assert.h>

int
string_length(const char* s)
{
    int n = 0;

    while(*s) {
        n += 1;
        s += 1;
    }

    return n;
}

bool
string_equal(const char* a, const char* b)
{
    int alen = string_length(a);
    int blen = string_length(b);

    if(alen != blen)
        return false;

    for(int i = 0; i < alen; i++) {
        if(a[i] != b[i])
            return false;
    }

    return true;
}

void
string_copy(char* dest, const char* src)
{
    int len = string_length(src);
    int i = 0;

    for(i = 0; i < len; i++) {
        dest[i] = src[i];
    }

    dest[i] = '\0';
}

Int
to_number(const char* s)
{
    bool negative = false;

    if(*s == '-') {
        negative = true;
        s += 1;
    }

    Int n = 0;

    while(*s) {
        n *= 10;
        n += (*s) - '0';
        s += 1;
    }

    return negative ? -n : n;
}

bool
is_digit(char c)
{
    if ((c >= '0' && c <= '9'))
        return true;

    return false;
}

bool
is_xdigit(char c)
{
    if (is_digit(c))
        return true;

    switch(c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return true;
    }

    return false;
}

bool
is_ident_start(char c)
{
    return
        (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || (c == '_')
        ;
}

bool
is_ident(char c)
{
    return
        (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || (c == '_')
        || is_digit(c)
        ;
}

int
get_xdigit_value(int char_digit_literal)
{
    int c = char_digit_literal;

    if(c >= '0' && c <= '9')
        return c - '0';

    switch(c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
        return c - 'a' + 10;

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return c - 'A' + 10;
    }

    assert(false);
    return 0;
}
