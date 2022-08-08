/* S390 Disassembler and debugger kernel module */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uintptr_t register_t;
typedef struct _cpu_context_t {
    union {
        register_t gp_regs[16];
        struct {
            register_t r0;
            register_t r1;
            register_t r2;
            register_t r3;
            register_t r4;
            register_t r5;
            register_t r6;
            register_t r7;
            register_t r8;
            register_t r9;
            register_t r10;
            register_t r11;
            register_t r12;
            register_t r13;
            register_t r14;
            register_t r15;
        };
    };
}cpu_context_t;

enum debug_inst_format {
    INST_FMT_NONE,
    INST_FMT_S,
    INST_FMT_S_NOP, /* S - operands ignored */
    INST_FMT_E,
    INST_FMT_RRE,
    INST_FMT_RRE_NOP, /* RRE - operands ignored */
    INST_FMT_RRE_R1ONLY, /* RRE - all operands but R1 are ignored */
    INST_FMT_RRF,
    INST_FMT_RRF_A,
    INST_FMT_RS,
    INST_FMT_RSY,
    INST_FMT_RX,
    INST_FMT_RXY,
    INST_FMT_SSE,
    INST_FMT_SSF,
    INST_FMT_SS,
    INST_FMT_SI,
    INST_FMT_RR,
    INST_FMT_RI,
    INST_FMT_RIL,
    INST_FMT_SIY,
    INST_FMT_DIAG,
};

struct debug_inst_desc {
    const char *name;
    enum debug_inst_format format;
    unsigned int op1;
};

void debug_disasm_print(const void *addr, size_t len);
void debug_frame_print(const cpu_context_t *frame);

/* Symbols of the kernel (last updated 17:10 on January 16, 2022)*/
struct debug_sym_data {
    void *addr;
    const char *name;
};

const debug_sym_data *debug_get_symbol(const void *addr);
int debug_unwind_stack_gcc(const void *addr);
int debug_unwind_stack_mvs(const void *addr, void *stack_bottom);

/**
 * @brief Instruction list for the System/Z architecture
 * See the z/Architecture Principles of Operation PDF
 * https://www.ibm.com/docs/en/SSQ2R2_15.0.0/com.ibm.tpf.toolkit.hlasm.doc/dz9zr006.pdf
 * 
 */
