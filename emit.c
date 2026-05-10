#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "parser.h"

#define MAX_PREFIXES 4
#define MAX_OPCODE   3
#define MAX_OPCODE_LEN 15

typedef struct {
    uint8_t prefixes[MAX_PREFIXES];
    uint8_t prefix_len;
    bool has_rex;
    uint8_t rex;
    uint8_t opcode[MAX_OPCODE];
    uint8_t opcode_len;
    bool has_modrm;
    uint8_t modrm;
    bool has_sib;
    uint8_t sib;
    uint8_t disp[4];
    uint8_t disp_len;
    uint8_t imm[8];
    uint8_t imm_len;
} Opcode;

#define OPCODE1(b0) \
    { .opcode = { (b0) }, .opcode_len = 1 }
#define OPCODE2(b0, b1) \
    { .opcode = { (b0), (b1) }, .opcode_len = 2 }
#define OPCODE3(b0, b1, b2) \
    { .opcode = { (b0), (b1), (b2) }, .opcode_len = 3 }

static const Opcode op_opcode_map[OP_END] = {
    [MOV] = OPCODE1(0x89),
    [MOVB] = OPCODE1(0x88),
    [MOVW] = OPCODE1(0x89),
    [MOVL] = OPCODE1(0x89),
    [MOVQ] = OPCODE1(0x89),
    [MOVABS] = OPCODE1(0xb8),
    [MOVBE] = OPCODE2(0x0f, 0x38),
    [MOVBEW] = OPCODE3(0x0f, 0x38, 0xf0),
    [MOVBEL] = OPCODE3(0x0f, 0x38, 0xf0),
    [MOVBEQ] = OPCODE3(0x0f, 0x38, 0xf0),
    [MOVS] = OPCODE1(0xa5),
    [MOVSB] = OPCODE1(0xa4),
    [MOVSW] = OPCODE1(0xa5),
    [MOVSL] = OPCODE1(0xa5),
    [MOVSQ] = OPCODE1(0xa5),
    [MOVSX] = OPCODE2(0x0f, 0xbf),
    [MOVSXB] = OPCODE2(0x0f, 0xbe),
    [MOVSXW] = OPCODE2(0x0f, 0xbf),
    [MOVSXL] = OPCODE1(0x63),
    [MOVSBW] = OPCODE2(0x0f, 0xbe),
    [MOVSBL] = OPCODE2(0x0f, 0xbe),
    [MOVSBQ] = OPCODE2(0x0f, 0xbe),
    [MOVSWL] = OPCODE2(0x0f, 0xbf),
    [MOVSWQ] = OPCODE2(0x0f, 0xbf),
    [MOVSLQ] = OPCODE1(0x63),
    [MOVZX] = OPCODE2(0x0f, 0xb7),
    [MOVZXB] = OPCODE2(0x0f, 0xb6),
    [MOVZXW] = OPCODE2(0x0f, 0xb7),
    [MOVZXL] = OPCODE2(0x0f, 0xb7),
    [MOVZBW] = OPCODE2(0x0f, 0xb6),
    [MOVZBL] = OPCODE2(0x0f, 0xb6),
    [MOVZBQ] = OPCODE2(0x0f, 0xb6),
    [MOVZWL] = OPCODE2(0x0f, 0xb7),
    [MOVZWQ] = OPCODE2(0x0f, 0xb7),

    [ADD] = OPCODE1(0x01), [ADDB] = OPCODE1(0x00), [ADDW] = OPCODE1(0x01), [ADDL] = OPCODE1(0x01), [ADDQ] = OPCODE1(0x01),
    [ADC] = OPCODE1(0x11), [ADCB] = OPCODE1(0x10), [ADCW] = OPCODE1(0x11), [ADCL] = OPCODE1(0x11), [ADCQ] = OPCODE1(0x11),
    [SUB] = OPCODE1(0x29), [SUBB] = OPCODE1(0x28), [SUBW] = OPCODE1(0x29), [SUBL] = OPCODE1(0x29), [SUBQ] = OPCODE1(0x29),
    [SBB] = OPCODE1(0x19), [SBBB] = OPCODE1(0x18), [SBBW] = OPCODE1(0x19), [SBBL] = OPCODE1(0x19), [SBBQ] = OPCODE1(0x19),
    [CMP] = OPCODE1(0x39), [CMPB] = OPCODE1(0x38), [CMPW] = OPCODE1(0x39), [CMPL] = OPCODE1(0x39), [CMPQ] = OPCODE1(0x39),
    [TEST] = OPCODE1(0x85), [TESTB] = OPCODE1(0x84), [TESTW] = OPCODE1(0x85), [TESTL] = OPCODE1(0x85), [TESTQ] = OPCODE1(0x85),
    [AND] = OPCODE1(0x21), [ANDB] = OPCODE1(0x20), [ANDW] = OPCODE1(0x21), [ANDL] = OPCODE1(0x21), [ANDQ] = OPCODE1(0x21),
    [OR] = OPCODE1(0x09), [ORB] = OPCODE1(0x08), [ORW] = OPCODE1(0x09), [ORL] = OPCODE1(0x09), [ORQ] = OPCODE1(0x09),
    [XOR] = OPCODE1(0x31), [XORB] = OPCODE1(0x30), [XORW] = OPCODE1(0x31), [XORL] = OPCODE1(0x31), [XORQ] = OPCODE1(0x31),

    [MUL] = OPCODE1(0xf7), [MULB] = OPCODE1(0xf6), [MULW] = OPCODE1(0xf7), [MULL] = OPCODE1(0xf7), [MULQ] = OPCODE1(0xf7),
    [IMUL] = OPCODE2(0x0f, 0xaf), [IMULB] = OPCODE1(0xf6), [IMULW] = OPCODE2(0x0f, 0xaf), [IMULL] = OPCODE2(0x0f, 0xaf), [IMULQ] = OPCODE2(0x0f, 0xaf),
    [DIV] = OPCODE1(0xf7), [DIVB] = OPCODE1(0xf6), [DIVW] = OPCODE1(0xf7), [DIVL] = OPCODE1(0xf7), [DIVQ] = OPCODE1(0xf7),
    [IDIV] = OPCODE1(0xf7), [IDIVB] = OPCODE1(0xf6), [IDIVW] = OPCODE1(0xf7), [IDIVL] = OPCODE1(0xf7), [IDIVQ] = OPCODE1(0xf7),
    [INC] = OPCODE1(0xff), [INCB] = OPCODE1(0xfe), [INCW] = OPCODE1(0xff), [INCL] = OPCODE1(0xff), [INCQ] = OPCODE1(0xff),
    [DEC] = OPCODE1(0xff), [DECB] = OPCODE1(0xfe), [DECW] = OPCODE1(0xff), [DECL] = OPCODE1(0xff), [DECQ] = OPCODE1(0xff),
    [NEG] = OPCODE1(0xf7), [NEGB] = OPCODE1(0xf6), [NEGW] = OPCODE1(0xf7), [NEGL] = OPCODE1(0xf7), [NEGQ] = OPCODE1(0xf7),
    [NOT] = OPCODE1(0xf7), [NOTB] = OPCODE1(0xf6), [NOTW] = OPCODE1(0xf7), [NOTL] = OPCODE1(0xf7), [NOTQ] = OPCODE1(0xf7),

    [SHL] = OPCODE1(0xd3), [SHLB] = OPCODE1(0xd2), [SHLW] = OPCODE1(0xd3), [SHLL] = OPCODE1(0xd3), [SHLQ] = OPCODE1(0xd3),
    [SHR] = OPCODE1(0xd3), [SHRB] = OPCODE1(0xd2), [SHRW] = OPCODE1(0xd3), [SHRL] = OPCODE1(0xd3), [SHRQ] = OPCODE1(0xd3),
    [SAL] = OPCODE1(0xd3), [SALB] = OPCODE1(0xd2), [SALW] = OPCODE1(0xd3), [SALL] = OPCODE1(0xd3), [SALQ] = OPCODE1(0xd3),
    [SAR] = OPCODE1(0xd3), [SARB] = OPCODE1(0xd2), [SARW] = OPCODE1(0xd3), [SARL] = OPCODE1(0xd3), [SARQ] = OPCODE1(0xd3),
    [ROL] = OPCODE1(0xd3), [ROLB] = OPCODE1(0xd2), [ROLW] = OPCODE1(0xd3), [ROLL] = OPCODE1(0xd3), [ROLQ] = OPCODE1(0xd3),
    [ROR] = OPCODE1(0xd3), [RORB] = OPCODE1(0xd2), [RORW] = OPCODE1(0xd3), [RORL] = OPCODE1(0xd3), [RORQ] = OPCODE1(0xd3),
    [RCL] = OPCODE1(0xd3), [RCLB] = OPCODE1(0xd2), [RCLW] = OPCODE1(0xd3), [RCLL] = OPCODE1(0xd3), [RCLQ] = OPCODE1(0xd3),
    [RCR] = OPCODE1(0xd3), [RCRB] = OPCODE1(0xd2), [RCRW] = OPCODE1(0xd3), [RCRL] = OPCODE1(0xd3), [RCRQ] = OPCODE1(0xd3),
    [SHLD] = OPCODE2(0x0f, 0xa5), [SHLDW] = OPCODE2(0x0f, 0xa5), [SHLDL] = OPCODE2(0x0f, 0xa5), [SHLDQ] = OPCODE2(0x0f, 0xa5),
    [SHRD] = OPCODE2(0x0f, 0xad), [SHRDW] = OPCODE2(0x0f, 0xad), [SHRDL] = OPCODE2(0x0f, 0xad), [SHRDQ] = OPCODE2(0x0f, 0xad),

    [BT] = OPCODE2(0x0f, 0xa3), [BTW] = OPCODE2(0x0f, 0xa3), [BTL] = OPCODE2(0x0f, 0xa3), [BTQ] = OPCODE2(0x0f, 0xa3),
    [BTC] = OPCODE2(0x0f, 0xbb), [BTCW] = OPCODE2(0x0f, 0xbb), [BTCL] = OPCODE2(0x0f, 0xbb), [BTCQ] = OPCODE2(0x0f, 0xbb),
    [BTR] = OPCODE2(0x0f, 0xb3), [BTRW] = OPCODE2(0x0f, 0xb3), [BTRL] = OPCODE2(0x0f, 0xb3), [BTRQ] = OPCODE2(0x0f, 0xb3),
    [BTS] = OPCODE2(0x0f, 0xab), [BTSW] = OPCODE2(0x0f, 0xab), [BTSL] = OPCODE2(0x0f, 0xab), [BTSQ] = OPCODE2(0x0f, 0xab),
    [BSF] = OPCODE2(0x0f, 0xbc), [BSFW] = OPCODE2(0x0f, 0xbc), [BSFL] = OPCODE2(0x0f, 0xbc), [BSFQ] = OPCODE2(0x0f, 0xbc),
    [BSR] = OPCODE2(0x0f, 0xbd), [BSRW] = OPCODE2(0x0f, 0xbd), [BSRL] = OPCODE2(0x0f, 0xbd), [BSRQ] = OPCODE2(0x0f, 0xbd),
    [BSWAP] = OPCODE1(0xc8),
    [LZCNT] = OPCODE2(0x0f, 0xbd), [LZCNTW] = OPCODE2(0x0f, 0xbd), [LZCNTL] = OPCODE2(0x0f, 0xbd), [LZCNTQ] = OPCODE2(0x0f, 0xbd),
    [TZCNT] = OPCODE3(0x0f, 0x38, 0xf7), [TZCNTW] = OPCODE3(0x0f, 0x38, 0xf7), [TZCNTL] = OPCODE3(0x0f, 0x38, 0xf7), [TZCNTQ] = OPCODE3(0x0f, 0x38, 0xf7),
    [POPCNT] = OPCODE2(0x0f, 0xb8), [POPCNTW] = OPCODE2(0x0f, 0xb8), [POPCNTL] = OPCODE2(0x0f, 0xb8), [POPCNTQ] = OPCODE2(0x0f, 0xb8),

    [JMP] = OPCODE1(0xe9), [JMPW] = OPCODE1(0xe9), [JMPL] = OPCODE1(0xe9), [JMPQ] = OPCODE1(0xe9),
    [CALL] = OPCODE1(0xe8), [CALLW] = OPCODE1(0xe8), [CALLL] = OPCODE1(0xe8), [CALLQ] = OPCODE1(0xe8),
    [RET] = OPCODE1(0xc3), [RETW] = OPCODE1(0xc3), [RETL] = OPCODE1(0xc3), [RETQ] = OPCODE1(0xc3),

    [JO] = OPCODE1(0x70), [JNO] = OPCODE1(0x71), [JB] = OPCODE1(0x72), [JC] = OPCODE1(0x72), [JNAE] = OPCODE1(0x72),
    [JAE] = OPCODE1(0x73), [JNB] = OPCODE1(0x73), [JNC] = OPCODE1(0x73), [JE] = OPCODE1(0x74), [JZ] = OPCODE1(0x74),
    [JNE] = OPCODE1(0x75), [JNZ] = OPCODE1(0x75), [JBE] = OPCODE1(0x76), [JNA] = OPCODE1(0x76),
    [JA] = OPCODE1(0x77), [JNBE] = OPCODE1(0x77), [JS] = OPCODE1(0x78), [JNS] = OPCODE1(0x79),
    [JP] = OPCODE1(0x7a), [JPE] = OPCODE1(0x7a), [JNP] = OPCODE1(0x7b), [JPO] = OPCODE1(0x7b),
    [JL] = OPCODE1(0x7c), [JNGE] = OPCODE1(0x7c), [JGE] = OPCODE1(0x7d), [JNL] = OPCODE1(0x7d),
    [JLE] = OPCODE1(0x7e), [JNG] = OPCODE1(0x7e), [JG] = OPCODE1(0x7f), [JNLE] = OPCODE1(0x7f),
    [JCXZ] = OPCODE1(0xe3), [JECXZ] = OPCODE1(0xe3), [JRCXZ] = OPCODE1(0xe3),
    [LOOP] = OPCODE1(0xe2), [LOOPE] = OPCODE1(0xe1), [LOOPZ] = OPCODE1(0xe1), [LOOPNE] = OPCODE1(0xe0), [LOOPNZ] = OPCODE1(0xe0),

    [SETO] = OPCODE2(0x0f, 0x90), [SETNO] = OPCODE2(0x0f, 0x91), [SETB] = OPCODE2(0x0f, 0x92), [SETC] = OPCODE2(0x0f, 0x92), [SETNAE] = OPCODE2(0x0f, 0x92),
    [SETAE] = OPCODE2(0x0f, 0x93), [SETNB] = OPCODE2(0x0f, 0x93), [SETNC] = OPCODE2(0x0f, 0x93), [SETE] = OPCODE2(0x0f, 0x94), [SETZ] = OPCODE2(0x0f, 0x94),
    [SETNE] = OPCODE2(0x0f, 0x95), [SETNZ] = OPCODE2(0x0f, 0x95), [SETBE] = OPCODE2(0x0f, 0x96), [SETNA] = OPCODE2(0x0f, 0x96),
    [SETA] = OPCODE2(0x0f, 0x97), [SETNBE] = OPCODE2(0x0f, 0x97), [SETS] = OPCODE2(0x0f, 0x98), [SETNS] = OPCODE2(0x0f, 0x99),
    [SETP] = OPCODE2(0x0f, 0x9a), [SETPE] = OPCODE2(0x0f, 0x9a), [SETNP] = OPCODE2(0x0f, 0x9b), [SETPO] = OPCODE2(0x0f, 0x9b),
    [SETL] = OPCODE2(0x0f, 0x9c), [SETNGE] = OPCODE2(0x0f, 0x9c), [SETGE] = OPCODE2(0x0f, 0x9d), [SETNL] = OPCODE2(0x0f, 0x9d),
    [SETLE] = OPCODE2(0x0f, 0x9e), [SETNG] = OPCODE2(0x0f, 0x9e), [SETG] = OPCODE2(0x0f, 0x9f), [SETNLE] = OPCODE2(0x0f, 0x9f),

    [CMOVO] = OPCODE2(0x0f, 0x40), [CMOVNO] = OPCODE2(0x0f, 0x41), [CMOVB] = OPCODE2(0x0f, 0x42), [CMOVC] = OPCODE2(0x0f, 0x42), [CMOVNAE] = OPCODE2(0x0f, 0x42),
    [CMOVAE] = OPCODE2(0x0f, 0x43), [CMOVNB] = OPCODE2(0x0f, 0x43), [CMOVNC] = OPCODE2(0x0f, 0x43), [CMOVE] = OPCODE2(0x0f, 0x44), [CMOVZ] = OPCODE2(0x0f, 0x44),
    [CMOVNE] = OPCODE2(0x0f, 0x45), [CMOVNZ] = OPCODE2(0x0f, 0x45), [CMOVBE] = OPCODE2(0x0f, 0x46), [CMOVNA] = OPCODE2(0x0f, 0x46),
    [CMOVA] = OPCODE2(0x0f, 0x47), [CMOVNBE] = OPCODE2(0x0f, 0x47), [CMOVS] = OPCODE2(0x0f, 0x48), [CMOVNS] = OPCODE2(0x0f, 0x49),
    [CMOVP] = OPCODE2(0x0f, 0x4a), [CMOVPE] = OPCODE2(0x0f, 0x4a), [CMOVNP] = OPCODE2(0x0f, 0x4b), [CMOVPO] = OPCODE2(0x0f, 0x4b),
    [CMOVL] = OPCODE2(0x0f, 0x4c), [CMOVNGE] = OPCODE2(0x0f, 0x4c), [CMOVGE] = OPCODE2(0x0f, 0x4d), [CMOVNL] = OPCODE2(0x0f, 0x4d),
    [CMOVLE] = OPCODE2(0x0f, 0x4e), [CMOVNG] = OPCODE2(0x0f, 0x4e), [CMOVG] = OPCODE2(0x0f, 0x4f), [CMOVNLE] = OPCODE2(0x0f, 0x4f),

    [PUSH] = OPCODE1(0xff), [PUSHW] = OPCODE1(0xff), [PUSHL] = OPCODE1(0xff), [PUSHQ] = OPCODE1(0xff),
    [POP] = OPCODE1(0x8f), [POPW] = OPCODE1(0x8f), [POPL] = OPCODE1(0x8f), [POPQ] = OPCODE1(0x8f),
    [PUSHF] = OPCODE1(0x9c), [PUSHFW] = OPCODE1(0x9c), [PUSHFL] = OPCODE1(0x9c), [PUSHFQ] = OPCODE1(0x9c),
    [POPF] = OPCODE1(0x9d), [POPFW] = OPCODE1(0x9d), [POPFL] = OPCODE1(0x9d), [POPFQ] = OPCODE1(0x9d),
    [LEA] = OPCODE1(0x8d), [LEAW] = OPCODE1(0x8d), [LEAL] = OPCODE1(0x8d), [LEAQ] = OPCODE1(0x8d),
    [XCHG] = OPCODE1(0x87), [XCHGB] = OPCODE1(0x86), [XCHGW] = OPCODE1(0x87), [XCHGL] = OPCODE1(0x87), [XCHGQ] = OPCODE1(0x87),
    [XADD] = OPCODE2(0x0f, 0xc1), [XADDB] = OPCODE2(0x0f, 0xc0), [XADDW] = OPCODE2(0x0f, 0xc1), [XADDL] = OPCODE2(0x0f, 0xc1), [XADDQ] = OPCODE2(0x0f, 0xc1),
    [CMPXCHG] = OPCODE2(0x0f, 0xb1), [CMPXCHGB] = OPCODE2(0x0f, 0xb0), [CMPXCHGW] = OPCODE2(0x0f, 0xb1), [CMPXCHGL] = OPCODE2(0x0f, 0xb1), [CMPXCHGQ] = OPCODE2(0x0f, 0xb1),
    [CMPXCHG8B] = OPCODE2(0x0f, 0xc7), [CMPXCHG16B] = OPCODE2(0x0f, 0xc7),

    [NOP] = OPCODE1(0x90), [PAUSE] = OPCODE2(0xf3, 0x90), [HLT] = OPCODE1(0xf4),
    [CLC] = OPCODE1(0xf8), [STC] = OPCODE1(0xf9), [CMC] = OPCODE1(0xf5),
    [CLD] = OPCODE1(0xfc), [STD] = OPCODE1(0xfd), [CLI] = OPCODE1(0xfa), [STI] = OPCODE1(0xfb),
    [LAHF] = OPCODE1(0x9f), [SAHF] = OPCODE1(0x9e),
    [CWD] = OPCODE1(0x99), [CDQ] = OPCODE1(0x99), [CQO] = OPCODE1(0x99),
    [CBW] = OPCODE1(0x98), [CWDE] = OPCODE1(0x98), [CDQE] = OPCODE1(0x98),
    [AAA] = OPCODE1(0x37), [AAD] = OPCODE1(0xd5), [AAM] = OPCODE1(0xd4), [AAS] = OPCODE1(0x3f), [DAA] = OPCODE1(0x27), [DAS] = OPCODE1(0x2f),
    [ENTER] = OPCODE1(0xc8), [LEAVE] = OPCODE1(0xc9), [LEAVEW] = OPCODE1(0xc9), [LEAVEL] = OPCODE1(0xc9), [LEAVEQ] = OPCODE1(0xc9),
    [INT] = OPCODE1(0xcd), [INTO] = OPCODE1(0xce), [IRET] = OPCODE1(0xcf), [IRETW] = OPCODE1(0xcf), [IRETL] = OPCODE1(0xcf), [IRETQ] = OPCODE1(0xcf),
    [SYSCALL] = OPCODE2(0x0f, 0x05), [SYSENTER] = OPCODE2(0x0f, 0x34), [SYSEXIT] = OPCODE2(0x0f, 0x35), [SYSRET] = OPCODE2(0x0f, 0x07),
    [CPUID] = OPCODE2(0x0f, 0xa2), [RDTSC] = OPCODE2(0x0f, 0x31), [RDTSCP] = OPCODE2(0x0f, 0x01), [RDPMC] = OPCODE2(0x0f, 0x33),
    [LFENCE] = OPCODE3(0x0f, 0xae, 0xe8), [MFENCE] = OPCODE3(0x0f, 0xae, 0xf0), [SFENCE] = OPCODE3(0x0f, 0xae, 0xf8),
    [LOCK] = OPCODE1(0xf0), [WAIT] = OPCODE1(0x9b), [UD2] = OPCODE2(0x0f, 0x0b),
    [IN] = OPCODE1(0xe5), [INB] = OPCODE1(0xe4), [INW] = OPCODE1(0xe5), [INL] = OPCODE1(0xe5),
    [OUT] = OPCODE1(0xe7), [OUTB] = OPCODE1(0xe6), [OUTW] = OPCODE1(0xe7), [OUTL] = OPCODE1(0xe7),
    [INS] = OPCODE1(0x6d), [INSB] = OPCODE1(0x6c), [INSW] = OPCODE1(0x6d), [INSL] = OPCODE1(0x6d),
    [OUTS] = OPCODE1(0x6f), [OUTSB] = OPCODE1(0x6e), [OUTSW] = OPCODE1(0x6f), [OUTSL] = OPCODE1(0x6f),
    [LODS] = OPCODE1(0xad), [LODSB] = OPCODE1(0xac), [LODSW] = OPCODE1(0xad), [LODSL] = OPCODE1(0xad), [LODSQ] = OPCODE1(0xad),
    [STOS] = OPCODE1(0xab), [STOSB] = OPCODE1(0xaa), [STOSW] = OPCODE1(0xab), [STOSL] = OPCODE1(0xab), [STOSQ] = OPCODE1(0xab),
    [SCAS] = OPCODE1(0xaf), [SCASB] = OPCODE1(0xae), [SCASW] = OPCODE1(0xaf), [SCASL] = OPCODE1(0xaf), [SCASQ] = OPCODE1(0xaf),
    [CMPS] = OPCODE1(0xa7), [CMPSB] = OPCODE1(0xa6), [CMPSW] = OPCODE1(0xa7), [CMPSL] = OPCODE1(0xa7), [CMPSQ] = OPCODE1(0xa7),
    [XLAT] = OPCODE1(0xd7), [XLATB] = OPCODE1(0xd7),
    [BOUND] = OPCODE1(0x62), [ARPL] = OPCODE1(0x63),
    [LAR] = OPCODE2(0x0f, 0x02), [LSL] = OPCODE2(0x0f, 0x03),
    [LGDT] = OPCODE2(0x0f, 0x01), [LIDT] = OPCODE2(0x0f, 0x01), [SGDT] = OPCODE2(0x0f, 0x01), [SIDT] = OPCODE2(0x0f, 0x01),
    [LLDT] = OPCODE2(0x0f, 0x00), [SLDT] = OPCODE2(0x0f, 0x00), [LTR] = OPCODE2(0x0f, 0x00), [STR] = OPCODE2(0x0f, 0x00),
    [LMSW] = OPCODE2(0x0f, 0x01), [SMSW] = OPCODE2(0x0f, 0x01), [CLTS] = OPCODE2(0x0f, 0x06),
    [VERR] = OPCODE2(0x0f, 0x00), [VERW] = OPCODE2(0x0f, 0x00), [WBINVD] = OPCODE2(0x0f, 0x09),
};

static size_t encode_to_bytes(const Opcode *opcode, uint8_t out[MAX_OPCODE_LEN]) {
    size_t n = 0;
    for (uint8_t i = 0; i < opcode->prefix_len; i++)
        out[n++] = opcode->prefixes[i];
    if (opcode->has_rex)
        out[n++] = opcode->rex;
    for (uint8_t i = 0; i < opcode->opcode_len; i++)
        out[n++] = opcode->opcode[i];
    if (opcode->has_modrm)
        out[n++] = opcode->modrm;
    if (opcode->has_sib)
        out[n++] = opcode->sib;
    for (uint8_t i = 0; i < opcode->disp_len; i++)
        out[n++] = opcode->disp[i];
    for (uint8_t i = 0; i < opcode->imm_len; i++)
        out[n++] = opcode->imm[i];
    return n;
}
