#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "lexer.h"

static char *ops[] = {
	"mov", "movb", "movw", "movl", "movq", "movabs", "movbe", "movbew",
	"movbel", "movbeq", "movs", "movsb", "movsw", "movsl", "movsq", "movsx",
	"movsxb", "movsxw", "movsxl", "movsbw", "movsbl", "movsbq", "movswl", "movswq",
	"movslq", "movzx", "movzxb", "movzxw", "movzxl", "movzbw", "movzbl", "movzbq",
	"movzwl", "movzwq", "add", "addb", "addw", "addl", "addq", "adc",
	"adcb", "adcw", "adcl", "adcq", "sub", "subb", "subw", "subl",
	"subq", "sbb", "sbbb", "sbbw", "sbbl", "sbbq", "mul", "mulb",
	"mulw", "mull", "mulq", "imul", "imulb", "imulw", "imull", "imulq",
	"div", "divb", "divw", "divl", "divq", "idiv", "idivb", "idivw",
	"idivl", "idivq", "inc", "incb", "incw", "incl", "incq", "dec",
	"decb", "decw", "decl", "decq", "neg", "negb", "negw", "negl",
	"negq", "cmp", "cmpb", "cmpw", "cmpl", "cmpq", "test", "testb",
	"testw", "testl", "testq", "and", "andb", "andw", "andl", "andq",
	"or", "orb", "orw", "orl", "orq", "xor", "xorb", "xorw",
	"xorl", "xorq", "not", "notb", "notw", "notl", "notq", "shl",
	"shlb", "shlw", "shll", "shlq", "shr", "shrb", "shrw", "shrl",
	"shrq", "sal", "salb", "salw", "sall", "salq", "sar", "sarb",
	"sarw", "sarl", "sarq", "rol", "rolb", "rolw", "roll", "rolq",
	"ror", "rorb", "rorw", "rorl", "rorq", "rcl", "rclb", "rclw",
	"rcll", "rclq", "rcr", "rcrb", "rcrw", "rcrl", "rcrq", "shld",
	"shldw", "shldl", "shldq", "shrd", "shrdw", "shrdl", "shrdq", "bt",
	"btw", "btl", "btq", "btc", "btcw", "btcl", "btcq", "btr",
	"btrw", "btrl", "btrq", "bts", "btsw", "btsl", "btsq", "bsf",
	"bsfw", "bsfl", "bsfq", "bsr", "bsrw", "bsrl", "bsrq", "bswap",
	"lzcnt", "lzcntw", "lzcntl", "lzcntq", "tzcnt", "tzcntw", "tzcntl", "tzcntq",
	"popcnt", "popcntw", "popcntl", "popcntq", "jmp", "jmpw", "jmpl", "jmpq",
	"call", "callw", "calll", "callq", "ret", "retw", "retl", "retq",
	"ja", "jae", "jb", "jbe", "jc", "je", "jg", "jge",
	"jl", "jle", "jna", "jnae", "jnb", "jnbe", "jnc", "jne",
	"jng", "jnge", "jnl", "jnle", "jno", "jnp", "jns", "jnz",
	"jo", "jp", "jpe", "jpo", "js", "jz", "jcxz", "jecxz",
	"jrcxz", "loop", "loope", "loopne", "loopnz", "loopz", "seta", "setae",
	"setb", "setbe", "setc", "sete", "setg", "setge", "setl", "setle",
	"setna", "setnae", "setnb", "setnbe", "setnc", "setne", "setng", "setnge",
	"setnl", "setnle", "setno", "setnp", "setns", "setnz", "seto", "setp",
	"setpe", "setpo", "sets", "setz", "cmova", "cmovae", "cmovb", "cmovbe",
	"cmovc", "cmove", "cmovg", "cmovge", "cmovl", "cmovle", "cmovna", "cmovnae",
	"cmovnb", "cmovnbe", "cmovnc", "cmovne", "cmovng", "cmovnge", "cmovnl", "cmovnle",
	"cmovno", "cmovnp", "cmovns", "cmovnz", "cmovo", "cmovp", "cmovpe", "cmovpo",
	"cmovs", "cmovz", "push", "pushw", "pushl", "pushq", "pop", "popw",
	"popl", "popq", "pushf", "pushfw", "pushfl", "pushfq", "popf", "popfw",
	"popfl", "popfq", "lea", "leaw", "leal", "leaq", "xchg", "xchgb",
	"xchgw", "xchgl", "xchgq", "xadd", "xaddb", "xaddw", "xaddl", "xaddq",
	"cmpxchg", "cmpxchgb", "cmpxchgw", "cmpxchgl", "cmpxchgq", "cmpxchg8b", "cmpxchg16b", "nop",
	"pause", "hlt", "clc", "stc", "cmc", "cld", "std", "cli",
	"sti", "lahf", "sahf", "cwd", "cdq", "cqo", "cbw", "cwde",
	"cdqe", "aaa", "aad", "aam", "aas", "daa", "das", "enter",
	"leave", "leavew", "leavel", "leaveq", "int", "into", "iret", "iretw",
	"iretl", "iretq", "syscall", "sysenter", "sysexit", "sysret", "cpuid", "rdtsc",
	"rdtscp", "rdpmc", "lfence", "mfence", "sfence", "lock", "wait", "ud2",
	"in", "inb", "inw", "inl", "out", "outb", "outw", "outl",
	"ins", "insb", "insw", "insl", "outs", "outsb", "outsw", "outsl",
	"lods", "lodsb", "lodsw", "lodsl", "lodsq", "stos", "stosb", "stosw",
	"stosl", "stosq", "scas", "scasb", "scasw", "scasl", "scasq", "cmps",
	"cmpsb", "cmpsw", "cmpsl", "cmpsq", "xlat", "xlatb", "bound", "arpl",
	"lar", "lsl", "lgdt", "lidt", "sgdt", "sidt", "lldt", "sldt",
	"ltr", "str", "lmsw", "smsw", "clts", "verr", "verw", "wbinvd",
};