const struct debug_inst_desc g_inst_descs[] = {
    /* Control instructions */
    { "bsa", INST_FMT_RRE, 0xb25a },
    { "bakr", INST_FMT_RRE, 0xb240 },
    { "bsg", INST_FMT_RRE, 0xb258 },
    { "csp", INST_FMT_RRE, 0xb250 },
    { "cspg", INST_FMT_RRE, 0xb98a },
    { "diag", INST_FMT_DIAG, 0x83 },
    { "esea", INST_FMT_RRE_R1ONLY, 0xb99d },
    { "epar", INST_FMT_RRE_R1ONLY, 0xb226 },
    { "epair", INST_FMT_RRE_R1ONLY, 0xb99a },
    { "esar", INST_FMT_RRE_R1ONLY, 0xb227 },
    { "esair", INST_FMT_RRE_R1ONLY, 0xb99b },
    { "ereg", INST_FMT_RRE, 0xb249 },
    { "eregg", INST_FMT_RRE, 0xb90e },
    { "esta", INST_FMT_RRE, 0xb24a },
    { "iac", INST_FMT_RRE, 0xb224 },
    { "ipk", INST_FMT_S_NOP, 0xb20b },
    { "iske", INST_FMT_RRE, 0xb229 },
    { "ivsk", INST_FMT_RRE, 0xb223 },
    { "idte", INST_FMT_RRF, 0xb98e },
    { "ipte", INST_FMT_RRF_A, 0xb221 },
    { "lasp", INST_FMT_SSE, 0xe500 },
    { "lctl", INST_FMT_RS, 0xb7 },
    { "lctlg", INST_FMT_RSY, 0xeb2f },
    { "lptea", INST_FMT_RRF, 0xb9aa },
    { "lpsw", INST_FMT_S, 0x8200 }, /* lower 8-bits are xx-any */
    { "lpswe", INST_FMT_S, 0xb2b2 },
    { "lra", INST_FMT_RX, 0xb1 },
    { "lray", INST_FMT_RXY, 0xe313 },
    { "lrag", INST_FMT_RXY, 0xe303 },
    { "lura", INST_FMT_RRE, 0xb24b },
    { "lurag", INST_FMT_RRE, 0xb905 },
    { "msta", INST_FMT_RRE_R1ONLY, 0xb247 },
    { "mvpg", INST_FMT_RRE, 0xb254 },
    { "mvcp", INST_FMT_SS, 0xda },
    { "mvcs", INST_FMT_SS, 0xdb },
    { "mvcdk", INST_FMT_SSE, 0xe50f },
    { "mvck", INST_FMT_SS, 0xd9 },
    { "mvcos", INST_FMT_SSF, 0xc8 },
    { "mvcsk", INST_FMT_SSE, 0xe50e },
    { "pgin", INST_FMT_RRE, 0xb22e },
    { "pgout", INST_FMT_RRE, 0xb22f },
    { "pfmf", INST_FMT_RRE, 0xb9af },
    { "ptff", INST_FMT_E, 0x0104 },
    { "ptf", INST_FMT_RRE_R1ONLY, 0xb9a2 },
    { "pc", INST_FMT_S, 0xb218 },
    { "pr", INST_FMT_E, 0x0101 },
    { "pt", INST_FMT_RRE, 0xb228 },
    { "pti", INST_FMT_RRE, 0xb99e },
    { "palb", INST_FMT_RRE_NOP, 0xb248 },
    { "ptlb", INST_FMT_RRE_NOP, 0xb20d },
    { "rrbe", INST_FMT_RRE, 0xb22a },
    { "rp", INST_FMT_S, 0xb277 },
    { "sac", INST_FMT_S, 0xb219 },
    { "sacf", INST_FMT_S, 0xb279 },
    { "sck", INST_FMT_S, 0xb204 },
    { "sckc", INST_FMT_S, 0xb206 },
    { "sckpf", INST_FMT_E, 0x0107 },
    { "spt", INST_FMT_S, 0xb208 },
    { "spx", INST_FMT_S, 0xb210 },
    { "spka", INST_FMT_S, 0xb20a },
    { "ssar", INST_FMT_RRE_R1ONLY, 0xb225 },
    { "ssair", INST_FMT_RRE_R1ONLY, 0xb99f },
    { "sske", INST_FMT_RRF, 0xb22b },
    { "ssm", INST_FMT_S, 0x8000 }, /* lower 8-bits are xx-any */
    { "sigp", INST_FMT_RS, 0xae },
    { "stckc", INST_FMT_S, 0xb207 },
    { "stctl", INST_FMT_RS, 0xb6 },
    { "stctlg", INST_FMT_RSY, 0xe825 },
    { "stap", INST_FMT_S, 0xb212 },
    { "stidp", INST_FMT_S, 0xb202 },
    { "stpt", INST_FMT_S, 0xb209 },
    { "stfl", INST_FMT_S, 0xb2b1 },
    { "stpx", INST_FMT_S, 0xb211 },
    { "strag", INST_FMT_SSE, 0xe502 },
    { "stsi", INST_FMT_S, 0xb27d },
    { "stnsm", INST_FMT_SI, 0xac },
    { "stosm", INST_FMT_SI, 0xad },
    { "stura", INST_FMT_RRE, 0xb248 },
    { "sturg", INST_FMT_RRE, 0xb925 },
    { "tar", INST_FMT_RRE, 0xb24c },
    { "tb", INST_FMT_RRE, 0xb22c },
    { "tprot", INST_FMT_SSE, 0xe501 },
    { "trace", INST_FMT_RS, 0x99 },
    { "tracg", INST_FMT_RSY, 0xeb0f },
    { "trap2", INST_FMT_E, 0x01ff },
    { "trap4", INST_FMT_S, 0xb2ff },
    /* General purpouse instructions */
    { "a", INST_FMT_RX, 0x5a },
    { "ar", INST_FMT_RR, 0x1a },
    { "ay", INST_FMT_RXY, 0xe35a },
    { "ag", INST_FMT_RXY, 0xe308 },
    { "agr", INST_FMT_RRE, 0xb908 },
    { "agf", INST_FMT_RXY, 0xe318 },
    { "agfr", INST_FMT_RRE, 0xb918 },
    { "ah", INST_FMT_RX, 0x4a },
    { "ahy", INST_FMT_RXY, 0xe37a },
    { "ahi", INST_FMT_RI, 0xa7a },
    { "aghi", INST_FMT_RI, 0xa7b },
    { "afi", INST_FMT_RIL, 0xc29 },
    { "asi", INST_FMT_SIY, 0xeb6a },
    { "agfi", INST_FMT_RIL, 0xc28 },
    { "agsi", INST_FMT_SIY, 0xeb7a },
    { "al", INST_FMT_RX, 0x5e },
    { "alr", INST_FMT_RR, 0x1e },
    { "aly", INST_FMT_RXY, 0xe35e },
    { "alg", INST_FMT_RXY, 0xe30a },
    { "algr", INST_FMT_RRE, 0xb90a },
    /* End of the list */
    { nullptr, INST_FMT_NONE, 0 }
};

