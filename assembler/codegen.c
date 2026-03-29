#include <stdio.h>
#include <wchar.h>
#include "../format.h"
#include "parser.h"




static void emit(Instruction *instr, FILE *file)
{
  fwrite(&instr->opcode, sizeof(uint8_t), 1, file);

  if(instr->operand_count > 0)
  {
    for(int i = 0; i < instr->operand_count; i++)
    {
      switch(instr->operands[i].type)
      {
        case VAL_I64:
          fwrite(&instr->operands[i].i64, sizeof(int64_t), 1, file);
          break;
        case VAL_U64:
          fwrite(&instr->operands[i].u64, sizeof(uint64_t), 1, file);
          break;
      
        case VAL_U16:
          fwrite(&instr->operands[i].u16, sizeof(uint16_t), 1, file);
          break;
        
        case VAL_U8:
          fwrite(&instr->operands[i].u8, sizeof(uint8_t), 1, file);
          break;
        
        case VAL_F64:
          fwrite(&instr->operands[i].f64, sizeof(double), 1, file);
          break;
       }
    }
  }
}







void codegen(char *output_file, Parser *parser)
{
  FILE *output = fopen(output_file, "wb");
  fwrite(MAGIC, 1, MAGIC_SIZE, output);
  fwrite(&parser->bc, sizeof(uint16_t),1, output);

  for(size_t i = 0; i < parser->ic; i++)
  {
    Instruction *instr = &parser->program[i];
    emit(instr, output);
  }
  fclose(output);
}


