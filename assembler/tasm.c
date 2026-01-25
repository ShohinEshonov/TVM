#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lexer.h"
#include "dynamic_array.h"




bool check_extension(char *filename, char *extension)
{
    char *dot = strrchr(filename, '.');
    if(!dot) return false;
    return strcmp(dot, extension) == 0;
}

bool file_exist(char *filename)
{
    FILE *file =fopen(filename, "r");
    if(file){

        fclose(file);
        return true;
    }
    return false;
    
}

char *read_file(char* filename)
{
    FILE *file = fopen(filename, "rb");
    if(file == NULL)
    {
        perror("Cannot open file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if(size < 0)
    {
        fprintf(stderr, "Error: cannot determine file size.\n");
        fclose(file);
        return NULL;
    }

    char *buffer = malloc(size+1);
    if(buffer == NULL){
        fprintf(stderr,"Error: cannot allocate memory for file (%ld bytes).\n", size);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, size, file);
    if(bytes_read != (size_t) size)
    {
        fprintf(stderr, "Error: couldn't read entire file.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    buffer[size] = '\0';
    fclose(file);
    return buffer;
}



int main(int argc,char *argv[]){
    char *input_file = NULL;
    char *output_file = NULL;
    
    if(argc < 2)
    {
        fprintf(stderr, "Usage: tasm <input file.tasm> -o <name of output.bin>.");
        return 1;
    }
    input_file = argv[1];
    if(!file_exist(input_file))
    {
        fprintf(stderr, "Error: provided file doesnt exist.\n");
        return 1;                
    }
    if(!check_extension(input_file, ".tasm"))
    {
        fprintf(stderr,"Error: provided file doesnt have .tasm extension.\n");
        return 1;
    }
    
    for(int i = 2; i < argc; i++)
    {
        if(strcmp(argv[i], "-o") == 0)
        {
            if(i+1 < argc)
            {
                output_file = argv[i+1];
                i++;
            }else{
                fprintf(stderr,"Error: you need to provide output file name after -o.\n");
                return 1; 
            }
        }
        else{
            fprintf(stderr, "Error: unknown argument: '%s'\n", argv[i]);
            return 1;
        }
    }
    if(output_file == NULL)
    {
        output_file = "output.bin";
        printf("You not provided output file name,output.bin will be generated.\n");
    }

    printf("Input: %s\n", input_file);
    printf("Output: %s\n", output_file);

    char *source_code = read_file(input_file);
    if(source_code == NULL)
    {
        return 1;
    }

    Lexer *lexer = init_lexer(source_code);
    DynamicArray tokens = scan_tokens(lexer);

    for(size_t i = 0; i < tokens.length; i++)
    {
        Token *t =(Token*)get_element(&tokens, i);
        printf("Token %zu: type=%d", i, t->type);
    
        switch(t->type) {
            case TOKEN_INT:
                printf(", int_value=%ld", t->value.int_value);
                break;
            case TOKEN_FLOAT:
                printf(", float_value=%f", t->value.float_value);
                break;
            case TOKEN_INSTRUCTION:
                printf(", instruction='%s'", t->value.string_value);
                break;
            case TOKEN_EOF:
                printf(" (EOF)");
                break;
        }
        printf("\n");
    }
    
    
    free(source_code);
    free_token_array(&tokens);
    return 0;
}