void debug_disasm_print(const void *addr, size_t len)
{
    const void *end_addr = (const void *)((const uint8_t *)addr + len);
    while((uintptr_t)addr < (uintptr_t)end_addr) {
        size_t i = 0;
        while(g_inst_descs[i].name != nullptr) {
            const struct debug_inst_desc *desc = &g_inst_descs[i];
            uint16_t op = *((const uint16_t *)addr);
            if(desc->op1 == op) {
                uint8_t r1, r2;

                /* Skip the opcode */
                addr = ((const uint8_t *)addr + 2);
                switch(desc->format) {
                case INST_FMT_RRE:
                case INST_FMT_RRE_NOP:
                case INST_FMT_RRE_R1ONLY:
                    /* [Opcode 16] [Blank 8] [R1 4] [R2 4] */
                    r1 = *((const uint8_t *)addr) >> 8;
                    r2 = *((const uint8_t *)addr) & 0xff;
                    addr = ((const uint8_t *)addr + 1); /* Skip r1,r2 */

                    if(desc->format == INST_FMT_RRE) {
                        printf("%s %p,%p\r\n", desc->name, (uintptr_t)r1, (uintptr_t)r2);
                    } else if(desc->format == INST_FMT_RRE_R1ONLY) {
                        printf("%s %p\r\n", desc->name, (uintptr_t)r1);
                    } else if(desc->format == INST_FMT_RRE_NOP) {
                        printf("%s\r\n", desc->name);
                    }
                    break;
                case INST_FMT_S:
                case INST_FMT_S_NOP:
                    addr = ((const uint8_t *)addr + 2); /* Skip the blank */

                    if(desc->format == INST_FMT_S) {
                        printf("%s ?\r\n", desc->name);
                    } else if(desc->format == INST_FMT_S_NOP) {
                        printf("%s\r\n", desc->name);
                    }
                    break;
                default:
                    printf("???\r\n");
                    break;
                }
                break;
            } else if((desc->op1 & 0xff00) == (op & 0xff00)) {
                uint8_t imm8, b, dh, op2;
                uint16_t dl;
                uint32_t d;

                addr = ((const uint8_t *)addr + 1);
                switch(desc->format) {
                case INST_FMT_SIY:
                    /* [Opcode 8] [Imm 8] [B 4] [DL 12] [DH 8] [Op2 8] */
                    imm8 = *((const uint8_t *)addr);

                    b = *((const uint16_t *)addr) >> (16 - 4);
                    dl = (*((const uint16_t *)addr) & 0xf) << 8; /* Take the upper 4 bits */
                    addr = ((const uint8_t *)addr + 1);
                    dh = *((const uint8_t *)addr); /* Then the lower 8 bits */
                    addr = ((const uint8_t *)addr + 1);

                    op2 = *((const uint8_t *)addr); /* Finally take the opcode */
                    addr = ((const uint8_t *)addr + 1);

                    d = (dh << 12) | dl;

                    printf("%s %p(%p),%p\r\n", desc->name, (uintptr_t)d, (uintptr_t)b, (uintptr_t)imm8);
                    break;
                default:
                    printf("???\r\n");
                    break;
                }
                break;
            }
            i++;
        }
    }
}

