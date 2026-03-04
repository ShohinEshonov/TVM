#include "../isa/isa_table.h"
#include "../bytecode/bytecode_format.h"
#include "lexer.h"
#include "dynamic_array.h"
#include "token_types.h"
#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define ARRAY_SIZE(xs) sizeof(xs) / sizeof(xs[0])




struct {char *mnem ; OP_CODE op_code; } mnemonic_to_isa[] =
{
  {"push_i", OP_PUSH_INT},
  {"push_f", OP_PUSH_FLOAT},
  
  {"pop", OP_POP},
  {"dup", OP_DUP},
  
  {"plus_i", OP_PLUS_INT},
  {"plus_f", OP_PLUS_FLOAT},
  {"minus_i", OP_MINUS_INT},
  {"minus_f", OP_MINUS_FLOAT},
  {"mult_i", OP_MULT_INT},
  {"mult_f", OP_MULT_FLOAT},
  {"div_i", OP_DIV_INT},
  {"div_f", OP_DIV_FLOAT},

  {"jmp", OP_JMP},
  {"jz", OP_JZ},
  {"jnz", OP_JNZ},

  {"lt_i", OP_LT_INT},
  {"lt_f", OP_LT_FLOAT},
  {"gt_i", OP_GT_INT},
  {"gt_f", OP_GT_FLOAT},
  {"eq_i", OP_EQ_INT},
  {"eq_f", OP_EQ_FLOAT},

  {"store", OP_STORE},
  {"load_i", OP_LOAD_INT},
  {"load_f", OP_LOAD_FLOAT},

  {"i2f", OP_INT_TO_FLOAT},
  {"f2i", OP_FLOAT_TO_INT},

  {"halt", OP_HALT},
};


static Token *next_token(Parser *parser)
{
    Token *t = get_element(&parser->tokens, parser->current);
    parser->current++;
    return t;
}

static Token *peek(Parser *parser)
{
    return get_element(&parser->tokens, parser->current);
}


static void parse_mnemonic(Token *token, Parser *parser)
{
    char *instruction_str = strndup(token->lexeme.start, token->lexeme.length);
    size_t instruction_opcode = -1;
    for(size_t i = 0; i < ARRAY_SIZE(mnemonic_to_isa); i++)
    {
        if(strcmp(instruction_str, mnemonic_to_isa[i].mnem) == 0)
        {
            instruction_opcode = mnemonic_to_isa[i].op_code;
            break;
        }
    }
    
    const Instr_def *instr_def = instr_def_by_type(instruction_opcode);
    if(instr_def->has_operand)
    {
        if(instr_def->operand_type == OPERAND_INT)
        {
            Token *next = peek(parser);
            if(next->type != TOKEN_INT)
            {
                fprintf(stderr, "Expected int but get unexpected token\n");
                exit(1);
            }else
            {
                Token *operand = next_token(parser);
                char *operand_str = strndup(operand->lexeme.start, operand->lexeme.length);
                Instruction instr = {.type = instruction_opcode, .operand.i64 = atoll(operand_str)};
                add_element(&parser->program, &instr);
            }
        }else if(instr_def->operand_type == OPERAND_FLOAT){
            Token *next = peek(parser);
            if(next->type != TOKEN_FLOAT)
            {
                fprintf(stderr,"Excpected float but get unexpected token\n");
                exit(1);
            }
            else
            {
                Token *operand = next_token(parser);
                char *operand_str = strndup(operand->lexeme.start, operand->lexeme.length);
                Instruction instr = {.type = instruction_opcode, .operand.f64 = atof(operand_str)};
                add_element(&parser->program, &instr);
            }
        }
    }else{
        Instruction instr = {.type = instruction_opcode};
        add_element(&parser->program, &instr);
    }
    free(instruction_str);
}



static void parse_token(Parser *parser)
{

    Token *t = next_token(parser);


    switch(t->type)
    {
        case(TOKEN_MNEMONIC):
        
            parse_mnemonic(t, parser);
            break;  
        
        case(TOKEN_EOF):
            break;
        case(TOKEN_IDENTIFIER):
            fprintf(stderr, "Unknown identifier\n");
            exit(1);
            break;
        default:
        {
            fprintf(stderr, "Unexpected token\n");
            exit(1);
        }
    }
}



static bool is_at_end(Parser *parser)
{
    Token *t = get_element(&parser->tokens, parser->current);
    return t->type == TOKEN_EOF; 
}





DynamicArray parse_tokens(Parser *parser)
{
    while(!is_at_end(parser))
    {
        parse_token(parser);
    }
    DynamicArray instrs = parser->program;
    free(parser);
    return instrs;
}



Parser *init_parser(DynamicArray *tokens)
{
    Parser *parser = malloc(sizeof(Parser));
    if(!parser)
    {
        perror("Cannot allocate parser");
        exit(1);
    }
    parser->tokens = *tokens;
    init_array(&parser->program, sizeof(Instruction), 16);
    parser->current = 0;
    return parser;    
}



