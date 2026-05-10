#include <stdio.h>
#include <stdint.h>
#include "parser.h"

#define CONSUME p++; l++; if (l > len) return ins

Instruction parse(Token *tokens, size_t len)
{
	size_t l = 0;
	Op op;
	Instruction ins = {
		.op = NULL_OP
	};
	Token *p = tokens;
	if (p->type != OP_TOKEN) {
		return ins;
	} else {
		op = p->literal.op;
		CONSUME;
		if (p->type == REG_TOKEN) {
			ins.src.type = REG;
			ins.src.val.reg = p->literal.reg;
			CONSUME;
		}
		else if (p->type == IMM_TOKEN) {
			ins.src.type = IMM;
			ins.src.val.imm = p->literal.numValue;
			CONSUME;
		}
		else {
			ins.src.type = MEM;
			if (p->type == NUM_TOKEN) {
				ins.src.val.mem.disp = p->literal.numValue;
				CONSUME;
			} else {
				return ins;
			}
			if (p->type == PAREN_TOKEN) {
				CONSUME;
				if (p->type == REG_TOKEN) {
					ins.src.val.mem.base = p->literal.reg;
					CONSUME;
					if (p->type == COMMA_TOKEN) {
						CONSUME;
					}
					else if (p->type == CPAREN_TOKEN) {
						CONSUME;
						goto skip1;
					} else {
						return ins;
					}
				} else {
					return ins;
				}
				if (p->type == REG_TOKEN) {
					ins.src.val.mem.index = p->literal.reg;
					CONSUME;
					if (p->type == COMMA_TOKEN) {
						CONSUME;
					} else if (p->type == CPAREN_TOKEN) {
						CONSUME;
						goto skip1;
					} else {
						return ins;
					}
				} else {
					return ins;
				}
				if (p->type == NUM_TOKEN) {
					ins.src.val.mem.scale = p->literal.numValue;
					CONSUME;
				} else {
					return ins;
				}

				if (p->type == CPAREN_TOKEN) {
					CONSUME;
				} else {
					return ins;
				}
			} else {
				return ins;
			}
		}
skip1:

		if (p->type == COMMA_TOKEN) {
			CONSUME;
		} else {
			return ins;
		}


		if (p->type == REG_TOKEN) {
			ins.dest.type = REG;
			ins.dest.val.reg = p->literal.reg;
			CONSUME;
		}
		else if (p->type == IMM_TOKEN) {
			ins.dest.type = IMM;
			ins.dest.val.imm = p->literal.numValue;
			CONSUME;
		}
		else {
			ins.dest.type = MEM;
			if (p->type == NUM_TOKEN) {
				ins.dest.val.mem.disp = p->literal.numValue;
				CONSUME;
			} else {
				return ins;
			}
			if (p->type == PAREN_TOKEN) {
				CONSUME;
				if (p->type == REG_TOKEN) {
					ins.dest.val.mem.base = p->literal.reg;
					CONSUME;
					if (p->type == COMMA_TOKEN) {
						CONSUME;
					} else if (p->type == CPAREN_TOKEN) {
						CONSUME;
						goto skip2;
					} else {
						return ins;
					}
				} else {
					return ins;
				}
				if (p->type == REG_TOKEN) {
					ins.dest.val.mem.index = p->literal.reg;
					CONSUME;
					if (p->type == COMMA_TOKEN) {
						CONSUME;
					} else if (p->type == CPAREN_TOKEN) {
						CONSUME;
						goto skip2;
					} else {
						return ins;
					}
				} else {
					return ins;
				}
				if (p->type == NUM_TOKEN) {
					ins.dest.val.mem.scale = p->literal.numValue;
					CONSUME;
				} else {
					return ins;
				}

				if (p->type == CPAREN_TOKEN) {
					CONSUME;
				} else {
					return ins;
				}
			} else {
				return ins;
			}
		}
	}
skip2:
	if (l < len) return ins;

	ins.op = op;
	return ins;
}