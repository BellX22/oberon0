#ifndef LEXER_H
#define LEXER_H
#include <stdbool.h>
#ifndef __cplusplus
typedef enum TokenKind TokenKind;
#endif

/*
 * This is the preferred style for multi-line
 * comments in the Linux kernel source code.
 * Please use it consistently.
 *
 * Description:  A column of asterisks on the left side,
 * with beginning and ending almost-blank lines.
 */

enum TokenKind {
	TK_UNKNOWN,       // null
	TK_TIMES,         // times     '*'
	TK_DIV,           // div       'div'
	TK_MOD,           // mod       'mod'
	TK_LOGIC_AND,     // and       '&'
	TK_PLUS,          // plus      '+'
	TK_MINUS,         // minus     '-'
	TK_LOGIC_OR,      // or        'or'
	TK_EQUAL,         // eql       '='
	TK_NOT_EQUAL,     // neq       '#'
	TK_LESS,          // lss       '<'
	TK_LESS_EQUAL,    // leq       '<='
	TK_GREATER,       // gtr       '>'
	TK_GREATER_EQUAL, // geq       '>='
	TK_PERIOD,        // period    '.'
	TK_COMMA,         // comma     ','
	TK_COLON,         // colon     ':'
	TK_RIGHT_PAREN,   // rparen    ')'
	TK_RIGHT_BRACKET, // rbrak     ']'
	TK_KEY_OF,        // of        'of'
	TK_KEY_THEN,      // then      'then'
	TK_KEY_DO,        // do        'do'
	TK_LEFT_PAREN,    // lparen    '('
	TK_LEFT_BRACKET,  // lbrak     '['
	TK_LOGIC_NOT,     // not       '~'
	TK_ASSIGN,        // becomes   ':='
	TK_LITERAL_NUMBER,// number
	TK_IDENTIFIER,    // ident
	TK_SEMICOLON,     // semicolon ';'
	TK_KEY_END,       // end       'end'
	TK_KEY_ELSE,      // eise      'else'
	TK_KEY_ELSEIF,    // elsif     'elsif'
	TK_KEY_UNTIL,     // until     'until'
	TK_KEY_IF,        // if        'if'
	TK_KEY_WHILE,     // while,    'while'
	TK_KEY_REPEAT,    // repeat    'repeat'
	TK_KEY_ARRAY,     // array     'array'
	TK_KEY_RECORD,    // record    'record'
	TK_KEY_CONST,     // const     'const'
	TK_KEY_TYPE,      // type      'type'
	TK_KEY_VAR,       // var       'var'
	TK_KEY_PROCEDURE, // procedure 'procedure'
	TK_KEY_BEGIN,     // begin     'begin'
	TK_KEY_MODULE,    // module    'module'
	TK_EOF,
	TK_COUNT
};

// comment start '(*'
// commment end  '*)'

void        scanner_init(const char *source);
TokenKind   scanner_get(void);
int         scanner_get_number(void);
const char *scanner_get_identifier(void);
void        scanner_mark_error(const char *fmt, ...);
bool        scanner_has_error(void);

#endif // LEXER_H
