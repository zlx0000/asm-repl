#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>

typedef struct {
    __uint64_t flags;
    __uint64_t rax;
    __uint64_t rbx;
    __uint64_t rcx;
    __uint64_t rdx;
    __uint64_t rsi;
    __uint64_t rdi;
    __uint64_t rsp;
    __uint64_t rbp;
    __uint64_t r8;
    __uint64_t r9;
    __uint64_t r10;
    __uint64_t r11;
    __uint64_t r12;
    __uint64_t r13;
    __uint64_t r14;
    __uint64_t r15;
    __uint64_t rip;
} State;

static struct sigaction sa;
struct sigaction old_sa[NSIG];
static State cstate;
static State rstate;

static char r[49152];
static uint8_t shellcode[16304];

static volatile sig_atomic_t lastSig = 0;

typedef struct {
    uint8_t *addr;
    uint8_t *epilogue;
    size_t codelen;
    size_t pagelen;
} code_page;

static code_page code;
static code_page lastCode;

__attribute__ ((__noreturn__)) void sig_handler(int sig)
{
    lastSig = sig;
     __asm__ __volatile__ (
        "movq 16(%0), %%rbx\n\t"
        "movq 24(%0), %%rcx\n\t"
        "movq 32(%0), %%rdx\n\t"
        "movq 40(%0), %%rsi\n\t"
        "movq 64(%0), %%rbp\n\t"
        "movq 72(%0), %%r8\n\t"
        "movq 80(%0), %%r9\n\t"
        "movq 88(%0), %%r10\n\t"
        "movq 96(%0), %%r11\n\t"
        "movq 104(%0), %%r12\n\t"
        "movq 112(%0), %%r13\n\t"
        "movq 120(%0), %%r14\n\t"
        "movq 128(%0), %%r15\n\t"
        "lea -8(%%rsp), %%rsp\n\t"
        "movq 0(%%rdi), %%rax\n\t"
        "movq %%rax, (%%rsp)\n\t"
        "popfq\n\t"
        "movq 56(%%rdi), %%rsp\n\t"
        "movq 8(%%rdi), %%rax\n\t"
        "push 136(%%rdi)\n\t"
        "movq 48(%%rdi), %%rdi\n\t"
        "ret\n\t"
        :
        : "D"(&rstate)
        :
    );
    __builtin_unreachable();
}

#define STR(x) #x
#define XSTR(x) STR(x)

#define OFF_FLAGS 0
#define OFF_RAX 8
#define OFF_RBX 16
#define OFF_RCX 24
#define OFF_RDX 32
#define OFF_RSI 40
#define OFF_RDI 48
#define OFF_RSP 56
#define OFF_RBP 64
#define OFF_R8  72
#define OFF_R9  80
#define OFF_R10 88
#define OFF_R11 96
#define OFF_R12 104
#define OFF_R13 112
#define OFF_R14 120
#define OFF_R15 128
#define OFF_RIP 136

#define SAVE_REG(reg, off, state) "movq %%" #reg ", " XSTR(off) "(%"#state")\n\t"

#define RESTORE_REG(reg, off, state) "movq" XSTR(off) "(%" #state ")" ", " "%%" #reg "\n\t"

