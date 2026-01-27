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
  UNEXPECTED_CHAR,
  UNKNOWN_IDENTIFIER,
  INVALID_NUMBER,
  UNEXPECTED_EOF,
}ErrorType;



typedef struct{
  char *start;
  size_t length;
}LexemeView;



typedef struct{
  ErrorType type;
  int column;
  int line;
  LexemeView lexeme;
}Error;




typedef struct {
    char *source;
    DynamicArray tokens;
    DynamicArray Error;
    int start;
    int current;
    int line;
}Lexer;

Lexer *init_lexer(char *source);
DynamicArray scan_tokens(Lexer *lexer);
void free_token_array(DynamicArray *arr);

#endif
