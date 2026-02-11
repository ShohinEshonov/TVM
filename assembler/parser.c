#include "../instructions.h"
#include "lexer.h"
#include "dynamic_array.h"
#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


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


static void parse_instruction(Token *token, Parser *parser)
{
    const Instr_def *instr_def = instr_def_by_name(token->value.string_value);
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
                Instruction instr = {.type = instr_def->type, .operand.i64 = operand->value.int_value};
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
                Instruction instr = {.type = instr_def->type, .operand.f64 = operand->value.float_value};
                add_element(&parser->program, &instr);
            }
        }
    }else{
        Instruction instr = {.type = instr_def->type};
        add_element(&parser->program, &instr);
    }
}



static void parse_token(Parser *parser)
{

    Token *t = next_token(parser);


    switch(t->type)
    {
        case(TOKEN_INSTRUCTION):
        
            parse_instruction(t, parser);
            break;  
        
        case(TOKEN_EOF):
        
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



