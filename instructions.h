#ifndef INSTRUCTION_H
#define INSTRUCTION_H




#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  OP_PUSH_INT,
  OP_PUSH_FLOAT,

  OP_POP,
  
  OP_DUP,

  OP_PLUS_INT,
  OP_MINUS_INT,
  OP_MULT_INT,
  OP_DIV_INT,

  OP_PLUS_FLOAT,
  OP_MINUS_FLOAT,
  OP_MULT_FLOAT,
  OP_DIV_FLOAT,

  OP_JMP,
  OP_JZ,
  OP_JNZ,

  OP_LT_INT,
  OP_GT_INT,
  OP_EQ_INT,


  OP_LT_FLOAT,
  OP_GT_FLOAT,
  OP_EQ_FLOAT,


  OP_STORE,
  OP_LOAD_INT,
  OP_LOAD_FLOAT,

  OP_INT_TO_FLOAT,
  OP_FLOAT_TO_INT,
  
 // OP_ALLOC,
  //OP_MEM_WRITE,
  //OP_MEM_FREE,
  //TODO: add stack frames and then memory alloc free write
  OP_HALT,
} Instr_type;



typedef union {
  int64_t i64;
  double f64;
}Operand;

typedef struct {
  Instr_type type;
  Operand operand;
} Instruction;



typedef enum {
  OPERAND_NONE,
  OPERAND_INT,
  OPERAND_FLOAT,
  OPERAND_ADDR,
}Operand_type;


typedef struct{
  Instr_type type;
  const char* name;
  bool has_operand;
  Operand_type operand_type;
}Instr_def;


extern const Instr_def INSTR_DEFS[];
extern const size_t INSTR_DEFS_COUNT;


const Instr_def* instr_def_by_name(const char* name);
const Instr_def* instr_def_by_type(Instr_type type);


#endif
