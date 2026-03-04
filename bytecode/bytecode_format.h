#ifndef BYTECODE_FORMAT_H
#define BYTECODE_FORMAT_H

#include "../isa/isa.h"
#include <stdint.h>



typedef union {
  int64_t i64;
  double f64;
}Operand;


typedef struct {
  OP_CODE type;
  Operand operand;
} Instruction;






#endif
