#include <stdint.h>
#include "lexer.h"

typedef struct {
	int64_t disp;
	Reg base;
	Reg index;
	uint8_t scale;
} Mem;

typedef struct {
	enum {REG, MEM, IMM} type;
	union {
		Reg reg;
		int64_t imm;
		Mem mem;
	} val;
} Arg;

typedef struct {
    Op op;
	Arg src;
	Arg dest;
} Instruction;

Instruction parse(Token *tokens, size_t len);