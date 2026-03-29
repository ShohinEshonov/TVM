#include "parser.h"
#include "../isa/isa_table.h"
#include "dynamic_array.h"
#include "lexer.h"
#include "token_types.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(xs) sizeof(xs) / sizeof(xs[0])

struct {
  char *mnem;
  OP_CODE op_code;
} mnemonic_to_isa[] = {
    {"push_i", OP_PUSH_INT},
    {"push_f", OP_PUSH_FLOAT},

    {"pop", OP_POP},
    {"dup", OP_DUP},

    {"plus_i", OP_PLUS_INT},
    {"plus_f", OP_PLUS_FLOAT},
    {"minus_i", OP_MINUS_INT},
    {"minus_f", OP_MINUS_FLOAT},
    {"mult_i", OP_MULT_INT},
    {"mult_f", OP_MULT_FLOAT},
    {"div_i", OP_DIV_INT},
    {"div_f", OP_DIV_FLOAT},

    {"jmp", OP_JMP},
    {"jz", OP_JZ},
    {"jnz", OP_JNZ},

    {"lt_i", OP_LT_INT},
    {"lt_f", OP_LT_FLOAT},
    {"gt_i", OP_GT_INT},
    {"gt_f", OP_GT_FLOAT},
    {"eq_i", OP_EQ_INT},
    {"eq_f", OP_EQ_FLOAT},

    {"store_global", OP_STORE_GLOBAL},
    {"load_global", OP_LOAD_GLOBAL},

    {"i2f", OP_INT_TO_FLOAT},
    {"f2i", OP_FLOAT_TO_INT},

    {"call", OP_CALL},
    {"return", OP_RET},

    {"load_arg", OP_LOAD_ARG},
    {"load_local", OP_LOAD_LOCAL},
    {"store_local", OP_STORE_LOCAL},

    {"halt", OP_HALT},
};

static Symbol *find_lexeme_table(DynamicArray *symbol_table, char *lexeme) {
  for (size_t i = 0; i < symbol_table->length; i++) {
    Symbol *symbol = (Symbol *)get_element(symbol_table, i);
    char *lexeme_table =
        strndup(symbol->token->lexeme.start, symbol->token->lexeme.length);
    if (strcmp(lexeme, lexeme_table) == 0) {
      free(lexeme_table);
      return symbol;
    }
    free(lexeme_table);
  }
  return NULL;
}

static bool is_lexeme_in_table(Parser *parser, char *lexeme) {
  for (size_t i = 0; i < parser->symbol_table.length; i++) {
    Symbol *symbol = (Symbol *)get_element(&parser->symbol_table, i);
    char *lexeme_table =
        strndup(symbol->token->lexeme.start, symbol->token->lexeme.length);
    if (strcmp(lexeme, lexeme_table) == 0) {
      free(lexeme_table);
      return true;
    }
    free(lexeme_table);
  }
  return false;
}

static Token *next_token(Parser *parser) {
  Token *t = get_element(&parser->tokens, parser->current);
  parser->current++;
  return t;
}

static TokenType peek(Parser *parser) {
  Token *token = (Token *)get_element(&parser->tokens, parser->current);
  return token->type;
}

static void eat_token(Parser *parser) { parser->current++; }

static Symbol *parser_identifier(Token *t, Parser *parser) {
  char *lexeme_identifier = strndup(t->lexeme.start, t->lexeme.length);
  if (is_lexeme_in_table(parser, lexeme_identifier)) {
    Symbol *symbol =
        find_lexeme_table(&parser->symbol_table, lexeme_identifier);
    free(lexeme_identifier);
    return symbol;
  } else {
    fprintf(stderr, "Undefined idnetifier: %s", lexeme_identifier);
    free(lexeme_identifier);
    exit(1);
  }
}

