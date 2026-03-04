#include "isa_table.h"
#include <stddef.h>
#include <string.h>


const Instr_def INSTR_DEFS[]={
	[OP_PUSH_INT] = { .has_operand = true, .operand_type = OPERAND_INT},
	[OP_PUSH_FLOAT] = { .has_operand = true, .operand_type = OPERAND_FLOAT},
	
	[OP_POP] = {.has_operand = false, .operand_type = OPERAND_NONE},
	[OP_DUP] = {.has_operand = false, .operand_type = OPERAND_NONE},
	
	[OP_PLUS_INT] = { .has_operand = false,.operand_type = OPERAND_NONE},
	[OP_MINUS_INT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_MULT_INT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_DIV_INT]  = { .has_operand = false, .operand_type = OPERAND_NONE},

	[OP_PLUS_FLOAT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_MINUS_FLOAT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_MULT_FLOAT] =  { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_DIV_FLOAT] = { .has_operand = false, .operand_type = OPERAND_NONE},

	[OP_JMP] = { .has_operand = true, .operand_type = OPERAND_INT},
	[OP_JZ] = { .has_operand = true, .operand_type = OPERAND_INT},
	[OP_JNZ] = { .has_operand = true, .operand_type = OPERAND_INT},

	[OP_LT_INT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_GT_INT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_EQ_INT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	
	[OP_LT_FLOAT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_GT_FLOAT] = { .has_operand = false, .operand_type = OPERAND_NONE},
	[OP_EQ_FLOAT] = { .has_operand = false, .operand_type = OPERAND_NONE},

	[OP_STORE] = { .has_operand = true, .operand_type = OPERAND_ADDR},
	[OP_LOAD_INT] = { .has_operand =  true, .operand_type = OPERAND_ADDR},
	[OP_LOAD_FLOAT] = { .has_operand = true, .operand_type = OPERAND_ADDR},

	[OP_INT_TO_FLOAT] = { .has_operand = false, .operand_type =  OPERAND_NONE},
	[OP_FLOAT_TO_INT] = { .has_operand = false, .operand_type = OPERAND_NONE},

	//{OP_ALLOC, "alloc", true, OPERAND_INT},
	//{OP_MEM_WRITE, "mem_write", false, OPERAND_NONE},
	//{OP_MEM_FREE, "mem_free", false, OPERAND_NONE},
	//TODO: add stack frames and then add memory alloc free write
	[OP_HALT] = { .has_operand = false, .operand_type = OPERAND_NONE},
};


const size_t INSTR_DEFS_COUNT = sizeof(INSTR_DEFS) / sizeof(INSTR_DEFS[0]);



const Instr_def* instr_def_by_type(OP_CODE type){
	if(type > INSTR_DEFS_COUNT)
	{
		return NULL;
	}
	return &INSTR_DEFS[type];
}





