#include "isa_table.h"
#include "isa.h"
#include <stddef.h>
#include <string.h>
#include <wchar.h>

const Instr_def INSTR_DEFS[] = {
    [OP_PUSH_INT] = {.operand_count = 1, .operand_type = {TYPE_I64}, .byte_size = 1 + sizeof(int64_t)},
    [OP_PUSH_FLOAT] = {.operand_count = 1, .operand_type = {TYPE_F64}, .byte_size = 1 + sizeof(double)},

    [OP_POP] = {.operand_count = 0, .byte_size = 1},
    [OP_DUP] = {.operand_count = 0, .byte_size = 1},

    [OP_PLUS_INT] = {.operand_count = 0, .byte_size = 1},
    [OP_MINUS_INT] = {.operand_count = 0, .byte_size = 1},
    [OP_MULT_INT] = {.operand_count = 0, .byte_size = 1},
    [OP_DIV_INT] = {.operand_count = 0, .byte_size = 1},

    [OP_PLUS_FLOAT] = {.operand_count = 0, .byte_size = 1},
    [OP_MINUS_FLOAT] = {.operand_count = 0, .byte_size = 1},
    [OP_MULT_FLOAT] = {.operand_count = 0, .byte_size = 1},
    [OP_DIV_FLOAT] = {.operand_count = 0, .byte_size = 1},

    [OP_JMP] = {.operand_count = 1, .operand_type = {TYPE_U64}, .byte_size = 1 + sizeof(uint64_t)},
    [OP_JZ] = {.operand_count = 1, .operand_type = {TYPE_U64}, .byte_size = 1 + sizeof(uint64_t)},
    [OP_JNZ] = {.operand_count = 1, .operand_type = {TYPE_U64}, .byte_size = 1 + sizeof(uint64_t)},

    [OP_LT_INT] = {.operand_count = 0, .byte_size = 1},
    [OP_GT_INT] = {.operand_count = 0, .byte_size = 1},
    [OP_EQ_INT] = {.operand_count = 0, .byte_size = 1},

    [OP_LT_FLOAT] = {.operand_count = 0, .byte_size = 1},
    [OP_GT_FLOAT] = {.operand_count = 0, .byte_size = 1},
    [OP_EQ_FLOAT] = {.operand_count = 0, .byte_size = 1},

    [OP_STORE_GLOBAL] = {.operand_count = 1, .operand_type = {TYPE_U8}, .byte_size = 1 + sizeof(uint8_t)},
    [OP_LOAD_GLOBAL] = {.operand_count = 1, .operand_type = {TYPE_U8}, .byte_size = 1 + sizeof(uint8_t)},

    [OP_INT_TO_FLOAT] = {.operand_count = 0, .byte_size = 1},
    [OP_FLOAT_TO_INT] = {.operand_count = 0, .byte_size = 1},

    [OP_CALL] = {.operand_count = 3,.operand_type = {TYPE_U64, TYPE_U8, TYPE_U8}, .byte_size = 1 + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint8_t)},
    [OP_RET] = {.operand_count = 0, .byte_size = 1},
    [OP_LOAD_ARG] = {.operand_count = 1, .operand_type = {TYPE_U8}, .byte_size = 1 + sizeof(uint8_t)},
    [OP_LOAD_LOCAL] = {.operand_count = 1, .operand_type = {TYPE_U8}, .byte_size = 1 + sizeof(uint8_t)},
    [OP_STORE_LOCAL] = {.operand_count = 1, .operand_type = {TYPE_U8}, .byte_size = 1 + sizeof(uint8_t)},

    [OP_HALT] = {.operand_count = 0, .byte_size = 1},
};

const size_t INSTR_DEFS_COUNT = sizeof(INSTR_DEFS) / sizeof(INSTR_DEFS[0]);

const Instr_def *instr_def_by_type(OP_CODE type) {
  if (type >= INSTR_DEFS_COUNT) {
    return NULL;
  }
  return &INSTR_DEFS[type];
}


uint8_t instr_size_by_type(OP_CODE type)
{
  return INSTR_DEFS[type].byte_size;
}
