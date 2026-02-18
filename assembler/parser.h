#ifndef PARSER_H
#define PARSER_H


#include "dynamic_array.h"
#include "../instructions.h"

typedef struct{
    DynamicArray tokens;
    DynamicArray program;
    int current;
}Parser;


DynamicArray parse_tokens(Parser *parser);
Parser *init_parser(DynamicArray *tokens);

#endif
