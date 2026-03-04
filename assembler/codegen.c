#include "dynamic_array.h"
#include <stdio.h>
#include "../format.h"
#include "../bytecode/bytecode_format.h"
#include "../isa/isa_table.h"



void codegen(char *output_file, DynamicArray *instructions)
{
  FILE *output = fopen(output_file, "wb");
  fwrite(MAGIC, 1, MAGIC_SIZE, output);
  fwrite(&instructions->length, 1, sizeof(uint16_t), output);

  for(size_t i = 0; i < instructions->length; i++)
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
