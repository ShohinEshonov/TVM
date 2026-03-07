#include "../isa/isa_table.h"
#include "../bytecode/bytecode_format.h"
#include "lexer.h"
#include "dynamic_array.h"
#include "token_types.h"
#include "parser.h"
#include <features.h>
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


static Symbol* find_lexeme_table(DynamicArray *symbol_table, char *lexeme)
{
  for (size_t i = 0; i < symbol_table->length; i++)
  {
     Symbol *symbol = (Symbol*) get_element(symbol_table, i);
     char* lexeme_table = strndup(symbol->token->lexeme.start, symbol->token->lexeme.length);
     if(strcmp(lexeme, lexeme_table) == 0)
     {
       free(lexeme_table);
       return symbol;
     }
     free(lexeme_table);
  }
  return NULL;
}






static bool is_lexeme_in_table(Parser* parser, char* lexeme)
{
  for (size_t i = 0; i < parser->symbol_table.length; i++)
  {
     Symbol *symbol = (Symbol*) get_element(&parser->symbol_table, i);
     char* lexeme_table = strndup(symbol->token->lexeme.start, symbol->token->lexeme.length);
     if(strcmp(lexeme, lexeme_table) == 0)
     {
         free(lexeme_table);
         return true;
     }
     free(lexeme_table);
  }
  return false;
}

static Token *next_token(Parser *parser)
{
    Token *t = get_element(&parser->tokens, parser->current);
    parser->current++;
    return t;
}

static TokenType peek(Parser *parser)
{
    Token *token = (Token *) get_element(&parser->tokens, parser->current);
    return token->type;
}


static void eat_token(Parser *parser)
{
    parser->current++;
}


static Symbol* parser_identifier(Token* t, Parser* parser)
{
    char *lexeme_identifier = strndup(t->lexeme.start, t->lexeme.length);
    if(is_lexeme_in_table(parser, lexeme_identifier))
    {
        Symbol* symbol = find_lexeme_table(&parser->symbol_table, lexeme_identifier);
        free(lexeme_identifier);
        return symbol;  
    }else
    {
        fprintf(stderr, "Undefined idnetifier: %s", lexeme_identifier);
        free(lexeme_identifier);
        exit(1);
    }
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
            TokenType next = peek(parser);
            if(next == TOKEN_IDENTIFIER)
            {
                Token* identifier = next_token(parser);
                Symbol* symbol_identifier = parser_identifier(identifier, parser);
                uint64_t operand = symbol_identifier->location;                             
                Instruction instr = {.type = instruction_opcode, .operand.i64 = operand};
                parser->program[parser->lc] = instr;
            }else if(next == TOKEN_INT)
            {
                Token *operand = next_token(parser);
                char *operand_str = strndup(operand->lexeme.start, operand->lexeme.length);
                Instruction instr = {.type = instruction_opcode, .operand.i64 = atoll(operand_str)};
                parser->program[parser->lc] = instr;
                free(operand_str);
            }else
            {
                fprintf(stderr, "Excepted int but get unexpected token\n");
                exit(1);
            }
            
        }else if(instr_def->operand_type == OPERAND_FLOAT){
            TokenType next = peek(parser);
            if(next != TOKEN_FLOAT)
            {
                fprintf(stderr,"Excpected float but get unexpected token\n");
                exit(1);
            }
            else
            {
                Token *operand = next_token(parser);
                char *operand_str = strndup(operand->lexeme.start, operand->lexeme.length);
                Instruction instr = {.type = instruction_opcode, .operand.f64 = atof(operand_str)};
                parser->program[parser->lc] = instr;
                free(operand_str);
            }
        }
    }else{
        Instruction instr = {.type = instruction_opcode};
        parser->program[parser->lc] = instr;
    }
    free(instruction_str);
}


static void parse_label(Token *t ,Parser *parser)
{
    char* symbol_lexeme = strndup(t->lexeme.start, t->lexeme.length);
    if(is_lexeme_in_table(parser, symbol_lexeme))
    {
        fprintf(stderr, "Second define of 1 label %s", symbol_lexeme);
        free(symbol_lexeme);
        exit(1);
    }else
    {
        Symbol sym = {.token = t,.location = parser->lc,.symbol_type = SYMBOL_LABEL};
        add_element(&parser->symbol_table, &sym);
    }
    free(symbol_lexeme);
}





static void parse_token(Parser *parser)
{

    Token *t = next_token(parser);

    switch(t->type)
    {
        case(TOKEN_MNEMONIC):     
            parse_mnemonic(t, parser);
            parser->lc++;
            break;  

        case(TOKEN_IDENTIFIER):
            if(peek(parser) == TOKEN_COLON)
            {
                eat_token(parser);
            }
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


void first_pass(Parser *parser)
{
    while(!is_at_end(parser))
    {
        Token *t = next_token(parser);

        if(t->type == TOKEN_IDENTIFIER)
        {
            if(peek(parser) == TOKEN_COLON)
            {
                parse_label(t, parser);
                eat_token(parser);
            }else{
                add_element(&parser->unsolved_refs, t); 
            }
        }else if(t->type == TOKEN_MNEMONIC)
        {
            parser->lc++;
        }
    }
}

void solve_refs(Parser *parser)
{
    for(size_t i = 0; i < parser->unsolved_refs.length; i++)
    {
        Token *ref = (Token*)get_element(&parser->unsolved_refs, i); 
        char* name = strndup(ref->lexeme.start, ref->lexeme.length);

        if(!is_lexeme_in_table(parser, name))
        {
            fprintf(stderr, "Undefined identifier %s", name);
            free(name);
            exit(1);
        }
        free(name);
    }
}

void parse_tokens(Parser *parser)
{
    if(parser->first_pass == false)
    {
        first_pass(parser);
        solve_refs(parser);
        parser->first_pass = true;
        parser->current = 0;    
    }
    parser->program = malloc(parser->lc * sizeof(Instruction));
    if(parser->program == NULL)
    {
        perror("Cannot allocate memory for parser");
        exit(1);
    }
    parser->lc = 0;
    while(!is_at_end(parser))
    {
        parse_token(parser);
    }
    free_array(&parser->symbol_table);
    free_array(&parser->unsolved_refs); 
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
    // init_array(&parser->program, sizeof(Instruction), 16);
    init_array(&parser->symbol_table, sizeof(Symbol), 5);
    init_array(&parser->unsolved_refs, sizeof(Token), 16);
    parser->program = NULL;
    parser->current = 0;
    parser->lc = 0;
    parser->first_pass = false;
    return parser;    
}