#define OPS_SIZE sizeof(ops) / sizeof(ops[0])

static char *regs[] = {
	"%al", "%ah", "%ax", "%eax", "%rax", "%bl", "%bh", "%bx",
	"%ebx", "%rbx", "%cl", "%ch", "%cx", "%ecx", "%rcx", "%dl",
	"%dh", "%dx", "%edx", "%rdx", "%sil", "%si", "%esi", "%rsi",
	"%dil", "%di", "%edi", "%rdi", "%bpl", "%bp", "%ebp", "%rbp",
	"%spl", "%sp", "%esp", "%rsp", "%ip", "%eip", "%rip",
	"%r8b", "%r8w", "%r8d", "%r8", "%r9b", "%r9w", "%r9d", "%r9",
	"%r10b", "%r10w", "%r10d", "%r10", "%r11b", "%r11w", "%r11d", "%r11",
	"%r12b", "%r12w", "%r12d", "%r12", "%r13b", "%r13w", "%r13d", "%r13",
	"%r14b", "%r14w", "%r14d", "%r14", "%r15b", "%r15w", "%r15d", "%r15",
	"%cs", "%ds", "%es", "%fs", "%gs", "%ss",
	"%cr0", "%cr1", "%cr2", "%cr3", "%cr4", "%cr5", "%cr6", "%cr7", "%cr8",
	"%dr0", "%dr1", "%dr2", "%dr3", "%dr4", "%dr5", "%dr6", "%dr7",
};

#define REGS_SIZE sizeof(regs) / sizeof(regs[0])

struct reg_map {
	char *name;
	Reg reg;
};

static struct reg_map reg_maps[] = {
	{"AH", AH}, {"AL", AL}, {"BH", BH}, {"BL", BL}, {"CH", CH}, {"CL", CL}, {"DH", DH}, {"DL", DL},
	{"SIL", SIL}, {"DIL", DIL}, {"BPL", BPL}, {"SPL", SPL},
	{"R8B", R8B}, {"R9B", R9B}, {"R10B", R10B}, {"R11B", R11B}, {"R12B", R12B}, {"R13B", R13B}, {"R14B", R14B}, {"R15B", R15B},
	{"AX", AX}, {"BX", BX}, {"CX", CX}, {"DX", DX}, {"SI", SI}, {"DI", DI}, {"BP", BP}, {"SP", SP},
	{"R8W", R8W}, {"R9W", R9W}, {"R10W", R10W}, {"R11W", R11W}, {"R12W", R12W}, {"R13W", R13W}, {"R14W", R14W}, {"R15W", R15W},
	{"EAX", EAX}, {"EBX", EBX}, {"ECX", ECX}, {"EDX", EDX}, {"ESI", ESI}, {"EDI", EDI}, {"EBP", EBP}, {"ESP", ESP},
	{"R8D", R8D}, {"R9D", R9D}, {"R10D", R10D}, {"R11D", R11D}, {"R12D", R12D}, {"R13D", R13D}, {"R14D", R14D}, {"R15D", R15D},
	{"RAX", RAX}, {"RBX", RBX}, {"RCX", RCX}, {"RDX", RDX}, {"RSI", RSI}, {"RDI", RDI}, {"RBP", RBP}, {"RSP", RSP},
	{"R8", R8}, {"R9", R9}, {"R10", R10}, {"R11", R11}, {"R12", R12}, {"R13", R13}, {"R14", R14}, {"R15", R15},
	{"IP", IP}, {"EIP", EIP}, {"RIP", RIP},
	{"CS", CS}, {"DS", DS}, {"ES", ES}, {"FS", FS}, {"GS", GS}, {"SS", SS},
	{"CR0", CR0}, {"CR1", CR1}, {"CR2", CR2}, {"CR3", CR3}, {"CR4", CR4}, {"CR5", CR5}, {"CR6", CR6}, {"CR7", CR7}, {"CR8", CR8},
	{"DR0", DR0}, {"DR1", DR1}, {"DR2", DR2}, {"DR3", DR3}, {"DR4", DR4}, {"DR5", DR5}, {"DR6", DR6}, {"DR7", DR7},
};