struct dasm_breakpoint {
    void *addr;
};

void debug_frame_print(const cpu_context_t *frame)
{
    dprintf("GR00: %p GR01: %p GR02: %p GR03: %p", (uintptr_t)frame->r0, (uintptr_t)frame->r1, (uintptr_t)frame->r2, (uintptr_t)frame->r3);
    dprintf("GR04: %p GR05: %p GR06: %p GR07: %p", (uintptr_t)frame->r4, (uintptr_t)frame->r5, (uintptr_t)frame->r6, (uintptr_t)frame->r7);
    dprintf("GR08: %p GR09: %p GR10: %p GR11: %p", (uintptr_t)frame->r8, (uintptr_t)frame->r9, (uintptr_t)frame->r10, (uintptr_t)frame->r11);
    dprintf("GR12: %p GR13: %p GR14: %p GR15: %p", (uintptr_t)frame->r12, (uintptr_t)frame->r13, (uintptr_t)frame->r14, (uintptr_t)frame->r15);
}

/* NOTE: These must be ordered in ascending order!! */
#define DEBUG_SYM_FN(x) \
    { &x, #x },

const struct debug_sym_data sym_tab[] = {
    { (void *)0x0000, "FLCRNPSW" },
    { (void *)0x0008, "FLCROPSW" },
    /** @todo Add kernel symbols */
    { nullptr, nullptr }
};

const struct debug_sym_data *debug_get_symbol(const void *addr)
{
    const struct debug_sym_data *sym, *r_sym;
    size_t i = 0;

    r_sym = &sym_tab[0];
    for(i = 0; i < sizeof(sym_tab) / sizeof(sym_tab[0]); i++) {
        sym = &sym_tab[i];
        if((uintptr_t)sym->addr < (uintptr_t)addr && (uintptr_t)sym->addr > (uintptr_t)r_sym->addr) {
            r_sym = sym;
            continue;
        }
    }
    return r_sym;
}

struct s390_gcc_call_stack {
    uint32_t backchain;
    uint32_t end_of_stack;
    uint32_t glue[2];
    uint32_t scratch[2];
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t r13;
    uint32_t r14;
    uint32_t r15;
    uint32_t f4;
    uint32_t f6;
    /* After this comes the local variables, alloca, etc */
} __attribute__((packed));

struct s390x_gcc_call_stack {
    uint64_t backchain;
    uint64_t end_of_stack;
    uint64_t glue[2];
    uint64_t scratch[2];
    uint64_t r6;
    uint64_t r7;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t f4;
    uint64_t f6;
} __attribute__((packed));

int debug_unwind_stack_gcc(const void *addr)
{
    struct s390x_gcc_call_stack *frame = (struct s390x_gcc_call_stack *)addr;
    while(frame != nullptr) {
        /* Obtain the return address of the backchain */
        uintptr_t retaddr;
        const struct debug_sym_data *retsym;

        retaddr = (uintptr_t)frame->r13;
        retsym = debug_get_symbol((const void *)retaddr);
        if(retsym != nullptr) {
            dprintf("RET=(%p=%s:%p)", (uintptr_t)retaddr, retsym->name, (uintptr_t)((ptrdiff_t)retaddr - (ptrdiff_t)retsym->addr));
        }
        dprintf("Frame=%p,BackChain=%p,EOS=%p", frame, frame->backchain, frame->end_of_stack);

        /* Bounds check */
        /** @todo Bounds check */
        frame = (struct s390x_gcc_call_stack *)((uintptr_t)frame->backchain);
    }
    return 0;
}

/* Information of the stack taken from http://mvs380.sourceforge.net/System380.txt */
struct s370_mvs_call_stack {
    uint32_t unused; /* 0 - unused by C but PL/I or COBOL might use it */
    uint32_t backchain; /* 4 - backchain to previous save area */
    uint32_t forwchain; /* 8 - forward chain to next save area */
    uint32_t r14; /* 12 - R14 */
    uint32_t r15; /* 16 - R15 */
    uint32_t r0; /* 20 - R0 */
    uint32_t r1; /* 24 - R1 */
    uint32_t r2; /* 28 - R2 */
    uint32_t r3; /* 32 - R3 */
    uint32_t r4; /* 36 - R4 */
    uint32_t r5; /* 40 - R5 */
    uint32_t r6; /* 44 - R6 */
    uint32_t r7; /* 48 - R7 */
    uint32_t r8; /* 52 - R8 */
    uint32_t r9; /* 56 - R9 */
    uint32_t r10; /* 60 - R10 */
    uint32_t r11; /* 64 - R11 */
    uint32_t r12; /* 68 - R12 */
    uint32_t crab; /* 72 - unused but could be used to store a CRAB */
    uint32_t top_of_stack; /* 76 - pointer to the top of the stack */
    uint32_t work_area[2]; /* 80 - work area for compiler-generated code (CONVLO)
                              84 - work area for compiler-generated code (CONVHI) */
    /* 88 - local variables begin */
} __attribute__((packed));

int debug_unwind_stack_mvs(const void *addr, void *stack_bottom)
{
    /* MVS call convention */
    uint8_t *frame = (uint8_t *)addr;
    /* Corrupt stack pointer? */
    if((ptrdiff_t)frame > (ptrdiff_t)stack_bottom) {
        dprintf("Potential corrupt stack, (SP=%p)\r\n", (uintptr_t)frame);
        frame = (uint8_t *)stack_bottom;
    }

    /* Pointer to top of the stack */
    while(frame != nullptr) {
        /* R14 is stored on frame+12; and holds the address of the return point of the caller */
        uintptr_t retaddr = (uintptr_t)*((const volatile uint32_t *)&frame[12]);
        const struct debug_sym_data *retsym = debug_get_symbol((const void *)retaddr);
        /* And R15 is also stored on frame+16, and contains the branching/base address of the callee */
        uintptr_t calladdr = (uintptr_t)*((const volatile uint32_t *)&frame[16]);
        const struct debug_sym_data *callsym = debug_get_symbol((const void *)calladdr);

        if(retsym != nullptr) {
            dprintf("RET=(%i=%s:%i)\r\n", (unsigned int)retaddr, retsym->name, (int)((ptrdiff_t)retaddr - (ptrdiff_t)retsym->addr));
        }

        if(callsym != nullptr) {
            dprintf("CALL=(%x=%s:%i)\r\n", (unsigned int)calladdr, callsym->name, (int)((ptrdiff_t)calladdr - (ptrdiff_t)callsym->addr));
        }

        /* Get the forward chain */
        dprintf("%p\r\n", (uintptr_t)frame);
        frame = *((uint8_t **)(&frame[8]));
    }
    return 0;
}

#define VERSION_STRING "v1.0"

int main(int argc, char **argv)
{
    int i;

    for(i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "/VERSION")) {
            printf("UDOS Debug Kernel Module (DBK) " VERSION_STRING "\r\n");
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "Unknown option %s\r\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    /** @todo Ask kernel to elevate privileges */
    /** @todo Set breakpoint svc */
    /** @todo Daemon debugger */
    while(1) {

    }
    return 0;
}
