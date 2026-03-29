#ifndef INSTRUCTION_H
#define INSTRUCTION_H


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "isa.h"


typedef enum {
  TYPE_I64,
  TYPE_U64,
  TYPE_F64,
  TYPE_U16,
  TYPE_U8,  
}Operand_type;


typedef struct{
  uint8_t operand_count;
  Operand_type operand_type[3];
  uint8_t byte_size;
}Instr_def;


extern const Instr_def INSTR_DEFS[];
extern const size_t INSTR_DEFS_COUNT;


const Instr_def* instr_def_by_type(OP_CODE type);
uint8_t instr_size_by_type(OP_CODE type);


#endif
