#include "scanner.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

static int         g_line = 1;
static const char* g_current = 0;
static bool        g_error = false;

static char        g_ch = '\0';
static char        g_identifier[MAX_STRLEN] = {0};
static int         g_number = 0;

struct KeywordEntry {
    TokenKind kind;
    const char* identifier;
};

static struct KeywordEntry
    g_keyword_table[] = {
    { TK_DIV,          "div"       },
    { TK_MOD,          "mod"       },
    { TK_LOGIC_OR,     "or"        },
    { TK_KEY_OF,       "of"        },
    { TK_KEY_THEN,     "then"      },
    { TK_KEY_DO,       "do"        },
    { TK_KEY_END,      "end"       },
    { TK_KEY_ELSE,     "else"      },
    { TK_KEY_ELSEIF,   "elsif"     },
    { TK_KEY_UNTIL,    "until"     },
    { TK_KEY_IF,       "if"        },
    { TK_KEY_WHILE,    "while"     },
    { TK_KEY_REPEAT,   "repeat"    },
    { TK_KEY_ARRAY,    "array"     },
    { TK_KEY_RECORD,   "record"    },
    { TK_KEY_CONST,    "const"     },
    { TK_KEY_TYPE,     "type"      },
    { TK_KEY_VAR,      "var"       },
    { TK_KEY_PROCEDURE,"procedure" },
    { TK_KEY_BEGIN,    "begin"     },
    { TK_KEY_MODULE,   "module"    },
};

inline static char
get_char(void)
{
    char c = *g_current;

    if(c == '\0')
        return c;

    if(c == '\n')
        g_line += 1;

    g_current += 1;
    return c;
}

inline static void
unget_char(void)
{
    g_current -= 1;
    g_ch = *g_current;
}

static TokenKind
scan_identifier(void)
{
    assert(is_ident(g_ch));
    // identifier
    int i = 0;

    do {
        if(i < MAX_STRLEN) {
            g_identifier[i] = g_ch;
            i += 1;
        }

        g_ch = get_char();
    } while(is_ident(g_ch));

    g_identifier[i] = '\0';
    // keyword
    int k = 0;

    while(k < ARRAY_COUNT(g_keyword_table)
          && !string_equal(g_keyword_table[k].identifier, g_identifier)) {
        k += 1;
    }

    if(k < ARRAY_COUNT(g_keyword_table)) {
        return g_keyword_table[k].kind;
    }

    return TK_IDENTIFIER;
}

static TokenKind
scan_decimal_number(void)
{
    assert(is_digit(g_ch));
    long long value = 0;

    do {
        if(value > INT_MIN && value < INT_MAX) {
            value = 10 * value + g_ch - '0';
        } else {
            scanner_mark_error("number too large for int");
            value = 0;
        }

        g_ch = get_char();
    } while(g_ch >= '0' && g_ch <= '9');

    g_number = (int)value;
    return TK_LITERAL_NUMBER;
}

static TokenKind
scan_hex_number(void)
{
    assert(is_xdigit(g_ch));
    long long value = 0;

    do {
        if(value > INT_MIN && value < INT_MAX) {
            value = 16 * value + get_xdigit_value(g_ch);
        } else {
            scanner_mark_error("number too large for int");
            value = 0;
        }

        g_ch = get_char();
    } while(is_xdigit(g_ch));

    g_number = (int)value;
    return TK_LITERAL_NUMBER;
}

static void
skip_comment(void)
{
    assert(g_ch == '*');

    do {
        // nested comments possible!
        do {
            g_ch = get_char();

            while(g_ch == '(') {
                g_ch = get_char();

                if(g_ch == '*')
                    skip_comment();
            }
        } while (g_ch != '*' || g_ch == '\0');

        do {
            g_ch = get_char();
        } while(g_ch == '*' || g_ch == '\0');
    } while(g_ch != ')' || g_ch == '\0');

    if(g_ch != '\0')
        g_ch = get_char();
    else
        scanner_mark_error("comment not terminated");
}

