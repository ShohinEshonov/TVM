#include "tvm.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int64_t f64_to_bits(double val){
  int64_t tmp;
  memcpy(&tmp, &val, sizeof(double));
  return tmp;
}


double bits_to_f64(int64_t val){
  double tmp;
  memcpy(&tmp, &val, sizeof(int64_t));
  return tmp;
}


char *trap_to_cstr(Trap trap) {
  switch (trap) {
  case TRAP_OK:
    return "TRAP_OK";
  case TRAP_ILLEGAL_INSTR:
    return "TRAP_ILLEGAL_INSTR";
  case TRAP_ILLEGAL_INSTR_ACCESS:
    return "TRAP_ILLEGAL_INSTR_ACCESS";
  case TRAP_ILLEGAL_MEMORY_ACCESS:
    return "TRAP_ILLEGAL_MEMORY_ACCESS";
  case TRAP_STACK_OVERFLOW:
    return "TRAP_STACK_OVERFLOW";
  case TRAP_STACK_UNDERFLOW:
    return "TRAP_STACK_UNDERFLOW";
  case TRAP_DIVISION_BY_ZERO:
    return "TRAP_DIVISION_BY_ZERO";

  default:
    return "UNKNOWN_TRAP";
  }
}



void tvm_dump(TVM *tvm) {
  printf("Stack: \n");
  if (tvm->sp == 0) {
    printf("   [empty]\n");
  } else {
    for (int i = 0; i < tvm->sp; i++) {
      if(tvm->stack[i].is_float == false)
      {
        printf("   %lld\n", (long long)tvm->stack[i].bits);
      }else{
        printf("   %g\n", bits_to_f64(tvm->stack[i].bits));
      }
    }
  }
}


