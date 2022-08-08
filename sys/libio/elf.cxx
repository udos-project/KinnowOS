#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"

#define abs(x) (((x) < 0) ? (-x) : (x))

/**
 * @brief This does not cover the nullptr case, however the ELF loader does not expect the buffer
 * to be at nullptr by any means. So we're very safe for now
 * 
 */
#define IS_IN_BOUNDS(x, end) ((uintptr_t)(end) >= (uintptr_t)(x)->hdr && (uintptr_t)(end) < (uintptr_t)(x)->hdr + (x)->size)

/**
 * @brief Relative version of IS_IN_BOUNDS
 * 
 */
#define IS_IN_BOUNDS_REL(x, offs) IS_IN_BOUNDS(x, (uintptr_t)(x)->hdr + (uintptr_t)(offs))

static void *dl_lookup_symbol(const char *name)
{
    return nullptr;
}

static struct elf64_phdr *elf64_get_phdr(struct elf64_reader *rdr, unsigned int idx)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_phdr *phdr;

    phdr = (struct elf64_phdr *)((uintptr_t)hdr + (uintptr_t)hdr->prog_tab + (hdr->prog_tab_entry_size * idx));
    if(!IS_IN_BOUNDS(rdr, phdr)) {
        dprintf("PHDR#%i out of bounds", idx);
        return nullptr;
    }
    return phdr;
}

static struct elf64_shdr *elf64_get_shdr(struct elf64_reader *rdr, unsigned int idx)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_shdr *shdr;

    shdr = (struct elf64_shdr *)((uintptr_t)hdr + (uintptr_t)hdr->sect_tab + (hdr->sect_tab_entry_size * idx));
    if(!IS_IN_BOUNDS(rdr, shdr)) {
        dprintf("SHDR#%i out of bounds", idx);
        return nullptr;
    }
    return shdr;
}

static struct elf64_shdr *elf64_get_string_shdr(struct elf64_reader *rdr)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_shdr *shdr;

    shdr = elf64_get_shdr(rdr, (int)hdr->str_shtab_idx);
    if(!IS_IN_BOUNDS(rdr, shdr)) {
        dprintf("SHDR#%i out of bounds", (int)hdr->str_shtab_idx);
        return nullptr;
    }
    return shdr;
}

static const char *elf64_get_string(struct elf64_reader *rdr, unsigned int offset)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_shdr *strtab;
    const char *str;

    strtab = elf64_get_string_shdr(rdr);
    if(!IS_IN_BOUNDS(rdr, strtab)) {
        dprintf("STRTAB is out of bounds/null");
        return nullptr;
    }

    str = (const char *)((uintptr_t)hdr + strtab->offset + offset);
    if(!IS_IN_BOUNDS(rdr, str)) {
        dprintf("STR is out of bounds/null");
        return nullptr;
    }
    return str;
}

static struct elf64_sym *elf64_get_symbol(struct elf64_reader *rdr, unsigned int table, unsigned int idx)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_shdr *symtab;
    struct elf64_sym *sym;

    if(table == SHN_UNDEF || idx == SHN_UNDEF) {
        return nullptr;
    }

    symtab = elf64_get_shdr(rdr, table);
    if(!IS_IN_BOUNDS(rdr, symtab)) {
        dprintf("SYMTAB#%i out of bounds", (int)table);
        return nullptr;
    }

    sym = (struct elf64_sym *)((uintptr_t)hdr + symtab->offset + (idx * symtab->entsize));
    return sym;
}

