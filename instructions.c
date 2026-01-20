#include "instructions.h"
#include <stddef.h>
#include <string.h>


const Instr_def INSTR_DEFS[]={
	{OP_PUSH_INT, "push_int", true, OPERAND_INT},
	{OP_PUSH_FLOAT, "push_float", true, OPERAND_FLOAT},
	
	{OP_POP, "pop", false, OPERAND_NONE},
	{OP_DUP, "dup", false, OPERAND_NONE},
	
	{OP_PLUS_INT, "plus_int", false, OPERAND_NONE},
	{OP_MINUS_INT, "minus_int", false, OPERAND_NONE},
	{OP_MULT_INT, "mult_int", false, OPERAND_NONE},
	{OP_DIV_INT, "div_int", false, OPERAND_NONE},

	{OP_PLUS_FLOAT, "plus_float", false, OPERAND_FLOAT},
	{OP_MINUS_FLOAT, "minus_float", false, OPERAND_FLOAT},
	{OP_MULT_FLOAT, "mult_float", false, OPERAND_FLOAT},
	{OP_DIV_FLOAT, "div_float", false, OPERAND_FLOAT},

	{OP_JMP, "jmp", true, OPERAND_INT},
	{OP_JZ, "jz", true, OPERAND_INT},
	{OP_JNZ, "jnz", true, OPERAND_INT},

	{OP_LT_INT, "lt_int", false, OPERAND_NONE},
	{OP_GT_INT, "gt_int", false, OPERAND_NONE},
	{OP_EQ_INT, "eq_int", false, OPERAND_NONE},
	
	{OP_LT_FLOAT, "lt_float", false, OPERAND_NONE},
	{OP_GT_FLOAT, "gt_float", false, OPERAND_NONE},
	{OP_EQ_FLOAT, "eq_float", false, OPERAND_NONE},

	{OP_STORE, "store", true, OPERAND_ADDR},
	{OP_LOAD_INT, "load_int", true, OPERAND_ADDR},
	{OP_LOAD_FLOAT, "load_float", true, OPERAND_ADDR},

	{OP_INT_TO_FLOAT, "int_to_float", false, OPERAND_NONE},
	{OP_FLOAT_TO_INT, "float_to_int", false, OPERAND_NONE},

	//{OP_ALLOC, "alloc", true, OPERAND_INT},
	//{OP_MEM_WRITE, "mem_write", false, OPERAND_NONE},
	//{OP_MEM_FREE, "mem_free", false, OPERAND_NONE},
	//TODO: add stack frames and then add memory alloc free write
	{OP_HALT, "halt", false, OPERAND_NONE},
};


const size_t INSTR_DEFS_COUNT = sizeof(INSTR_DEFS) / sizeof(INSTR_DEFS[0]);


const Instr_def* instr_def_by_name(const char* name){
	for(size_t i = 0; i < INSTR_DEFS_COUNT; i++)
	{
		if(strcmp(INSTR_DEFS[i].name, name)==0)
		{
			return &INSTR_DEFS[i];
		}
	}
	return NULL;
}

const Instr_def* instr_def_by_type(Instr_type type){
	for(size_t i = 0; i < INSTR_DEFS_COUNT; i++)
	{
		if(INSTR_DEFS[i].type == type)
		{
			return &INSTR_DEFS[i];
		}
	}
	return NULL;
}






