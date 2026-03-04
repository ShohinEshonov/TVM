#ifndef INSTRUCTION_H
#define INSTRUCTION_H


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "isa.h"


typedef enum {
  OPERAND_NONE,
  OPERAND_INT,
  OPERAND_FLOAT,
  OPERAND_ADDR,
}Operand_type;


typedef struct{
  OP_CODE type;
  const char* name;
  bool has_operand;
  Operand_type operand_type;
}Instr_def;


extern const Instr_def INSTR_DEFS[];
extern const size_t INSTR_DEFS_COUNT;


const Instr_def* instr_def_by_name(const char* name);
const Instr_def* instr_def_by_type(OP_CODE type);


#endif
