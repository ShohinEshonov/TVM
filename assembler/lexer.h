#ifndef LEXER_H

#define LEXER_H

#include <stdint.h>
#include <stddef.h>
#include "dynamic_array.h"
#include "token_types.h"



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



typedef struct{
  TokenType type;
  LexemeView lexeme;
  int line;
}Token;



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
