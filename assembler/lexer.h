#ifndef LEXER_H

#define LEXER_H

#include <stdint.h>
#include <stddef.h>
#include "dynamic_array.h"

typedef enum {
  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_INSTRUCTION,
  TOKEN_EOF,
}TokenType;


typedef struct{
  TokenType type;
  union {
    int64_t int_value;
    double float_value;
    char *string_value;
  } value;
  int line;
}Token;




typedef enum {
  INVALID_CHAR,
  UNKNOWN_IDENTIFIER,
  INVALID_NUMBER,
  UNEXPECTED_EOF,
}ErrorType;


typedef struct{
  ErrorType error;
  int column;
  int line;
  char *lexeme;
}Error;




typedef struct {
    char *source;
    DynamicArray tokens;
    int start;
    int current;
    int line;
}Lexer;

Lexer *init_lexer(char *source);
DynamicArray scan_tokens(Lexer *lexer);
void free_token_array(DynamicArray *arr);

#endif