static void *elf64_get_symbol_value(struct elf64_reader *rdr, unsigned int table, unsigned int idx)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_shdr *symtab;
    struct elf64_sym *sym;

    symtab = elf64_get_shdr(rdr, table);
    if(!IS_IN_BOUNDS(rdr, symtab)) {
        dprintf("SYMTAB#%i out of bounds", (int)table);
        return nullptr;
    }

    sym = elf64_get_symbol(rdr, table, idx);
    if(!IS_IN_BOUNDS(rdr, sym)) {
        dprintf("SYM#%i out of bounds", (int)idx);
        return nullptr;
    }

    if(sym->section_idx == SHN_UNDEF) {
        /* External symbol */
        struct elf64_shdr *strtab;
        const char *name;
        void *target;

        strtab = elf64_get_shdr(rdr, symtab->link);
        if(!IS_IN_BOUNDS(rdr, strtab)) {
            dprintf("STRTAB#%i out of bounds", (int)symtab->link);
            return nullptr;
        }

        name = (const char *)((uintptr_t)hdr + strtab->offset + sym->name);
        if(!IS_IN_BOUNDS(rdr, name)) {
            dprintf("Name out of bounds");
            return nullptr;
        }

        /** @todo Obtain target from symbol lookup */
        target = dl_lookup_symbol(name);
        if(target == nullptr) {
            if((sym->info >> 4) & STB_WEAK) {
                /* Weak symbol is 0 */
                return nullptr;
            } else {
                dprintf("Symbol %s not found", name);
                return nullptr;
            }
        } else {
            return target;
        }
    } else if(sym->section_idx == SHN_ABS) {
        return (void *)&sym->value;
    } else {
        struct elf64_shdr *tarsect;
        void *target;
        tarsect = elf64_get_shdr(rdr, sym->section_idx);
        if(!IS_IN_BOUNDS(rdr, tarsect)) {
            dprintf("TARSECT#%i out of bounds", (int)sym->section_idx);
            return nullptr;
        }

        /* This target is not subject to any boundaries, however: */
        /** @todo We should probably do this entire elf loading in the user program
         * instead of the kernel. */
        target = (void *)((uintptr_t)hdr + tarsect->offset + sym->value);
        return target;
    }
    return nullptr;
}


/**
 * @brief Performs a relocation patch upon the specified reference and symvals, it is used by both
 * relocation and relocation with add functions, as to reduce duplication of code.
 * 
 * @param rdr Elf reader
 * @param ref Location to patch
 * @param offs Offset
 * @param type Type of patch
 * @return int Return code
 */
static int elf64_reloc_patch(struct elf64_reader *rdr, void *ref, uintptr_t offs, uint8_t type)
{
    /** @todo Do ALL the relocations */
    switch(type) {
    case R_390_NONE:
        dprintf("R_390_NONE");
        break;
    case R_390_8:
        dprintf("R_390_8");
        *((uint8_t *)ref) = (uint8_t)((uint8_t)offs + *((uint8_t *)ref));
        break;
    case R_390_12:
        dprintf("R_390_12");
        /* Clear the lower 12-bits and then OR the required relocation */
        *((uint16_t *)ref) &= 0xf000;
        *((uint16_t *)ref) |= (uint16_t)((uint16_t)offs + *((uint16_t *)ref)) & 0xfff;
        break;
    case R_390_16:
        dprintf("R_390_16");
        *((uint16_t *)ref) = (uint16_t)((uint16_t)offs + *((uint16_t *)ref));
        break;
    case R_390_32:
        dprintf("R_390_32");
        *((uint32_t *)ref) = (uint32_t)((uint32_t)offs + *((uint32_t *)ref));
        break;
    default:
        dprintf("R_390_??? %i", (int)type);
        break;
    }
    return 0;
}

static int elf64_do_reloc(struct elf64_reader *rdr, const struct elf64_rel* rel, const struct elf64_shdr *reltab)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_shdr *target;
    uintptr_t addr;
    uint64_t *ref;
    int symval = 0;

    target = elf64_get_shdr(rdr, reltab->info);
    if(!IS_IN_BOUNDS(rdr, target)) {
        dprintf("TARGET#%i out of bounds", (int)reltab->info);
        return -1;
    }

    addr = (uintptr_t)hdr + target->offset;
    if(!IS_IN_BOUNDS(rdr, addr)) {
        dprintf("ADDR+%i out of bounds", (int)target->offset);
        return -1;
    }

    /** @todo Obtain symvalue or something idk */
    fprintf(stderr, "TODO: Obtain symvalue");
    while(1);

    ref = (uint64_t *)(addr + rel->offset);
    if(elf64_reloc_patch(rdr, ref, (uintptr_t)symval, (uint8_t)(rel->info & 0xff)) < 0) {
        dprintf("Relocation %i failed!", (int)rel->info & 0xff);
        return -1;
    }
    return 0;
}