static void
skip_line_comment(void)
{
    assert(g_ch == '/');

    while(g_ch != '\n' && g_ch != '\0')
        g_ch = get_char();
}

inline static void
skip_whitespace(void)
{
    while(true) {
        switch(g_ch) {
        case '\n':
        case ' ':
        case '\r':
        case '\t':
            g_ch = get_char();
            break;

        default:
            return;
        }
    }
}


TokenKind
scanner_get(void)
{
    skip_whitespace();

    if(g_ch == '\0')
        return TK_EOF;

    TokenKind kind = TK_UNKNOWN;

    if(is_ident_start(g_ch))
        kind = scan_identifier();
    else if(is_digit(g_ch))
        kind = scan_decimal_number();
    else {
        switch(g_ch) {
            break;

        case '!':
            g_ch = get_char();
            kind = scan_hex_number();
            break;

        case '&':
            g_ch = get_char();
            kind = TK_LOGIC_AND;
            break;

        case '*':
            g_ch = get_char();
            kind = TK_TIMES;
            break;

        case '+':
            g_ch = get_char();
            kind = TK_PLUS;
            break;

        case '-':
            g_ch = get_char();
            kind = TK_MINUS;
            break;

        case '=':
            g_ch = get_char();
            kind = TK_EQUAL;
            break;

        case '#':
            g_ch = get_char();
            kind = TK_NOT_EQUAL;
            break;

        case ';':
            g_ch = get_char();
            kind = TK_SEMICOLON;
            break;

        case ',':
            g_ch = get_char();
            kind = TK_COMMA;
            break;

        case '.':
            g_ch = get_char();
            kind = TK_PERIOD;
            break;

        case '[':
            g_ch = get_char();
            kind = TK_LEFT_BRACKET;
            break;

        case ']':
            g_ch = get_char();
            kind = TK_RIGHT_BRACKET;
            break;

        case '~':
            g_ch = get_char();
            kind = TK_LOGIC_NOT;
            break;

        case '<':
            g_ch = get_char();

            if(g_ch == '=') {
                kind = TK_LESS_EQUAL;
                g_ch = get_char();
            } else {
                kind = TK_LESS;
            }

            break;

        case '>':
            g_ch = get_char();

            if(g_ch == '=') {
                kind = TK_GREATER_EQUAL;
                g_ch = get_char();
            } else {
                kind = TK_GREATER;
            }

            break;

        case ':':
            g_ch = get_char();

            if(g_ch == '=') {
                kind = TK_ASSIGN;
                g_ch = get_char();
            } else {
                kind = TK_COLON;
            }

            break;

        case ')':
            g_ch = get_char();
            kind = TK_RIGHT_PAREN;
            break;

        case '(':
            g_ch = get_char();

            if(g_ch == '*') {
                skip_comment();
                kind = scanner_get();
            } else {
                kind = TK_LEFT_PAREN;
            }

            break;

        case '/':
            g_ch = get_char();

            if(g_ch == '/') {
                skip_line_comment();
                kind = scanner_get();
            } else {
                unget_char();
            }

            break;

        default:
            break;
        }
    }

    return kind;
}

void
scanner_init(const char* source)
{
    g_current = source;
    g_number = -1;
    g_error = false;
    g_line = 1;
    g_ch = get_char();
}


const char*
scanner_get_identifier(void)
{
    return g_identifier;
}

int
scanner_get_number(void)
{
    int result = g_number;
    assert(result >= 0);
    return result;
}

void
scanner_mark_error(const char* fmt, ...)
{
    va_list arg;
    printf("ERROR at line %d: ", g_line);
    va_start(arg, fmt);
    vprintf(fmt, arg);
    va_end(arg);
    printf("\n");
    g_error = true;
    exit(EXIT_FAILURE);
}

bool
scanner_has_error(void)
{
    return g_error;
}