#define REG_MAPS_SIZE sizeof(reg_maps) / sizeof(reg_maps[0])

static DFA_state is_space(char *t)
{
	char *p = t;
	enum state {
		a,
		b
	} s;
	s = a;
	for (;;) {
		switch (s) {
			case a:
				if (IS_SPACE(*p))
					s = b;
				else
					return MISMATCH;
				p++;
				break;
			case b:
				if (IS_SPACE(*p))
					s = b;
				else if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				p++;
				break;
		}
	}
}

DFA_state is_reg(char *t)
{
    for (int i = 0; i < REGS_SIZE; i++) {
        size_t len_t = strlen(t);
        size_t len_k = strlen(regs[i]);

        if (strncmp(t, regs[i], len_t) == 0) {
            if (len_t == len_k)
                return MATCH;
            else
                return PENDING;
        }
    }
    return MISMATCH;
}

DFA_state is_op(char *t)
{
    for (int i = 0; i < OPS_SIZE; i++) {
        size_t len_t = strlen(t);
        size_t len_k = strlen(ops[i]);

        if (strncasecmp(t, ops[i], len_t) == 0) {
            if (len_t == len_k)
                return MATCH;
            else
                return PENDING;
        }
    }
    return MISMATCH;
}

static DFA_state is_num(char *t)
{
	char *p = t;
	enum state {
		start,
		sign,
		digit
	} s;
	s = start;
	for (;;) {
		switch (s) {
			case start:
				if (IS_DIGIT(*p))
					s = digit;
				else if ((*p) == '+' || (*p) == '-')
					s = sign;
				else
					return MISMATCH;
				p++;
				break;
			case sign:
				if (IS_DIGIT(*p))
					s = digit;
				else
					return MISMATCH;
				p++;
				break;
			case digit:
				if (IS_DIGIT(*p))
					s = digit;
				else if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				p++;
				break;
		}
	}
}

static DFA_state is_imm(char *t)
{
	char *p = t;
	enum state {
		start,
		prefix,
		sign,
		digit
	} s;
	s = start;
	for (;;) {
		switch (s) {
			case start:
				if (*p == '$')
					s = prefix;
				else
					return MISMATCH;
				p++;
				break;
			case prefix:
				if (IS_DIGIT(*p))
					s = digit;
				else if ((*p) == '+' || (*p) == '-')
					s = sign;
				else if (IS_EOL(*p))
					return PENDING;
				else
					return MISMATCH;
				p++;
				break;
			case sign:
				if (IS_DIGIT(*p))
					s = digit;
				else
					return MISMATCH;
				p++;
				break;
			case digit:
				if (IS_DIGIT(*p))
					s = digit;
				else if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				p++;
				break;
		}
	}
}

static DFA_state is_paren(char *t)
{
	int len = strlen(t);
	if (len > 1)
		return MISMATCH;
	if (len == 1) {
		if (t[0] == '(')
				return MATCH;
			else
				return MISMATCH;
	}
}

static DFA_state is_cparen(char *t)
{
	int len = strlen(t);
	if (len > 1)
		return MISMATCH;
	if (len == 1) {
		if (t[0] == ')')
				return MATCH;
			else
				return MISMATCH;
	}
}

static DFA_state is_semicolon(char *t)
{
	int len = strlen(t);
	if (len > 1)
		return MISMATCH;
	if (len == 1) {
		if (t[0] == ';')
				return MATCH;
			else
				return MISMATCH;
	}
}

static DFA_state is_comma(char *t)
{
	int len = strlen(t);
	if (len > 1)
		return MISMATCH;
	if (len == 1) {
		if (t[0] == ',')
				return MATCH;
			else
				return MISMATCH;
	}
}

static bool possible(DFA_state *s)
{
	for (TokenType i = TOKEN_TYPE_NULL+1; i < TOKEN_TYPE_END; i++) {
		if (s[i] != MISMATCH)
			return true;
	}
	return false;
}