static int elf64_do_reloc_add(struct elf64_reader *rdr, const struct elf64_rela* rela, const struct elf64_shdr *reltab)
{
    struct elf64_header *hdr = rdr->hdr;
    struct elf64_shdr *target;
    uintptr_t addr;
    uint64_t *ref;

    target = elf64_get_shdr(rdr, reltab->info);
    if(!IS_IN_BOUNDS(rdr, target)) {
        dprintf("TARGET#%i out of bounds", (int)reltab->info);
        return -1;
    }

    addr = (uintptr_t)hdr + target->offset;
    if(!IS_IN_BOUNDS(rdr, addr)) {
        dprintf("ADDR+%i out of bounds", (int)target->offset);
        return -1;
    }

    /** @todo Obtain symvalue or something idk */
    ref = (uint64_t *)(addr + rela->offset);
    if(elf64_reloc_patch(rdr, ref, (uintptr_t)rela->addend, (uint8_t)(rela->info & 0xff)) < 0) {
        dprintf("Relocation (Add) %i failed!", (int)rela->info & 0xff);
        return -1;
    }
    return 0;
}

static int elf64_load_section(struct elf64_reader *rdr, const struct elf64_shdr *shdr)
{
    struct elf64_header *hdr = rdr->hdr;
    void *addr = nullptr;
    size_t i;

    if(shdr->size == 0) {
        return -1;
    }

    dprintf("Section %s,flags=%p,type=%p,shdr_addr=%p,shdr_size=%u,rdr_size=%u", elf64_get_string(rdr, shdr->name), (uintptr_t)shdr->flags, (uintptr_t)shdr->type, (uintptr_t)shdr->addr, shdr->size, rdr->size);

    /* These sections are present on the file */
    if(shdr->type == SHT_PROGBITS) {
        if(!IS_IN_BOUNDS_REL(rdr, shdr->offset)) {
            dprintf("SHDR_OFFSET+%u out of bounds", (size_t)shdr->offset);
            return -1;
        }

        /* Load the section image accordingly */
        if(shdr->flags & SHF_ALLOC) {
            const void *src_addr;
            void *dest_addr;

            /* Source address to copy from (inside the ELF image) */
            src_addr = (const void *)((uintptr_t)hdr + shdr->offset);
            if(!IS_IN_BOUNDS(rdr, src_addr)) {
                dprintf("SRC_ADDR+%p out of bounds", src_addr);
                return -1;
            }

            /* Destination address to copy to */
            dest_addr = (void *)shdr->addr;
            if(dest_addr == nullptr) {
                return -1;
            }

            /* Copy to whatever addr is pointing to, no questions asked */
            /** @todo Ask questions and don't let userspace programs overwrite the kernel!!!! */
            dprintf("LOADER-MOVE %p->%p", src_addr, dest_addr);
            memcpy(dest_addr, (void *)src_addr, (size_t)shdr->size);
        }
    }
    /* Sections not present on the file */
    else if(shdr->type == SHT_NOBITS) {
        /* Skip if the section is empty */
        if(shdr->size == 0) {
            return -1;
        }

        /** @todo Map pages to the address in the virtual space */
        /* Sections that needs to be allocated */
        if(shdr->flags & SHF_ALLOC) {
            /** @todo Make this be virtual memory!!! */
            /*addr = pmalloc((size_t)shdr->size, (size_t)shdr->addralign);*/
            addr = (void *)((uintptr_t)shdr->addr);
            if(addr == nullptr) {
                return -1;
            }
            dprintf("LOADER-CLEAR %p(%u)", addr, (size_t)shdr->size);
            memset(addr, 0, (size_t)shdr->size);
        }
    }
    /* Sections for relocation */
    else if(shdr->type == SHT_REL) {
        dprintf("Relocation section!");
#if defined DEBUG
        if(shdr->entsize != sizeof(struct elf64_rel)) {
            dprintf("Warning: entsize is not equal to the rel_ent size");
        }
#endif
        for(i = 0; i < shdr->size / shdr->entsize; i++) {
            const struct elf64_rel* rel;
            rel = (const struct elf64_rel *)((uintptr_t)hdr + shdr->offset + (i * shdr->entsize));
            if(!IS_IN_BOUNDS(rdr, rel)) {
                dprintf("REL#%i out of bounds", (int)i);
                continue;
            }

            if(elf64_do_reloc(rdr, rel, shdr) < 0) {
                dprintf("REL#%i failed", (int)i);
                continue;
            }
        }
    }
    else if(shdr->type == SHT_RELA) {
        dprintf("Relocation (Added) section!");
#if defined DEBUG
        if(shdr->entsize != sizeof(struct elf64_rela)) {
            dprintf("Warning: entsize is not equal to the rel_ent size");
        }
#endif
        for(i = 0; i < shdr->size / shdr->entsize; i++) {
            const struct elf64_rela* rela;
            rela = (const struct elf64_rela *)((uintptr_t)hdr + shdr->offset + (i * shdr->entsize));
            if(!IS_IN_BOUNDS(rdr, rela)) {
                dprintf("RELA#%i out of bounds", (int)i);
                continue;
            }

            if(elf64_do_reloc_add(rdr, rela, shdr) < 0) {
                dprintf("RELA#%i failed", (int)i);
                continue;
            }
        }
    }
    return 0;
}

