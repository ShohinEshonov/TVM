#ifndef PARSER_H
#define PARSER_H


#include "dynamic_array.h"
#include "../instructions.h"

typedef union {
  int64_t i64;
  double f64;
}Operand;

typedef struct {
  Instr_type type;
  Operand operand;
} Instruction;


typedef struct{
    DynamicArray tokens;
    DynamicArray program;
    int current;
}Parser;


DynamicArray parse_tokens(Parser *parser);
Parser *init_parser(DynamicArray *tokens);

#endif