static Literal literal(TokenType t, char *lexeme)
{
	Literal r;
	memset(&r, 0, sizeof(r));
	switch (t) {
		case OP_TOKEN:
			for (int i = 0; i < OPS_SIZE; i++) {
				if (strcasecmp(lexeme, ops[i]) == 0) {
					r.op = (Op)(i + 1);
					return r;
				}
			}
			return r;
		case NUM_TOKEN:
			r.numValue = atoi(lexeme);
			return r;
		case IMM_TOKEN:
			r.numValue = atoi(lexeme+1);
			return r;
		case REG_TOKEN:
			for (int i = 0; i < REG_MAPS_SIZE; i++) {
				if (strcasecmp(lexeme+1, reg_maps[i].name) == 0) {
					r.reg = reg_maps[i].reg;
					return r;
				}
			}
			return r;
		default:
			return r;
	}
}

int lexer(char *bf, Token *tokens, int lineNum)
{
	int col = 0;
	char *bfend = bf;
	while(*bfend != '\0') bfend++;
	if (bfend == bf) return 0;
	DFA_state states[TOKEN_TYPE_END];
	struct longest_match {
		char *end;
		size_t len;
		TokenType type;
	} m;
	
	char *start = bf;
	char *end = bf;
	size_t tokenslen = 0;
	m.end = bf;
next:
	m.len = 0;
	m.type = TOKEN_TYPE_NULL;
	for (TokenType i = TOKEN_TYPE_NULL+1; i < TOKEN_TYPE_END; i++)
    	states[i] = PENDING;
	char t[BFSIZE];
	memset(t, 0, sizeof(t));
	size_t tlen = 0;
	if (start >= bfend)
		return tokenslen;
	while (possible(states) && end < bfend) {
		end++;
		tlen++;
		if (tlen >= BFSIZE - 2) {
			fprintf(stderr, "token at %d,%d is too long: `%.20s`\n", lineNum, col, t);
			return -1;
		}
		strncpy(t, start, tlen);
		for (TokenType i = TOKEN_TYPE_NULL+1; i < TOKEN_TYPE_END; i++) {
			switch (i) {
				case REG_TOKEN:
					if (states[REG_TOKEN] != MISMATCH)
						states[REG_TOKEN] = is_reg(t);
					break;
				case NUM_TOKEN:
					if (states[NUM_TOKEN] != MISMATCH)
						states[NUM_TOKEN] = is_num(t);
					break;
				case IMM_TOKEN:
					if (states[IMM_TOKEN] != MISMATCH)
						states[IMM_TOKEN] = is_imm(t);
					break;
				case OP_TOKEN:
					if (states[OP_TOKEN] != MISMATCH)
						states[OP_TOKEN] = is_op(t);
					break;
				case COMMA_TOKEN:
					if (states[COMMA_TOKEN] != MISMATCH)
						states[COMMA_TOKEN] = is_comma(t);
					break;
				case PAREN_TOKEN:
					if (states[PAREN_TOKEN] != MISMATCH)
						states[PAREN_TOKEN] = is_paren(t);
					break;
				case CPAREN_TOKEN:
					if (states[CPAREN_TOKEN] != MISMATCH)
						states[CPAREN_TOKEN] = is_cparen(t);
					break;
				case SPACE_TOKEN:
					if (states[SPACE_TOKEN] != MISMATCH)
						states[SPACE_TOKEN] = is_space(t);
					break;
			}
		}
		for (TokenType i = TOKEN_TYPE_NULL+1; i < TOKEN_TYPE_END; i++) {
			if (states[i] == MATCH) {
				if (tlen >= m.len) {
					m.len = tlen;
					m.end = end;
					m.type = i;
					break;
				}
			}
		}
	}
	if (m.type != TOKEN_TYPE_NULL) {
		if (m.type != SPACE_TOKEN) {
			strncpy(tokens[tokenslen].lexeme, start, m.len);
			tokens[tokenslen].lexeme[m.len] = '\0';
			tokens[tokenslen].literal = literal(m.type, tokens[tokenslen].lexeme);
			tokens[tokenslen].type = m.type;
			tokens[tokenslen].lineNum = lineNum;
			tokens[tokenslen].colNum = col;
			tokenslen++;
		}
		start = m.end;
		end = start;
		col = start - bf;
		m.type = TOKEN_TYPE_NULL;
		m.end = start;
		m.len = 0;
	} else {
		fprintf(stderr, "unexpected token at %d,%d: `%s`\n", lineNum, col, t);
		return -1;
	}
	goto next;

	return tokenslen;
}
