#include "tvm.h"
#include "file_handler.h"
#include "isa/isa.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int64_t f64_to_bits(double val) {
  int64_t tmp;
  memcpy(&tmp, &val, sizeof(double));
  return tmp;
}

double bits_to_f64(int64_t val) {
  double tmp;
  memcpy(&tmp, &val, sizeof(int64_t));
  return tmp;
}

uint8_t read_u8(TVM *tvm) { return tvm->program[tvm->ip++]; }

uint16_t read_u16(TVM *tvm) {
  uint16_t tmp;
  memcpy(&tmp, &tvm->program[tvm->ip], sizeof(uint16_t));
  tvm->ip += 2;
  return tmp;
}

uint64_t read_u64(TVM *tvm){
  uint64_t tmp;
  memcpy(&tmp, &tvm->program[tvm->ip], sizeof(uint64_t));
  tvm->ip += 8;
  return tmp;
}



int64_t read_i64(TVM *tvm) {
  int64_t tmp;
  memcpy(&tmp, &tvm->program[tvm->ip], sizeof(int64_t));
  tvm->ip += 8;
  return tmp;
}

int64_t read_f64(TVM *tvm) {
  double tmp;
  memcpy(&tmp, &tvm->program[tvm->ip], sizeof(double));
  tvm->ip += 8;
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
  case TRAP_ILLEGAL_GLOBALS_ACCESS:
    return "TRAP_ILLEGAL_GLOBALS_ACCESS";
  case TRAP_ILLEGAL_MEMORY_ACCESS:
    return "TRAP_ILLEGAL_MEMORY_ACCESS";
  case TRAP_STACK_OVERFLOW:
    return "TRAP_STACK_OVERFLOW";
  case TRAP_STACK_UNDERFLOW:
    return "TRAP_STACK_UNDERFLOW";
  case TRAP_DIVISION_BY_ZERO:
    return "TRAP_DIVISION_BY_ZERO";
  case TRAP_TYPE_MISMATCH:
    return "TRAP_TYPE_MISSMATCH";
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
      if (tvm->stack[i].is_float == false) {
        printf("   %lld\n", (long long)tvm->stack[i].bits);
      } else {
        printf("   %g\n", bits_to_f64(tvm->stack[i].bits));
      }
    }
  }
}

