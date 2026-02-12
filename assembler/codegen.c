#include "dynamic_array.h"
#include "parser.h"
#include "stdio.h"



void codegen(char *output_file, DynamicArray *instructions)
{
  FILE *output = fopen(output_file, "wb");
  for(int i = 0; i < instructions->length; i++)
  {
    Instruction *instr = (Instruction *) get_element(instructions, i);
    Instr_def const *instr_def = instr_def_by_type(instr->type);
    if(instr_def->has_operand)
    {
      fputc(instr->type, output);
      fwrite(&instr->operand, sizeof(instr->operand), 1, output);
    }else
    {
      fputc(instr->type, output);
    }
  }
  fclose(output);
}
