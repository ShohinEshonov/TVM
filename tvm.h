#ifndef TVM_H
#define TVM_H

#include <stdint.h>
#include "instructions.h"

#define STACK_SIZE 1024
#define HEAP_SIZE 4 * (1024 * 1024)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define TVM_PROGRAM_CAPACITY 1024

typedef uint32_t Memory_addr;

typedef enum {
  TRAP_OK = 0,
  TRAP_ILLEGAL_INSTR,
  TRAP_ILLEGAL_INSTR_ACCESS,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
  TRAP_DIVISION_BY_ZERO,
  TRAP_ILLEGAL_MEMORY_ACCESS,
  TRAP_TYPE_MISMATCH,
} Trap;

typedef union {
  int64_t i64;
  double f64;
}Operand;

typedef struct {
  Instr_type type;
  Operand operand;
} Instruction;

typedef struct{
  int64_t bits;
  bool is_float;
}StackValue;

typedef struct {
  StackValue stack[STACK_SIZE];
  uint16_t sp;
  Instruction program[TVM_PROGRAM_CAPACITY];
  uint16_t program_size;
  uint16_t ip;
  uint8_t memory[HEAP_SIZE];
  bool halt;
} TVM;

TVM* tvm_init(Instruction program[], int program_size);
Trap tvm_execute_instr(TVM *tvm);
void tvm_dump(TVM *tvm);
void tvm_run_program(TVM *tvm);
void tvm_free(TVM* tvm);
int64_t f64_to_bits(double val);
double bits_to_f64(int64_t val);
char *trap_to_cstr(Trap trap); 

#endif