Trap tvm_execute_instr(TVM *tvm) {
  if (tvm->ip >= tvm->program_size) {
    return TRAP_ILLEGAL_INSTR_ACCESS;
  }
  printf("ip=%d opcode=%d\n", tvm->ip, tvm->program[tvm->ip]);
  uint8_t opcode = read_u8(tvm);

  switch (opcode) {
  case OP_PUSH_INT: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    int64_t val = read_i64(tvm);
    tvm->stack[tvm->sp++] = (StackValue){.bits = val, .is_float = false};
    break;
  }

  case OP_PUSH_FLOAT: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    double val = read_f64(tvm);
    tvm->stack[tvm->sp++] = (StackValue){
        .bits = f64_to_bits(val),
        .is_float = true}; // because our stack is int64_t we need use IEEE 754
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
    tvm->stack[tvm->sp++] =
        (StackValue){.bits = f64_to_bits(b + a), .is_float = true};
    tvm->ip++;
    break;
  }

  case OP_MINUS_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;
    tvm->stack[tvm->sp++] = (StackValue){.bits = b - a, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_MINUS_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);
    tvm->stack[tvm->sp++] =
        (StackValue){.bits = f64_to_bits(b - a), .is_float = true};
    tvm->ip++;
    break;
  }

  case OP_MULT_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;
    tvm->stack[tvm->sp++] = (StackValue){.bits = b * a, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_MULT_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);
    tvm->stack[tvm->sp++] =
        (StackValue){.bits = f64_to_bits(b * a), .is_float = true};
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
    tvm->stack[tvm->sp++] = (StackValue){.bits = b / a, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_DIV_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);
    tvm->stack[tvm->sp++] =
        (StackValue){.bits = f64_to_bits(b / a), .is_float = true};
    tvm->ip++;
    break;
  }

  case OP_JMP: {
    uint64_t position = read_u64(tvm);
    if (position >= tvm->program_size) {
      return TRAP_ILLEGAL_INSTR_ACCESS;
    }
    tvm->ip = position;
    break;
  }

  case OP_JZ: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t cond = tvm->stack[--tvm->sp].bits;
    if (cond == 0) {
      uint16_t position = read_u64(tvm);
      if (position >= tvm->program_size) {
        return TRAP_ILLEGAL_INSTR_ACCESS;
      }
      tvm->ip = position;
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
      uint16_t position = read_u64(tvm);
      if (position >= tvm->program_size) {
        return TRAP_ILLEGAL_INSTR_ACCESS;
      }
      tvm->ip = position;
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

    tvm->stack[tvm->sp++] =
        (StackValue){.bits = (b < a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_LT_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);

    tvm->stack[tvm->sp++] =
        (StackValue){.bits = (b < a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_GT_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;

    tvm->stack[tvm->sp++] =
        (StackValue){.bits = (b > a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_GT_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);

    tvm->stack[tvm->sp++] =
        (StackValue){.bits = (b > a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_EQ_INT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp].bits;
    int64_t b = tvm->stack[--tvm->sp].bits;

    tvm->stack[tvm->sp++] =
        (StackValue){.bits = (b == a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_EQ_FLOAT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    double a = bits_to_f64(tvm->stack[--tvm->sp].bits);
    double b = bits_to_f64(tvm->stack[--tvm->sp].bits);

    tvm->stack[tvm->sp++] =
        (StackValue){.bits = (b == a) ? 1 : 0, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_INT_TO_FLOAT: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }
    if (tvm->stack[tvm->sp - 1].is_float == true) {
      return TRAP_TYPE_MISMATCH;
    }
    int64_t val = f64_to_bits(tvm->stack[tvm->sp - 1].bits);
    tvm->stack[tvm->sp - 1] =
        (StackValue){.bits = (double)val, .is_float = true};
    tvm->ip++;
    break;
  }

  case OP_FLOAT_TO_INT: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }
    if (tvm->stack[tvm->sp - 1].is_float == false) {
      return TRAP_TYPE_MISMATCH;
    }
    double val = bits_to_f64(tvm->stack[tvm->sp - 1].bits);
    tvm->stack[tvm->sp - 1] =
        (StackValue){.bits = (int64_t)val, .is_float = false};
    tvm->ip++;
    break;
  }

  case OP_STORE_GLOBAL: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }
    uint8_t index = read_u8(tvm);
    tvm->globals[index] = tvm->stack[tvm->sp - 1];
    tvm->sp--;
    break;
  }

  case OP_LOAD_GLOBAL: {
    uint8_t index = read_u8(tvm);
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }

    tvm->stack[tvm->sp++] = tvm->globals[index];
    break;
  }

  case OP_CALL: {
    if (tvm->ip + 1 > tvm->program_size) {
      return TRAP_ILLEGAL_INSTR_ACCESS;
    }
    tvm->stack[tvm->sp++] = (StackValue){.bits = tvm->fp, .is_float = false};
    uint16_t position = read_u16(tvm);
    uint8_t arg_count = read_u8(tvm);
    uint8_t max_locals = read_u8(tvm);
    tvm->stack[tvm->sp++] = (StackValue){.bits = tvm->ip, .is_float = false};
    if (tvm->sp < arg_count) // we checking arg count
    {
      return TRAP_STACK_UNDERFLOW;
    }
    tvm->stack[tvm->sp++] = (StackValue){.bits = arg_count, .is_float = false};
    tvm->stack[tvm->sp++] = (StackValue){.bits = max_locals, .is_float = false};
    tvm->fp = tvm->sp;
    tvm->sp += max_locals; // reserving local vars
    tvm->ip = position;    // jumping to func label
    break;
  }

  case OP_RET: {
    StackValue retval = tvm->stack[tvm->sp - 1];
    tvm->sp = tvm->fp;
    uint8_t arg_count = tvm->stack[--tvm->sp].bits;

    tvm->fp = tvm->stack[--tvm->sp].bits;
    tvm->ip = tvm->stack[--tvm->sp].bits;
    tvm->sp -= arg_count;
    tvm->stack[tvm->sp++] = retval;
    break;
  }

  case OP_LOAD_ARG: {
    uint8_t index = read_u8(tvm);
    tvm->stack[tvm->sp++] =
        tvm->stack[tvm->fp - 5 -
                   index]; // we subtracting 3 bc prev fp and return ip
    break;
  }

  case OP_STORE_LOCAL: {
    if (tvm->fp == 0) {
      return TRAP_ILLEGAL_INSTR;
    }
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }
    uint8_t index = read_u8(tvm);
    uint8_t local_count = tvm->stack[tvm->fp - 1].bits;
    if (index >= local_count) {
      return TRAP_ILLEGAL_INSTR;
    }
    tvm->stack[tvm->fp + index] = tvm->stack[--tvm->sp];
    break;
  }

  case OP_LOAD_LOCAL: {
    if (tvm->fp == 0) {
      return TRAP_ILLEGAL_INSTR;
    }
    uint8_t index = read_u8(tvm);
    uint8_t local_count = tvm->stack[tvm->fp - 1].bits;
    if (index >= local_count) {
      return TRAP_ILLEGAL_INSTR;
    }
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    tvm->stack[tvm->sp++] = tvm->stack[tvm->fp + index];
    break;
  }

    // case OP_: {
    //   if (tvm->sp < 1) {
    //     return TRAP_STACK_UNDERFLOW;
    //   }
    //   Memory_addr addr = (Memory_addr)instr.operand.i64;
    //   if (addr > HEAP_SIZE - sizeof(int64_t)) {
    //     return TRAP_ILLEGAL_MEMORY_ACCESS;
    //   }
    //   int64_t val = tvm->stack[--tvm->sp].bits;
    //   memcpy(&tvm->memory[addr], &val, sizeof(val));
    //   tvm->ip++;
    //   break;
    // }

    // case OP_LOAD_INT: {
    //   if (tvm->sp >= STACK_SIZE) {
    //     return TRAP_STACK_OVERFLOW;
    //   }
    //   Memory_addr addr = (Memory_addr)instr.operand.i64;
    //   if (addr > HEAP_SIZE - sizeof(int64_t)) {
    //     return TRAP_ILLEGAL_MEMORY_ACCESS;
    //   }
    //   int64_t val;
    //   memcpy(&val, &tvm->memory[addr], sizeof(val));
    //   tvm->stack[tvm->sp++] = (StackValue){.bits = val, .is_float = false};
    //   tvm->ip++;
    //   break;
    // }

    // case OP_LOAD_FLOAT: {
    //   if (tvm->sp >= STACK_SIZE) {
    //     return TRAP_STACK_OVERFLOW;
    //   }
    //   Memory_addr addr = (Memory_addr)instr.operand.i64;
    //   if (addr > HEAP_SIZE - sizeof(int64_t)) {
    //     return TRAP_ILLEGAL_MEMORY_ACCESS;
    //   }
    //   int64_t val;
    //   memcpy(&val, &tvm->memory[addr], sizeof(val));
    //   tvm->stack[tvm->sp++] = (StackValue){.bits = val, .is_float = true};
    //   tvm->ip++;
    //   break;
    // }

  case OP_HALT: {
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

TVM *tvm_init(uint8_t *program, int program_size) {
  TVM *tvm = malloc(sizeof(TVM));
  if (!tvm) {
    fprintf(stderr, "Memmory allocation failde\n");
    exit(1);
  }
  memset(tvm, 0, sizeof(TVM));
  tvm->program = program;
  tvm->program_size = program_size;
  tvm->halt = false;
  return tvm;
}

void tvm_free(TVM *tvm) { free(tvm); }

int main(int argc, char *argv[]) {
  char *input_file = NULL;

  if (argc < 2) {
    fprintf(stderr, "Usage: tvm <input file.bin>");
    return 1;
  }
  input_file = argv[1];
  FILE *input_binary = open_file(input_file);
  Header input_header = read_header(input_binary);
  printf("prog_size = %d\n", input_header.prog_size);
  uint8_t *program = read_file(input_binary, &input_header.prog_size);
  TVM *tvm = tvm_init(program, input_header.prog_size);
  printf("program_size = %d\n", tvm->program_size);
  tvm_run_program(tvm);
}
