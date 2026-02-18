#include "format.h"
#include "instructions.h"
#include "tvm.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>




bool check_extension(char *filename, char *extension) {
  char *dot = strrchr(filename, '.');
  if (!dot)
    return false;
  return strcmp(dot, extension) == 0;
}


Header read_header(FILE *file)
{
    Header h;
    fread(h.magic,1, MAGIC_SIZE, file);
    fread(&h.prog_size, 1, sizeof(uint16_t), file);

    if(memcmp(h.magic,MAGIC ,MAGIC_SIZE) != 0)
    {
      fprintf(stderr, "Invalid format file.");
      exit(1);
    }
    return h;
}


FILE * open_file(char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    if(file == NULL)
    {
      fprintf(stderr, "Provided file doesn't exist.");
      exit(1);
    }
    if(check_extension(file_name, ".bin"))
    {
      return file;
    }else
    {
      fprintf(stderr, "Provided file doesn't have .bin extension.");
      exit(1);
    }
}


Instruction *read_file(FILE *input_file, uint16_t prog_size)
{
    Instruction *program = malloc(sizeof(Instruction) * prog_size);
    memset(program, 0, prog_size);
    for(int i = 0; i < prog_size; i++)
    {
      fread(&program[i].type,  1, 1, input_file);
      const Instr_def *instr_def = instr_def_by_type(program[i].type);
      if(instr_def->has_operand)
      {
        if(instr_def->operand_type == OPERAND_INT)
        {
          fread(&program[i].operand, sizeof(int64_t), 1, input_file);
        }else if(instr_def->operand_type == OPERAND_FLOAT)
        {
          fread(&program[i].operand,  sizeof(double), 1, input_file);
        }else if(instr_def->operand_type == OPERAND_ADDR)
        {
          fread(&program[i].operand,  sizeof(Memory_addr), 1, input_file);
        }
      }else
      {
        continue;
      }
    }
    return program;
}
