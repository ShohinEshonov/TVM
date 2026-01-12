#include  <stdio.h>
#include <assert.h>
#include <stdint.h>

#define STACK_SIZE 1024
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct{
  int32_t stack[STACK_SIZE];
  uint16_t stack_size;
  uint16_t ip;
}TVM;

typedef enum{
  OP_PUSH,
  OP_PLUS
}Instr_type;

typedef struct{
  Instr_type type;
  int operand;
}Instruction;

typedef enum{
  TRAP_OK = 0,
  TRAP_ILLEGAL_INSTR,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
}Trap;

char* trap_to_cstr(Trap trap){
  switch(trap)
  {
    case TRAP_ILLEGAL_INSTR:
      return "TRAP_ILLEGAL_INSTR";
    case TRAP_STACK_OVERFLOW:
      return "TRAP_STACK_OVERFLOW";
    case TRAP_STACK_UNDERFLOW:
      return "TRAP_STACK_UNDERFLOW";
  }
}



Trap tvm_execute_instr(TVM *tvm, Instruction instr){
  switch(instr.type)
  {
    case OP_PUSH:
      if(tvm->stack_size >= STACK_SIZE)
      {
        return TRAP_STACK_OVERFLOW;
      }
      tvm->stack[tvm->stack_size++] = instr.operand;
      break;

    case OP_PLUS:
      if(tvm->stack_size < 2)
      {
        return TRAP_STACK_UNDERFLOW;
      }
      tvm->stack[tvm->stack_size-2] += tvm->stack[tvm->stack_size-1];
      tvm->stack_size -= 1;
      break;
    default:
      return TRAP_ILLEGAL_INSTR; 
  }
  return TRAP_OK;
}



void tvm_dump(TVM *tvm){
  printf("Stack: \n");
  if(tvm->stack_size == 0){
    printf("   [empty]");
  }else{
    for(int i = 0; i < tvm->stack_size; i++){
      printf("   %d\n", tvm->stack[i]);
    }
  }
}




Instruction program[] = {{OP_PUSH, 6},{OP_PUSH, 9},{OP_PLUS}, {OP_PUSH, 5}, {OP_PUSH, 10}};

TVM tvm = {0};

int main()
{
  for(int i = 0; i < ARRAY_SIZE(program); i++){
    Trap trap = tvm_execute_instr(&tvm, program[i]);
    if(trap != TRAP_OK){
      fprintf(stderr, "Trap catched: %s\n", trap_to_cstr(trap));
    }
   }
   tvm_dump(&tvm);
  return 0;
}