Trap tvm_execute_instr(TVM *tvm) {
  if (tvm->ip >= tvm->program_size) {
    return TRAP_ILLEGAL_INSTR_ACCESS;
  }

  Instruction instr = tvm->program[tvm->ip];

  switch (instr.type) {
  case OP_PUSH_INT: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    tvm->stack[tvm->sp++] = (StackValue){.bits = instr.operand.i64, .is_float = false};
    tvm->ip++;
    break;
  }


  case OP_PUSH_FLOAT: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    tvm->stack[tvm->sp++] = (StackValue){.bits = f64_to_bits(instr.operand.f64), .is_float = true};  //because our stack is int64_t we need use IEEE 754
    tvm->ip++;
    break;
  }

  case OP_POP: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }
    tvm->sp--;
    tvm->ip++;
    break;
  }

  case OP_DUP: {
    if (tvm->sp < 1) {
      fprintf(stderr, "Stack underflow at IP: %d\n", tvm->ip);
      return TRAP_STACK_UNDERFLOW;
    }
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }

    tvm->stack[tvm->sp] = tvm->stack[tvm->sp - 1];
    tvm->sp++;
    tvm->ip++;
    break;
  }




  case OP_PLUS_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;
    tvm->stack[tvm->sp++] = (StackValue){.bits = b + a, .is_float = false};
    tvm->ip++;
    break;
  }


  case OP_PLUS_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);
    tvm->stack[tvm->sp++] = (StackValue){.bits = f64_to_bits(b + a), .is_float = true};
    tvm->ip++;
    break;
  }


  
  case OP_MINUS_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;
    tvm->stack[tvm->sp++] =(StackValue){.bits = b - a, .is_float = false};
    tvm->ip++;
    break;
  }


  case OP_MINUS_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);
    tvm->stack[tvm->sp++] = (StackValue){.bits = f64_to_bits(b - a), .is_float =true};
    tvm->ip++;
    break;
  }

  
  case OP_MULT_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;
    tvm->stack[tvm->sp++] = (StackValue) {.bits = b * a, .is_float = false};
    tvm->ip++;
    break;
  }

  
  case OP_MULT_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);
    tvm->stack[tvm->sp++] = (StackValue){.bits = f64_to_bits(b * a), .is_float = true};
    tvm->ip++;
    break;
  }


  
  case OP_DIV_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int64_t a = tvm->stack[--tvm->sp].bits;
    if (a == 0) {
      return TRAP_DIVISION_BY_ZERO;
    }
    int64_t b = tvm->stack[--tvm->sp].bits;
    tvm->stack[tvm->sp++] = (StackValue) {.bits = b / a, .is_float = false} ;
    tvm->ip++;
    break;
  }


  case OP_DIV_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);
    tvm->stack[tvm->sp++] = (StackValue) {.bits = f64_to_bits(b / a), .is_float = true};
    tvm->ip++;
    break;
  }


  case OP_JMP: {
    if (instr.operand.i64 < 0 || instr.operand.i64  >= tvm->program_size) {
      return TRAP_ILLEGAL_INSTR_ACCESS;
    }
    tvm->ip = instr.operand.i64;
    break;
  }

  case OP_JZ: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t cond = tvm->stack[--tvm->sp].bits;
    if (cond == 0) {
      if (instr.operand.i64 < 0 || instr.operand.i64 >= tvm->program_size) {
        return TRAP_ILLEGAL_INSTR_ACCESS;
      }
      tvm->ip = instr.operand.i64;
    } else {
      tvm->ip++;
    }
    break;
  }

  case OP_JNZ: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t cond = tvm->stack[--tvm->sp].bits;
    if (cond != 0) {
      if (instr.operand.i64 < 0 || instr.operand.i64  >= tvm->program_size) {
        return TRAP_ILLEGAL_INSTR_ACCESS;
      }
      tvm->ip = instr.operand.i64;
    } else {
      tvm->ip++;
    }
    break;
  }

  case OP_LT_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;

    tvm->stack[tvm->sp++] = (StackValue){.bits = (b < a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }


  case OP_LT_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);

    tvm->stack[tvm->sp++] =(StackValue){.bits =  (b < a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }


  case OP_GT_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;

    tvm->stack[tvm->sp++] = (StackValue){.bits = (b > a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }

  
  case OP_GT_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);

    tvm->stack[tvm->sp++] = (StackValue){.bits = (b > a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }




  case OP_EQ_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;

    tvm->stack[tvm->sp++] =(StackValue) {.bits = (b == a) ? 1 : 0, .is_float = false} ;
    tvm->ip++;
    break;
  }

  
  case OP_EQ_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);

    tvm->stack[tvm->sp++] = (StackValue){.bits = (b == a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }
  
  case OP_INT_TO_FLOAT:{
    if(tvm->sp < 1){
      return TRAP_STACK_UNDERFLOW;
    }
    if(tvm->stack[tvm->sp-1].is_float == true){
      return TRAP_TYPE_MISMATCH;
    }
    int64_t val = f64_to_bits(tvm->stack[tvm->sp-1].bits);
    tvm->stack[tvm->sp-1] = (StackValue){.bits = (double) val, .is_float = true};
    tvm->ip++;
    break;
  }
  
  case OP_FLOAT_TO_INT:{
    if(tvm->sp < 1){
      return TRAP_STACK_UNDERFLOW;
    }
    if(tvm->stack[tvm->sp-1].is_float == false){
      return TRAP_TYPE_MISMATCH;
    }
    double val = bits_to_f64(tvm->stack[tvm->sp-1].bits);
    tvm->stack[tvm->sp-1] = (StackValue){.bits = (int64_t) val, .is_float = false};
    tvm->ip++;
    break;
  }


  case OP_STORE: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }
    Memory_addr addr = (Memory_addr)instr.operand.i64;
    if (addr > HEAP_SIZE - sizeof(int64_t)) {
      return TRAP_ILLEGAL_MEMORY_ACCESS;
    }
    int64_t val = tvm->stack[--tvm->sp].bits;
    memcpy(&tvm->memory[addr], &val, sizeof(val));
    tvm->ip++;
    break;
  }


  case OP_LOAD_INT: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    Memory_addr addr = (Memory_addr)instr.operand.i64;
    if (addr > HEAP_SIZE - sizeof(int64_t)) {
      return TRAP_ILLEGAL_MEMORY_ACCESS;
    }
    int64_t val;
    memcpy(&val, &tvm->memory[addr], sizeof(val));
    tvm->stack[tvm->sp++] = (StackValue){.bits = val, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_LOAD_FLOAT: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    Memory_addr addr = (Memory_addr)instr.operand.i64;
    if (addr > HEAP_SIZE - sizeof(int64_t)) {
      return TRAP_ILLEGAL_MEMORY_ACCESS;
    }
    int64_t val;
    memcpy(&val, &tvm->memory[addr], sizeof(val));
    tvm->stack[tvm->sp++] = (StackValue){.bits = val, .is_float = true};
    tvm->ip++;
    break;
  }

  case OP_HALT:{
      tvm->halt = true;
      break;
    }


  default:
    return TRAP_ILLEGAL_INSTR;
  }
  return TRAP_OK;
}

void tvm_run_program(TVM *tvm) {
  while (tvm->halt != true) {
    Trap trap = tvm_execute_instr(tvm);
    if (trap != TRAP_OK) {
      fprintf(stderr, "Trap catched: %s\n", trap_to_cstr(trap));
      break;
    };
    tvm_dump(tvm);
  }
}

TVM* tvm_init(Instruction program[], int program_size) {
  TVM* tvm = malloc(sizeof(TVM));
  if(!tvm)
  {
    fprintf(stderr, "Memmory allocation failde\n");
    exit(1);
  }
  memset(tvm, 0, sizeof(TVM));
  memcpy(tvm->program, program, sizeof(program[0]) * program_size);
  tvm->program_size = program_size;
  tvm->halt = false;
  return tvm;
}

void tvm_free(TVM* tvm)
{
  free(tvm);
}



