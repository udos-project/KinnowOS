#ifndef __LIBIO_ELF_H__
#define __LIBIO_ELF_H__ 1

#include <stdint.h>
#include <stddef.h>

/* The string "ELF" in ASCII */
#define ELF_MAGIC "\x7F\x45\x4c\x46"

#define SHN_UNDEF 0
#define SHN_ABS 0xFFF1

enum elf_machine_types {
    EM_NONE = 0x00,
    EM_SPARC = 0x02,
    EM_X86 = 0x03,
    EM_MIPS = 0x08,
    EM_PPCW = 0x14,
    EM_ARM = 0x28,
    EM_SH = 0x2A,
    EM_IA64 = 0x32,
    EM_X86_64 = 0x3E,
    EM_AARCH64 = 0x87,
    EM_RISCV = 0xF3
};

enum elf_symbol_bindings {
    STB_LOCAL = 0,
    STB_GLOBAL = 1,
    STB_WEAK = 2
};

enum elf_symbol_types {
    STT_NOTYPE = 0,
    STT_OBJECT = 1,
    STT_FUNC = 2
};

enum elf_section_types {
    SHT_NULL = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB = 2,
    SHT_STRTAB = 3,
    SHT_RELA = 4,
    SHT_NOBITS = 8,
    SHT_REL = 9,
};

enum elf_section_flags {
    SHF_WRITE = 0x01,
    SHF_ALLOC = 0x02,
    SHF_STRINGS = 0x20
};

enum elf_prgram_flags {
    PT_NULL = 0x00,
    PT_LOAD = 0x01,
    PT_DYNAMIC = 0x02,
    PT_INTERP = 0x03,
    PT_NOTE = 0x04,
    PT_SHLIB = 0x05,
    PT_PHDR = 0x06
};

enum elf_reloc_types {
    R_390_NONE = 0,
    R_390_8 = 1,
    R_390_12 = 2,
    R_390_16 = 3,
    R_390_32 = 4,
    R_390_PC32 = 5,
    R_390_GOT12 = 6,
    R_390_GOT32 = 7,
    R_390_PLT32 = 8,
    R_390_COPY = 9,
    R_390_GLOB_DAT = 10,
    R_390_JMP_SLOT = 11,
    R_390_RELATIVE = 12,
    R_390_GOTOFF = 13,
    R_390_GOTPC = 14,
    R_390_GOT16 = 15,
    R_390_PC16 = 16,
    R_390_PC16DBL = 17,
    R_390_PLT16DBL = 18
};

/* ELF bits */
#define ELF_HEADER(x) \
struct elf ##x ## _header { \
    uint8_t id[4]; \
    uint8_t bits; \
    uint8_t endian; \
    uint8_t hdr_version; \
    uint8_t abi; \
    uint64_t unused; \
    uint16_t type; \
    uint16_t instr_set; \
    uint32_t version; \
    uint ##x ## _t entry; \
    uint ##x ## _t prog_tab; \
    uint ##x ## _t sect_tab; \
    uint32_t flags; \
    uint16_t hdr_size; \
    uint16_t prog_tab_entry_size; \
    uint16_t n_prog_tab_entry; \
    uint16_t sect_tab_entry_size; \
    uint16_t n_sect_tab_entry; \
    uint16_t str_shtab_idx; \
};

#define ELF_SHDR(x) \
struct elf ##x ## _shdr { \
    uint32_t name; \
    uint32_t type; \
    uint ##x ## _t flags; \
    uint ##x ## _t addr; \
    uint ##x ## _t offset; \
    uint ##x ## _t size; \
    uint32_t link; \
    uint32_t info; \
    uint ##x ## _t addralign; \
    uint ##x ## _t entsize; \
};

struct elf32_phdr {
    uint32_t type;
    uint32_t offset;
    uint32_t v_addr;
    uint32_t p_addr;
    uint32_t file_size;
    uint32_t mem_size;
    uint32_t flags;
    uint32_t align;
};

struct elf64_phdr {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t v_addr;
    uint64_t p_addr;
    uint64_t file_size;
    uint64_t mem_size;
    uint64_t align;
};

struct elf32_sym {
    uint32_t name;
    uint32_t value;
    uint32_t size;
    uint8_t info;
    uint8_t other;
    uint16_t section_idx;
};

struct elf64_sym {
    uint32_t name;
    uint8_t info;
    uint8_t other;
    uint16_t section_idx;
    uint64_t value;
    uint64_t size;
};

#define ELF_REL(x) \
struct elf ##x ## _rel { \
    uint ##x ## _t offset; \
    uint32_t info; \
};

#define ELF_RELA(x) \
struct elf ##x ## _rela { \
    uint ##x ## _t offset; \
    uint ##x ## _t info; \
    int32_t addend; \
};

ELF_HEADER(32)
ELF_SHDR(32)
ELF_REL(32)
ELF_RELA(32)

ELF_HEADER(64)
ELF_SHDR(64)
ELF_REL(64)
ELF_RELA(64)

struct elf64_reader {
    struct elf64_header *hdr;
    size_t size;
    void *base;
};

int elf64_check_valid(struct elf64_reader *rdr);
int elf64_load(void *buffer, size_t n, void **entry);

#endif
