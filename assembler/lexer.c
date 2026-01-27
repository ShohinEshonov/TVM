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


static Error construct_error(ErrorType type, int line,int column, LexemeView *lexeme)
{
    Error err;
    err.type = type;
    err.line = line;
    err.column = column;
    if(lexeme)
    {        
        err.lexeme = *lexeme;
    }else{
        err.lexeme.start = NULL;
        err.lexeme.length = 0;
    }
    return err;
}

static void print_error(const Error *e)
{
    const char *type_str;

    switch (e->type)
    {
        case UNEXPECTED_CHAR:
            type_str = "Unexpected character";
            break;
        case UNEXPECTED_EOF:
            type_str = "Unexpected end of file";
            break;
        default:
            type_str = "Unknown error";
            break;
    }

    fprintf(stderr, "Error: %s at line %d, column %d\n",
            type_str, e->line, e->column);

    if (e->lexeme.start && e->lexeme.length > 0)
    {
        fprintf(stderr, "  %.*s\n", (int)e->lexeme.length, e->lexeme.start);
        fprintf(stderr, "  ");
        for (size_t i = 0; i < e->lexeme.length; i++)
            fputc('^', stderr);
        fprintf(stderr, "\n");
    }
}


static void print_errors(DynamicArray *arr)
{
    for(size_t i = 0; i < arr->length; i++)
    {
        Error *e = (Error*)get_element(arr, i);
	print_error(e);        
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
    init_array(&lexer->Error, sizeof(Error), 5);
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

static void panic_mode(Lexer *lexer)
{
    for(;;)
    {
        char c = peek(lexer);

        if(c == '\n')
        {
	    advance(lexer);
	    lexer->line++;
            break;
        }else if(c == '\0')
	{
	    break;
	}
        advance(lexer);
    }
}



static void scan_number(Lexer *lexer)
{
    TokenType type = TOKEN_INT;
    while(isdigit((unsigned char) peek(lexer)))
    {
        advance(lexer);
    }
    if (peek(lexer) == '.')
    {
        advance(lexer);
        if(!isdigit((unsigned char) peek(lexer)))
        {
            LexemeView lexeme = {.start = &lexer->source[lexer->start], .length = lexer->current - lexer->start};
            Error error = construct_error(UNEXPECTED_CHAR, lexer->line, lexer->current, &lexeme);
            add_element(&lexer->Error,&error);
            panic_mode(lexer);
	    return;
        }
        
        while(isdigit((unsigned char) peek(lexer)))
        {
            advance(lexer);
        }
        type = TOKEN_FLOAT;
    }
    if(isalpha((unsigned char) peek(lexer)))
    {
        LexemeView lexeme = {.start = &lexer->source[lexer->start], .length = lexer->current - lexer->start};
        Error error = construct_error(UNEXPECTED_CHAR, lexer->line, lexer->current, &lexeme);
        add_element(&lexer->Error,&error);
        panic_mode(lexer);
	return;
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
    while(isalpha((unsigned char)peek(lexer)) || peek(lexer) == '_')
    {
	if(is_at_end(lexer))
	{
	    Error err = construct_error(UNEXPECTED_EOF, lexer->line, lexer->current, NULL);
	    add_element(&lexer->Error, &err);
	    return;
	}
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
        LexemeView lexeme = {.start = &lexer->source[lexer->start], .length = lexer->current - lexer->start };
        Error err = construct_error(UNEXPECTED_CHAR, lexer->line, lexer->start, &lexeme );
	add_element(&lexer->Error, &err);
        panic_mode(lexer);
        free(instruction_str);
    }
}


static void scan_token(Lexer *lexer)
{
    char c = advance(lexer);
           
    if(isdigit((unsigned char)c)){
        scan_number(lexer);        
    }else if(isalpha((unsigned char)c)){
        scan_instruction(lexer);
    }else if(c == ' ' ||c == '\t'|| c == '\r'){   
    }else if(c == '\n'){
        lexer->line++;
    }else{
	LexemeView lexeme = {.start = &lexer->source[lexer->start], .length = 1};
	Error err = construct_error(UNEXPECTED_CHAR, lexer->line, lexer->current, &lexeme);
	add_element(&lexer->Error, &err);
	panic_mode(lexer);
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
    print_errors(&lexer->Error);
    free_array(&lexer->Error);
    free(lexer);
    return result;
}