int elf64_check_valid(struct elf64_reader *rdr)
{
    struct elf64_header *hdr = rdr->hdr;
    /* Check signature of ELF file */
    if(memcmp(&hdr->id, ELF_MAGIC, 4) != 0) {
        return -1;
    }
    return 0;
}

int elf64_load(void *buffer, size_t n, void **entry)
{
    struct elf64_reader rdr;
    struct elf64_header *hdr;
    size_t i;

    rdr.hdr = (struct elf64_header *)buffer;
    rdr.size = n;
    rdr.base = (void *)0x80000;
    hdr = rdr.hdr;

    dprintf("Loading ELF64 buffer=%p,n=%u,entryPtr=%p", rdr.hdr, rdr.size, entry);

    /* Check validity of the ELF */
    if(elf64_check_valid(&rdr) != 0) {
        fprintf(stderr, "Invalid ELF64 file\r\n");
        return -1;
    }

    for(i = 0; i < hdr->n_prog_tab_entry; i++) {
        struct elf64_phdr *phdr;
        phdr = elf64_get_phdr(&rdr, i);
        if(!IS_IN_BOUNDS(&rdr, phdr)) {
            dprintf("PHDR#%i out of bounds", (int)i);
            return -1;
        }

        /* Specifies an interpreter to open, to interpret this file */
        if(phdr->flags == PT_INTERP) {
            auto *interp_dsname = (char *)malloc(phdr->file_size + 1);
            if(interp_dsname == nullptr) {
                return -1;
            }
            memcpy(interp_dsname, (void *)((uintptr_t)rdr.hdr + phdr->offset), (size_t)phdr->file_size);
            if(interp_dsname[phdr->file_size - 1] != '\0') {
                dprintf("Warning: interp_dsname does not have a terminating null character");
                interp_dsname[phdr->file_size] = '\0';
            }
            dprintf("Interpreter: %s", interp_dsname);
            free(interp_dsname);
            while(1);
        }
    }

    /* Iterate over the sections and load them on the storage */
    for(i = 0; i < hdr->n_sect_tab_entry; i++) {
        struct elf64_shdr *shdr;
        shdr = elf64_get_shdr(&rdr, i);
        if(!IS_IN_BOUNDS(&rdr, shdr)) {
            dprintf("SHDR#%i out of bounds", (int)i);
            return -1;
        }
        /* Sections not present on the file */
        elf64_load_section(&rdr, shdr);
    }

    dprintf("Entry=%p", (uintptr_t)hdr->entry);
    *entry = (void *)hdr->entry;
    return 0;
}