static bool parser_number(char *str, Operand_type expected_type, Value *out) {

  if (expected_type == TYPE_F64) {
    char *end;
    errno = 0;
    double value = strtod(str, &end);

    if (errno == ERANGE || *end != '\0') {
      fprintf(stderr, "Error: invalid float %s", str);
      return false;
    }
    out->type = VAL_F64;
    out->f64 = (double)value;
    return true;
  }

  else {
    char *end;
    errno = 0;
    int64_t value = strtoll(str, &end, 10);

    if (errno == ERANGE || *end != '\0') {
      fprintf(stderr, "Error: Invalid number %s", str);
      return false;
    }

    switch (expected_type) {
    case TYPE_I64:
      out->type = VAL_I64;
      out->i64 = value;
      break;

    case TYPE_U16:
      if (value < 0 || value > 65535) {
        fprintf(stderr, "Error: %ld doesnt fit in U8 type.", value);
        return false;
      }
      out->type = VAL_U16;
      out->u16 = (uint16_t)value;
      break;
    case TYPE_U8:
      if (value < 0 || value > 255) {
        fprintf(stderr, "Error: %ld doesnt fit in U8 type.", value);
        return false;
      }
      out->type = VAL_U16;
      out->u16 = (uint8_t)value;
      break;
    default:
      break;
    }
  }

  return true;
}

static void parse_mnemonic(Token *token, Parser *parser) {
  char *instruction_str = strndup(token->lexeme.start, token->lexeme.length);
  size_t instruction_opcode = -1;

  for (size_t i = 0; i < ARRAY_SIZE(mnemonic_to_isa); i++) {
    if (strcmp(instruction_str, mnemonic_to_isa[i].mnem) == 0) {
      instruction_opcode = mnemonic_to_isa[i].op_code;
      break;
    }
  }
  free(instruction_str);

  const Instr_def *instr_def = instr_def_by_type(instruction_opcode);

  if (instr_def->operand_count > 0) {
    Instruction instr = {.opcode = instruction_opcode,
                         .operand_count = instr_def->operand_count};

    for (int i = 0; i < instr_def->operand_count; i++) {
      switch (instr_def->operand_type[i]) {
      case TYPE_I64: {
        TokenType next = peek(parser);
        if (next == TOKEN_INT) {
          Token *operand = next_token(parser);
          char *operand_str =
              strndup(operand->lexeme.start, operand->lexeme.length);
          if (!parser_number(operand_str, TYPE_I64, &instr.operands[i])) {
            free(operand_str);
            exit(1);
          }
          free(operand_str);
        } else {
          fprintf(stderr, "Unexpected type, expected int\n");
          exit(1);
        }
        break;
      }
      case TYPE_U64: {
        TokenType next = peek(parser);
        if (next == TOKEN_INT) {
          Token *operand = next_token(parser);
          char *operand_str =
              strndup(operand->lexeme.start, operand->lexeme.length);
          if (!parser_number(operand_str, TYPE_U64, &instr.operands[i])) {
            free(operand_str);
            exit(1);
          }
          free(operand_str);
        } else if (next == TOKEN_IDENTIFIER) {
          Token *identifier = next_token(parser);
          Symbol *symbol_identifier = parser_identifier(identifier, parser);
          instr.operands[i].type = VAL_U64;
          instr.operands[i].u64 = symbol_identifier->location;
        } else {
          fprintf(stderr, "Unexpected type, expected int or identifier\n");
          exit(1);
        }
        break;
      }
      case TYPE_U16: {
        TokenType next = peek(parser);
        if (next == TOKEN_INT) {
          Token *operand = next_token(parser);
          char *operand_str =
              strndup(operand->lexeme.start, operand->lexeme.length);
          if (!parser_number(operand_str, TYPE_U16, &instr.operands[i])) {
            free(operand_str);
            exit(1);
          }
          free(operand_str);
        } else {
          fprintf(stderr, "Unexpected type, expected int\n");
          exit(1);
        }
        break;
      }
      case TYPE_U8: {
        TokenType next = peek(parser);
        if (next == TOKEN_INT) {
          Token *operand = next_token(parser);
          char *operand_str =
              strndup(operand->lexeme.start, operand->lexeme.length);
          if (!parser_number(operand_str, TYPE_U8, &instr.operands[i])) {
            free(operand_str);
            exit(1);
          }
          free(operand_str);
        } else {
          fprintf(stderr, "Unexpected type, expected int\n");
          exit(1);
        }
        break;
      }
      case TYPE_F64: {
        TokenType next = peek(parser);
        if (next == TOKEN_FLOAT || next == TOKEN_INT) {
          Token *operand = next_token(parser);
          char *operand_str =
              strndup(operand->lexeme.start, operand->lexeme.length);
          if (!parser_number(operand_str, TYPE_F64, &instr.operands[i])) {
            free(operand_str);
            exit(1);
          }
          free(operand_str);
        } else {
          fprintf(stderr, "Unexpected type, expected float\n");
          exit(1);
        }
        break;
      }
      }
    }

    parser->program[parser->ic] = instr;
  } else {
    Instruction instr = {.opcode = instruction_opcode, .operand_count = 0};
    parser->program[parser->ic] = instr;
  }
}

