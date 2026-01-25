#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include <stdbool.h>
#include <ctype.h>
#include "../instructions.h"
#include "dynamic_array.h"




void free_token_array(DynamicArray *arr)
{
    for(size_t i = 0; i < arr->length; i++)
    {
        Token *t = (Token*)get_element(arr, i);
        if(t->type == TOKEN_INSTRUCTION)
        {
            free(t->value.string_value);
        }
    }
    free_array(arr);
}


Lexer *init_lexer(char *source)
{
    Lexer *lexer = malloc(sizeof(Lexer));
    if(!lexer)
    {
        perror("Cannot allocate lexer");
        exit(1);
    }
    lexer->source = source;
    init_array(&lexer->tokens, sizeof(Token), 16);
    lexer->current = 0;
    lexer->start = 0;
    lexer->line = 1;
    return lexer;
}

static char advance(Lexer *lexer)
{
    return lexer->source[lexer->current++];
}


static bool is_at_end(Lexer *lexer)
{
    return lexer->source[lexer->current] == '\0';
}

static char peek(Lexer *lexer)
{
    if(is_at_end(lexer))
    {
        return '\0';
    }
    return lexer->source[lexer->current];
}



static void scan_number(Lexer *lexer)
{
    TokenType type = TOKEN_INT;
    while(isdigit(peek(lexer)))
    {
        advance(lexer);
    }
    if (peek(lexer) == '.')
    {
        advance(lexer);
        if(!isdigit(peek(lexer)))
        {
            fprintf(stderr, "Error at line %d: Expected number after '.' \n", lexer->line);
            exit(1);
        }
        
        while(isdigit(peek(lexer)))
        {
            advance(lexer);
        }
        type = TOKEN_FLOAT;
    }

    char *num_str = strndup(&lexer->source[lexer->start], lexer->current - lexer->start);
    if(type == TOKEN_INT)
    {
        Token token = {.type = type, .value.int_value = atoll(num_str), .line = lexer->line };
        add_element(&lexer->tokens, &token);
    }else{
        Token token = {.type = type, .value.float_value = atof(num_str), .line = lexer->line };
        add_element(&lexer->tokens, &token);
    }
    free(num_str);
 }

static void scan_instruction(Lexer *lexer)
{
    while(isalpha(peek(lexer)) || peek(lexer) == '_')
    {
        advance(lexer);
    }

    char *instruction_str = strndup(&lexer->source[lexer->start], lexer->current - lexer->start);

    const Instr_def *instr = instr_def_by_name(instruction_str);

    if(instr != NULL)
    {
        Token token = {.type = TOKEN_INSTRUCTION, .value.string_value = instruction_str, .line = lexer->line};
        add_element(&lexer->tokens, &token);
    }else
    {
        fprintf(stderr, "Error at line: %d. Unexpected instruction '%s'.\n", lexer->line, instruction_str);
        free(instruction_str);
        exit(1);
    }
}


static void scan_token(Lexer *lexer)
{
    char c = advance(lexer);
           
    if(isdigit(c)){
        scan_number(lexer);        
    }else if(isalpha(c)){
        scan_instruction(lexer);
    }else if(c == ' ' ||c == '\t'|| c == '\r'){   
    }else if(c == '\n'){
        lexer->line++;
    }else{
        fprintf(stderr, "Error at line %d: Unexpected character", lexer->line);
        exit(1);
    }   
}


DynamicArray scan_tokens(Lexer *lexer)
{
    while(!is_at_end(lexer))
    {
        lexer->start = lexer->current;
        scan_token(lexer);
    }
    Token eof = {.type = TOKEN_EOF, .line = lexer->line};
    add_element(&lexer->tokens, &eof);

    DynamicArray result = lexer->tokens;
    free(lexer);
    return result;
}



