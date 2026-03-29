#ifndef PARSER_H
#define PARSER_H


#include "dynamic_array.h"
#include "../isa/isa_table.h"
#include "lexer.h"


#define MAX_OPERANDS 3





typedef enum
{
    VAL_I64,
    VAL_U64,
    VAL_U8,
    VAL_U16,
    VAL_F64,
}ValType;


typedef struct
{
    ValType type;
    union{
        int64_t i64;
        uint64_t u64;
        uint8_t u8;
        uint16_t u16;
        double f64;
    };
}Value;



typedef struct
{
    OP_CODE opcode;
    int operand_count;
    Value operands[MAX_OPERANDS];
}Instruction;




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
    uint64_t ic;
    uint64_t bc; //its needed for jumps to jump to byte addresss
    bool first_pass;
}Parser;


void parse_tokens(Parser *parser);
Parser *init_parser(DynamicArray *tokens);

#endif