static void parse_label(Token *t, Parser *parser) {
  char *symbol_lexeme = strndup(t->lexeme.start, t->lexeme.length);
  if (is_lexeme_in_table(parser, symbol_lexeme)) {
    fprintf(stderr, "Second define of 1 label %s", symbol_lexeme);
    free(symbol_lexeme);
    exit(1);
  } else {
    Symbol sym = {
        .token = t, .location = parser->bc, .symbol_type = SYMBOL_LABEL};
    add_element(&parser->symbol_table, &sym);
  }
  free(symbol_lexeme);
}

static void parse_token(Parser *parser) {

  Token *t = next_token(parser);

  switch (t->type) {
  case (TOKEN_MNEMONIC):
    parse_mnemonic(t, parser);
    parser->ic++;
    break;

  case (TOKEN_IDENTIFIER):
    if (peek(parser) == TOKEN_COLON) {
      eat_token(parser);
    }
    break;

  case (TOKEN_EOF):
    break;
  default: {
    fprintf(stderr, "Unexpected token\n");
    exit(1);
  }
  }
}

static bool is_at_end(Parser *parser) {
  Token *t = get_element(&parser->tokens, parser->current);
  return t->type == TOKEN_EOF;
}

void first_pass(Parser *parser) {
  while (!is_at_end(parser)) {
    Token *t = next_token(parser);

    if (t->type == TOKEN_IDENTIFIER) {
      if (peek(parser) == TOKEN_COLON) {
        parse_label(t, parser);
        eat_token(parser);
      } else {
        add_element(&parser->unsolved_refs, t);
      }
    } else if (t->type == TOKEN_MNEMONIC) {
        char *lexeme = strndup(t->lexeme.start, t->lexeme.length);
        for(size_t i = 0; i < ARRAY_SIZE(mnemonic_to_isa); i++)
        {
          if(strcmp(lexeme, mnemonic_to_isa[i].mnem) == 0)
          {
            const Instr_def *instr_def  = instr_def_by_type(mnemonic_to_isa[i].op_code); \
            parser->bc+=instr_def->byte_size;
            for(int j = 0; j < instr_def->operand_count; j++)
            {
              eat_token(parser);
            }
            break;
          }
        } 
      parser->ic++;
      free(lexeme);
    }
  }
}

void solve_refs(Parser *parser) {
  for (size_t i = 0; i < parser->unsolved_refs.length; i++) {
    Token *ref = (Token *)get_element(&parser->unsolved_refs, i);
    char *name = strndup(ref->lexeme.start, ref->lexeme.length);

    if (!is_lexeme_in_table(parser, name)) {
      fprintf(stderr, "Undefined identifier %s", name);
      free(name);
      exit(1);
    }
    free(name);
  }
}

void parse_tokens(Parser *parser) {
  if (parser->first_pass == false) {
    first_pass(parser);
    solve_refs(parser);
    parser->first_pass = true;
    parser->current = 0;
  }
  parser->program = malloc(parser->ic * sizeof(Instruction));
  if (parser->program == NULL) {
    perror("Cannot allocate memory for parser");
    exit(1);
  }
  parser->ic = 0;
  while (!is_at_end(parser)) {
    parse_token(parser);
  }
  free_array(&parser->unsolved_refs);
}

Parser *init_parser(DynamicArray *tokens) {
  Parser *parser = malloc(sizeof(Parser));
  if (!parser) {
    perror("Cannot allocate parser");
    exit(1);
  }
  parser->tokens = *tokens;
  // init_array(&parser->program, sizeof(Instruction), 16);
  init_array(&parser->symbol_table, sizeof(Symbol), 5);
  init_array(&parser->unsolved_refs, sizeof(Token), 16);
  parser->program = NULL;
  parser->current = 0;
  parser->ic = 0;
  parser->bc = 0;
  parser->first_pass = false;
  return parser;
}
