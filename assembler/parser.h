#ifndef PARSER_H
#define PARSER_H


#include "dynamic_array.h"
#include "../isa/isa_table.h"
#include "lexer.h"
#include "../bytecode/bytecode_format.h"


typedef enum{
    SYMBOL_LABEL,
}SymbolType;



typedef struct{
    SymbolType symbol_type;
    Token *token;
    int64_t location;
}Symbol;


typedef struct{
    DynamicArray tokens;
    Instruction *program;
    DynamicArray symbol_table;
    DynamicArray unsolved_refs;
    int current;
    int64_t lc;
    bool first_pass;
    
}Parser;


void parse_tokens(Parser *parser);
Parser *init_parser(DynamicArray *tokens);

#endif
