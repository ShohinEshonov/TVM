#include <assert.h>
#include <endian.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define STACK_SIZE 1024
#define HEAP_SIZE 4 * (1024 * 1024)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define TVM_PROGRAM_CAPACITY 1024

typedef uint64_t Memory_addr;

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

  OP_LOAD,
  OP_STORE,

  OP_HALT,
} Instr_type;

typedef struct {
  Instr_type type;
  int64_t operand;
} Instruction;

typedef enum {
  TRAP_OK = 0,
  TRAP_ILLEGAL_INSTR,
  TRAP_ILLEGAL_INSTR_ACCESS,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
  TRAP_DIVISION_BY_ZERO,

  TRAP_ILLEGAL_MEMORY_ACCESS,
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
  case TRAP_DIVISION_BY_ZERO:
    return "TRAP_DIVISION_BY_ZERO";

  default:
    return "UNKNOWN_TRAP";
  }
}

typedef struct {
  int64_t stack[STACK_SIZE];
  uint16_t sp;
  Instruction program[TVM_PROGRAM_CAPACITY];
  int program_size;
  uint16_t ip;
  uint8_t memory[HEAP_SIZE];
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

    int64_t top = tvm->stack[tvm->sp - 1];
    tvm->stack[tvm->sp++] = top;
    tvm->ip++;
    break;
  }

  case OP_PLUS: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int64_t a = tvm->stack[--tvm->sp];
    int64_t b = tvm->stack[--tvm->sp];
    tvm->stack[tvm->sp++] = b + a;
    tvm->ip++;
    break;
  }
  case OP_MINUS: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp];
    int64_t b = tvm->stack[--tvm->sp];
    tvm->stack[tvm->sp++] = b - a;
    tvm->ip++;
    break;
  }
  case OP_MULT: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int64_t a = tvm->stack[--tvm->sp];
    int64_t b = tvm->stack[--tvm->sp];
    tvm->stack[tvm->sp++] = b * a;
    tvm->ip++;
    break;
  }
  case OP_DIV: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }
    int64_t a = tvm->stack[--tvm->sp];
    if (a == 0) {
      return TRAP_DIVISION_BY_ZERO;
    }
    int64_t b = tvm->stack[--tvm->sp];
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

    int cond = tvm->stack[--tvm->sp];
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

    int cond = tvm->stack[--tvm->sp];
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

    int64_t a = tvm->stack[--tvm->sp];
    int64_t b = tvm->stack[--tvm->sp];

    tvm->stack[tvm->sp++] = (b > a) ? 1 : 0;
    tvm->ip++;
    break;
  }

  case OP_EQ: {
    if (tvm->sp < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    int64_t a = tvm->stack[--tvm->sp];
    int64_t b = tvm->stack[--tvm->sp];

    tvm->stack[tvm->sp++] = (b == a) ? 1 : 0;
    tvm->ip++;
    break;
  }

  case OP_HALT: {
    tvm->halt = true;
    break;
  }

  case OP_STORE: {
    if (tvm->sp < 1) {
      return TRAP_STACK_UNDERFLOW;
    }
    Memory_addr addr = (Memory_addr)instr.operand;
    if (addr > HEAP_SIZE - sizeof(int64_t)) {
      return TRAP_ILLEGAL_MEMORY_ACCESS;
    }
    int64_t val = tvm->stack[--tvm->sp];
    memcpy(&tvm->memory[addr], &val, sizeof(val));
    tvm->ip++;
    break;
  }

  case OP_LOAD: {
    if (tvm->sp >= STACK_SIZE) {
      return TRAP_STACK_OVERFLOW;
    }
    Memory_addr addr = (Memory_addr)instr.operand;
    if (addr > HEAP_SIZE - sizeof(int64_t)) {
      return TRAP_ILLEGAL_MEMORY_ACCESS;
    }
    int64_t val;
    memcpy(&val, &tvm->memory[addr], sizeof(val));
    tvm->stack[tvm->sp++] = val;
    tvm->ip++;
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
    printf("   [empty]\n`");
  } else {
    for (int i = 0; i < tvm->sp; i++) {
      printf("   %lld\n", (long long)tvm->stack[i]);
    }
  }
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


//example for factorial of 5
Instruction program[] = {
    {OP_PUSH, 1},  // 0: accumulator = 1
    {OP_STORE, 0}, // 1: memory[0] = 1

    {OP_PUSH, 5},  // 2: n = 5
    {OP_STORE, 8}, // 3: memory[8] = 5

    // LOOP_START (адрес 4):
    {OP_LOAD, 8}, // 4: load n
    {OP_PUSH, 1}, // 5:
    {OP_LT},      // 6: n < 1?
    {OP_JNZ, 17}, // 7: if yes exit

    // accumulator *= n
    {OP_LOAD, 0},  // 8: load accumulator
    {OP_LOAD, 8},  // 9: load n
    {OP_MULT},     // 10: accumulator * n
    {OP_STORE, 0}, // 11: memory[0] = accumulator * n

    // n--
    {OP_LOAD, 8},  // 12: load n
    {OP_PUSH, 1},  // 13:
    {OP_MINUS},    // 14: n - 1
    {OP_STORE, 8}, // 15: memory[8] = n - 1

    {OP_JMP, 4}, // 16: jump LOOP_START

    // EXIT:
    {OP_LOAD, 0}, // 17: load result
    {OP_HALT}     // 18:
};
int main() {
  TVM* tvm = tvm_init(program, ARRAY_SIZE(program));
  tvm_run_program(tvm);
  tvm_free(tvm);
  return 0;
}
