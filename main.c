#include <assert.h>
#include <endian.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STACK_SIZE 1024
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define TVM_PROGRAM_CAPACITY 1024

typedef enum {
  OP_PUSH,
  OP_POP,
  OP_DUP,

  OP_PLUS,
  OP_MINUS,
  OP_MULT,
  OP_DIV,

  OP_JMP,
  OP_JZ,
  OP_JNZ,

  OP_LT,
  OP_GT,
  OP_EQ,

  OP_HALT,
} Instr_type;

typedef struct {
  Instr_type type;
  int operand;
} Instruction;

typedef enum {
  TRAP_OK = 0,
  TRAP_ILLEGAL_INSTR,
  TRAP_ILLEGAL_INSTR_ACCESS,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
} Trap;

char *trap_to_cstr(Trap trap) {
  switch (trap) {
  case TRAP_OK:
    return "TRAP_OK";
  case TRAP_ILLEGAL_INSTR:
    return "TRAP_ILLEGAL_INSTR";
  case TRAP_ILLEGAL_INSTR_ACCESS:
    return "TRAP_ILLEGAL_INSTR_ACCESS";
  case TRAP_STACK_OVERFLOW:
    return "TRAP_STACK_OVERFLOW";
  case TRAP_STACK_UNDERFLOW:
    return "TRAP_STACK_UNDERFLOW";
  default:
    return "UNKNOWN_TRAP";
  }
}

typedef struct {
  int32_t stack[STACK_SIZE];
  uint16_t sp;
  Instruction program[TVM_PROGRAM_CAPACITY];
  int program_size;
  uint16_t ip;
  bool halt;
} TVM;

Trap tvm_execute_instr(TVM *tvm) {
  if (tvm->ip >= tvm->program_size) {
    return TRAP_ILLEGAL_INSTR_ACCESS;
  }

  Instruction instr = tvm->program[tvm->ip];

  switch (instr.type) {
  case OP_PUSH: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    tvm->stack[tvm->sp++] = instr.operand;
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
      return TRAP_STACK_UNDERFLOW;
    }
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }

    int top = tvm->stack[tvm->sp - 1];
    tvm->stack[tvm->sp++] = top;
    tvm->ip++;
    break;
  }

  case OP_PLUS: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int a = tvm->stack[--tvm->sp];
    int b = tvm->stack[--tvm->sp];
    tvm->stack[tvm->sp++] = b + a;
    tvm->ip++;
    break;
  }
  case OP_MINUS: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int a = tvm->stack[--tvm->sp];
    int b = tvm->stack[--tvm->sp];
    tvm->stack[tvm->sp++] = b - a;
    tvm->ip++;
    break;
  }
  case OP_MULT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int a = tvm->stack[--tvm->sp];
    int b = tvm->stack[--tvm->sp];
    tvm->stack[tvm->sp++] = b * a;
    tvm->ip++;
    break;
  }
  case OP_DIV: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int a = tvm->stack[--tvm->sp];
    int b = tvm->stack[--tvm->sp];
    tvm->stack[tvm->sp++] = b / a;
    tvm->ip++;
    break;
  }

  case OP_JMP: {
    if (instr.operand < 0 || instr.operand >= tvm->program_size) {
      return TRAP_ILLEGAL_INSTR_ACCESS;
    }
    tvm->ip = instr.operand;
    break;
  }

  case OP_JZ: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }

    int cond = tvm->stack[tvm->sp-1];
    if (cond == 0) {
      if (instr.operand < 0 || instr.operand >= tvm->program_size) {
        return TRAP_ILLEGAL_INSTR_ACCESS;
      }
      tvm->ip = instr.operand;
    } else {
      tvm->ip++;
    }
    break;
  }

  case OP_JNZ: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }

    int cond = tvm->stack[tvm->sp-1];
    if (cond != 0) {
      if (instr.operand < 0 || instr.operand >= tvm->program_size) {
        return TRAP_ILLEGAL_INSTR_ACCESS;
      }
      tvm->ip = instr.operand;
    } else {
      tvm->ip++;
    }
    break;
  }

  case OP_LT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int a = tvm->stack[--tvm->sp];
    int b = tvm->stack[--tvm->sp];

    tvm->stack[tvm->sp++] = (b < a) ? 1 : 0;
    tvm->ip++;
    break;
  }

  case OP_GT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int a = tvm->stack[--tvm->sp];
    int b = tvm->stack[--tvm->sp];

    tvm->stack[tvm->sp++] = (b > a) ? 1 : 0;
    tvm->ip++;
    break;
  }

  case OP_EQ: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int a = tvm->stack[--tvm->sp];
    int b = tvm->stack[--tvm->sp];

    tvm->stack[tvm->sp++] = (b == a) ? 1 : 0;
    tvm->ip++;
    break;
  }

  case OP_HALT: {
    tvm->halt = true;
    break;
  }
  default:
    return TRAP_ILLEGAL_INSTR;
  }
  return TRAP_OK;
}

void tvm_dump(TVM *tvm) {
  printf("Stack: \n");
  if (tvm->sp == 0) {
    printf("   [empty]");
  } else {
    for (int i = 0; i < tvm->sp; i++) {
      printf("   %d\n", tvm->stack[i]);
    }
  }
}

void tvm_run_program(TVM *tvm) {
  while (tvm->halt != true) {
    Trap trap = tvm_execute_instr(tvm);
    if (trap != TRAP_OK) {
      fprintf(stderr, "Trap catched: %s\n", trap_to_cstr(trap));
    };
    tvm_dump(tvm);
  }
}

TVM tvm_init(Instruction program[], int program_size) {
  TVM tvm = {0};
  memcpy(tvm.program, program, sizeof(program[0]) * program_size);
  tvm.program_size = program_size;
  tvm.halt = false;
  return tvm;
}

Instruction program[] = {{OP_PUSH, 6}, {OP_PUSH, 9}, {OP_EQ},
                         {OP_JZ, 5},   {OP_JMP, 1},  {OP_HALT}};

int main() {
  TVM tvm = tvm_init(program, ARRAY_SIZE(program));
  tvm_run_program(&tvm);
  return 0;
}