int gen_code(code_page *code, uint8_t * shellcode, size_t codelen)
{
    assert(code->codelen <= code->pagelen);

    /*
    start:
    push %rdi
    movabs $cstate, %rdi
    movq %rax, 8(%rdi)
    movq %rbx, 16(%rdi)
    movq %rcx, 24(%rdi)
    movq %rdx, 32(%rdi)
    movq %rsi, 40(%rdi)
    movq %rsp, 56(%rdi)
    movq %rbp, 64(%rdi)
    movq %r8, 72(%rdi)
    movq %r9, 80(%rdi)
    movq %r10, 88(%rdi)
    movq %r11, 96(%rdi)
    movq %r12, 104(%rdi)
    movq %r13, 112(%rdi)
    movq %r14, 120(%rdi)
    movq %r15, 128(%rdi)
    lea start(%rip), %rax
    movq %rax, 136(%rdi)
    pushfq
    movq (%rsp), %rax
    movq %rax, (%rdi)
    addq $8, 56(%rdi)
    popfq
    pop 48(%rdi)

    movabs $rstate, %rdi
    movq 16(%rdi), %rbx
    movq 24(%rdi), %rcx
    movq 32(%rdi), %rdx
    movq 40(%rdi), %rsi
    movq 64(%rdi), %rbp
    movq 72(%rdi), %r8
    movq 80(%rdi), %r9
    movq 88(%rdi), %r10
    movq 96(%rdi), %r11
    movq 104(%rdi), %r12
    movq 112(%rdi), %r13
    movq 120(%rdi), %r14
    movq 128(%rdi), %r15
    lea -8(%rsp), %rsp
    movq 0(%rdi), %rax
    movq %rax, (%rsp)
    popfq
    movq 56(%rdi), %rsp
    movq 8(%rdi), %rax
    push 136(%rdi)
    movq 48(%rdi), %rdi
    ret
    */

    static uint8_t epilogue[] = {
        0x57,

        0x48,0xbf, 0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,

        0x48,0x89,0x47,0x08,
        0x48,0x89,0x5f,0x10,
        0x48,0x89,0x4f,0x18,
        0x48,0x89,0x57,0x20,
        0x48,0x89,0x77,0x28,
        0x48,0x89,0x67,0x38,
        0x48,0x89,0x6f,0x40,
        0x4c,0x89,0x47,0x48,
        0x4c,0x89,0x4f,0x50,
        0x4c,0x89,0x57,0x58,
        0x4c,0x89,0x5f,0x60,
        0x4c,0x89,0x67,0x68,
        0x4c,0x89,0x6f,0x70,
        0x4c,0x89,0x77,0x78,
        0x4c,0x89,0xbf,0x80,0x00,0x00,0x00,
        0x48,0x8d,0x05,0xaf,0xff,0xff,0xff,
        0x48,0x89,0x87,0x88,0x00,0x00,0x00,
        0x9c,
        0x48,0x8b,0x04,0x24,
        0x48,0x89,0x07,
        0x48,0x83,0x47,0x38,0x08,
        0x9d,
        0x8f,0x47,0x30,

        0x48,0xbf, 0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,

        0x48,0x8b,0x5f,0x10,
        0x48,0x8b,0x4f,0x18,
        0x48,0x8b,0x57,0x20,
        0x48,0x8b,0x77,0x28,
        0x48,0x8b,0x6f,0x40,
        0x4c,0x8b,0x47,0x48,
        0x4c,0x8b,0x4f,0x50,
        0x4c,0x8b,0x57,0x58,
        0x4c,0x8b,0x5f,0x60,
        0x4c,0x8b,0x67,0x68,
        0x4c,0x8b,0x6f,0x70,
        0x4c,0x8b,0x77,0x78,
        0x4c,0x8b,0xbf,0x80,0x00,0x00,0x00,
        0x48,0x8d,0x64,0x24,0xf8,
        0x48,0x8b,0x07,
        0x48,0x89,0x04,0x24,
        0x9d,
        0x48,0x8b,0x67,0x38,
        0x48,0x8b,0x47,0x08,
        0xff,0xb7,0x88,0x00,0x00,0x00,
        0x48,0x8b,0x7f,0x30,
        0xc3
    };

    uint64_t p = (uint64_t)&cstate;  //patch the address.
    memcpy(epilogue + 3, &p, sizeof(p));
    p = (uint64_t)&rstate;
    memcpy(epilogue + 107, &p, sizeof(p));
    size_t pagesize = sysconf(_SC_PAGESIZE);
    if (code->addr == NULL) {
        code->codelen = sizeof(epilogue);
        size_t pagelen = (code->codelen + pagesize - 1) & ~(pagesize - 1);
        code->pagelen = pagelen;
        code->addr = mmap(NULL, 4ULL * 1024 * 1024 * 1024, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (code->addr == MAP_FAILED) return -1;
        if (mprotect(code->addr, code->pagelen, PROT_READ | PROT_WRITE) != 0) return -1;
        code->epilogue = code->addr;
        memcpy(code->epilogue, epilogue, sizeof(epilogue));
        return mprotect(code->addr, code->pagelen, PROT_READ | PROT_EXEC);
    } else {
        if (mprotect(code->addr, code->pagelen, PROT_READ | PROT_WRITE) != 0) return -1;
        size_t newSize = code->codelen + codelen;
        code->codelen = newSize;
        if (newSize >= code->pagelen) {
            newSize = (newSize + pagesize - 1) & ~(pagesize - 1);
            if (mprotect(code->addr, newSize, PROT_READ | PROT_WRITE) != 0) return -1;
            code->pagelen = newSize;
        }
        memcpy(code->epilogue, shellcode, codelen);
        code->epilogue += codelen;
        memcpy(code->epilogue, epilogue, sizeof(epilogue));
        return mprotect(code->addr, code->pagelen, PROT_READ | PROT_EXEC);
    }
}

#define SAVE_STATE(state)       \
    __asm__ __volatile__(       \
    "1:\n\t"                    \
    SAVE_REG(rax, OFF_RAX, 0)   \
    "lea 2f(%%rip), %%rax\n\t"  \
    "movq %%rax, 136(%0)\n\t"   \
    "movq 0(%0), %%rax\n\t"     \
    SAVE_REG(rbx, OFF_RBX, 0)   \
    SAVE_REG(rcx, OFF_RCX, 0)   \
    SAVE_REG(rdx, OFF_RDX, 0)   \
    SAVE_REG(rsi, OFF_RSI, 0)   \
    SAVE_REG(rdi, OFF_RDI, 0)   \
    SAVE_REG(rsp, OFF_RSP, 0)   \
    SAVE_REG(rbp, OFF_RBP, 0)   \
    SAVE_REG(r8,  OFF_R8, 0)    \
    SAVE_REG(r9,  OFF_R9, 0)    \
    SAVE_REG(r10, OFF_R10, 0)   \
    SAVE_REG(r11, OFF_R11, 0)   \
    SAVE_REG(r12, OFF_R12, 0)   \
    SAVE_REG(r13, OFF_R13, 0)   \
    SAVE_REG(r14, OFF_R14, 0)   \
    SAVE_REG(r15, OFF_R15, 0)   \
    "pushfq\n\t"                \
    "movq (%%rsp), %%rax\n\t"   \
    "movq %%rax, 0(%0)\n\t"     \
    "popfq\n\t"                 \
    "2:\n\t"                    \
    :                           \
    : "D"(state)                \
    : "%rax", "memory"          \
    );                          \

int enter_code(char* row, uint8_t *code)
{
    char *pl = row;
    uint8_t *pc = code;
    int len = 0;

    while (*pl) {
        while (*pl == ' ') pl++;
        if (*pl == '\0') break;

        char *end;
        long val = strtol(pl, &end, 16);

        if (pl == end) break;
        if (val < 0 || val > UINT8_MAX) return -1;

        *pc++ = (uint8_t)val;
        len++;

        pl = end;
    }

    return len;
}

int main(int argc, char **argv)
{
    int codelen;
    bool def = false;
    
    memset(&sa, 0, sizeof(sa));
    memset(&old_sa, 0, sizeof(old_sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags = SA_NODEFER;
    sigemptyset(&sa.sa_mask);

    for (int i = 1; i < NSIG; i++) {
        if (i == SIGKILL || i == SIGSTOP) continue;
        sigaction(i, &sa, &old_sa[i]);
    }

    for (int i = 1; i < NSIG; i++) {
        if (i == SIGKILL || i == SIGSTOP) continue;
        sigaction(i, &old_sa[i], NULL);
    }
    
    void *cstack = malloc(8448 * sizeof(char));
    uintptr_t top = (uintptr_t)cstack + 8192;
    top &= ~0xFULL;

    code.addr = NULL;
    code.epilogue = NULL;
    code.codelen = 0;
    code.pagelen = 0;

    cstate.rsp = (uint64_t) top;
    cstate.rax = 0;
    cstate.rbx = 0;
    cstate.rcx = 0;
    cstate.rdx = 0;
    cstate.rsi = 0;
    cstate.rdi = 0;
    cstate.rbp = 0;
    cstate.r8 = 0;
    cstate.r9 = 0;
    cstate.r10 = 0;
    cstate.r11 = 0;
    cstate.r12 = 0;
    cstate.r13 = 0;
    cstate.r14 = 0;
    cstate.r15 = 0;
    if (gen_code(&code, NULL, 0)) {
        perror("gencode failed");
        goto end;
    }
    cstate.rip = (uint64_t)code.addr;
    //lastState = cstate;
    //lastCode = code;
    SAVE_STATE(&rstate);

repl:
    for (int i = 1; i < NSIG; i++) {
        if (i == SIGKILL || i == SIGSTOP) continue;
        sigaction(i, &old_sa[i], NULL);
    }
    if (lastSig != 0) {
        code.codelen = lastCode.codelen;
        code.epilogue = lastCode.epilogue;
        printf("(%s)0x%lx>", strsignal(lastSig), cstate.rip);
        lastSig = 0;
    }
    else
        printf("0x%lx>", cstate.rip);
    if (!fgets(r, sizeof(r), stdin)) {
        printf("\n");
        goto end;
    }
    if (strcmp(r, "quit\n") == 0 || strcmp(r, "exit\n") == 0) goto end;
    if (strcmp(r, "def\n") == 0) {
        if (def)
            printf("ignored.\n");
        def = true;
        goto repl;
    }
    if (strcmp(r, "enddef\n") == 0) {
        if (!def)
            printf("ignored.\n");
        def = false;
        goto repl;
    }
    codelen = enter_code(r, shellcode);
    if (codelen <= 0) goto repl;
    lastCode = code;
    if (gen_code(&code, shellcode, codelen)) goto end;
    if (!def) {
        for (int i = 1; i < NSIG; i++) {
            if (i == SIGKILL || i == SIGSTOP) continue;
            sigaction(i, &sa, NULL);
        }
        goto run;
    } else {
        cstate.rip = (uint64_t)code.epilogue;
        goto repl;
    }

run:
    __asm__ __volatile__ (
        "movq 16(%0), %%rbx\n\t"
        "movq 24(%0), %%rcx\n\t"
        "movq 32(%0), %%rdx\n\t"
        "movq 40(%0), %%rsi\n\t"
        "movq 64(%0), %%rbp\n\t"
        "movq 72(%0), %%r8\n\t"
        "movq 80(%0), %%r9\n\t"
        "movq 88(%0), %%r10\n\t"
        "movq 96(%0), %%r11\n\t"
        "movq 104(%0), %%r12\n\t"
        "movq 112(%0), %%r13\n\t"
        "movq 120(%0), %%r14\n\t"
        "movq 128(%0), %%r15\n\t"
        "lea -8(%%rsp), %%rsp\n\t"
        "movq 0(%%rdi), %%rax\n\t"
        "movq %%rax, (%%rsp)\n\t"
        "popfq\n\t"
        "movq 56(%%rdi), %%rsp\n\t"
        "movq 8(%%rdi), %%rax\n\t"
        "push 136(%%rdi)\n\t"
        "movq 48(%%rdi), %%rdi\n\t"
        "ret\n\t"
        :
        : "D"(&cstate)
        :
    );
    __builtin_unreachable();

end:
    munmap(code.addr, 4ULL * 1024 * 1024 * 1024);
    free(cstack);
    return 0;
}