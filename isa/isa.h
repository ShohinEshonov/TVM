#ifndef ISA_H
#define ISA_H

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
} OP_CODE;

#endif
