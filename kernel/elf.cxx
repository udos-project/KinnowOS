#include <elf.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <timeshr.hxx>
#include <arch/virtual.hxx>
#include <locale.hxx>
#include <errcode.hxx>

#define abs(x) (((x) < 0) ? (-x) : (x))

/// @brief This does not cover the nullptr case, however the ELF loader does not expect the buffer
/// to be at nullptr by any means. So we're very safe for now
#define IS_IN_BOUNDS(x, end) ((uintptr_t)(end) >= (uintptr_t)(x).hdr && (uintptr_t)(end) < (uintptr_t)(x).hdr + (x).size)

/// @brief Relative version of IS_IN_BOUNDS
#define IS_IN_BOUNDS_REL(x, offs) IS_IN_BOUNDS(x, (uintptr_t)(x).hdr + (uintptr_t)(offs))

namespace elf64 {
	static inline elf64::phdr *get_phdr(elf64::reader& rdr, size_t idx);
	static inline elf64::section_header *get_shdr(elf64::reader& rdr, size_t idx);
	static inline elf64::section_header *get_string_shdr(elf64::reader& rdr);
	static inline const char *get_string(elf64::reader& rdr, size_t offset, elf64::section_header *strtab = nullptr);
	static inline elf64::sym *get_symbol(elf64::reader& rdr, size_t table, size_t idx);
	static inline void *get_symbol_value(elf64::reader& rdr, size_t table, size_t idx);
	static inline int reloc_patch(elf64::reader& rdr, void *ref, uintptr_t offs, size_t type);
	static inline int do_reloc(elf64::reader& rdr, const elf64::rel& rel, const elf64::section_header& reltab);
	static inline int do_reloc_add(elf64::reader& rdr, const elf64::rela& rela, const elf64::section_header& reltab);
	static inline int load_section(elf64::reader& rdr, const elf64::section_header *shdr);
	static inline int load_rel_section(elf64::reader& rdr, const elf64::section_header *shdr);
}

static inline elf64::phdr *elf64::get_phdr(elf64::reader& rdr, size_t idx)
{
	auto *hdr = rdr.hdr;
	auto *phdr = reinterpret_cast<elf64::phdr *>((uintptr_t)hdr + (uintptr_t)hdr->prog_tab + (hdr->prog_tab_entry_size * idx));
	if(!IS_IN_BOUNDS(rdr, phdr)) {
		debug_printf("PHDR#%i\x01\x17", idx);
		return nullptr;
	}
	return phdr;
}

static inline elf64::section_header *elf64::get_shdr(elf64::reader& rdr, size_t idx)
{
	auto *hdr = rdr.hdr;
	auto *shdr = reinterpret_cast<elf64::section_header *>((uintptr_t)hdr + (uintptr_t)hdr->sect_tab + (hdr->sect_tab_entry_size * idx));
	if(!IS_IN_BOUNDS(rdr, shdr)) {
		debug_printf("SHDR#%i\x01\x17", idx);
		return nullptr;
	}
	return shdr;
}

static inline elf64::section_header *elf64::get_string_shdr(elf64::reader& rdr)
{
	auto *hdr = rdr.hdr;
	auto *shdr = elf64::get_shdr(rdr, (int)hdr->str_shtab_idx);
	if(!IS_IN_BOUNDS(rdr, shdr)) {
		debug_printf("SHDR#%i\x01\x17", (int)hdr->str_shtab_idx);
		return nullptr;
	}
	return shdr;
}

static inline const char *elf64::get_string(elf64::reader& rdr, size_t offset, elf64::section_header *strtab)
{
	auto *hdr = rdr.hdr;

	if(strtab == nullptr) strtab = elf64::get_string_shdr(rdr);
	if(!IS_IN_BOUNDS(rdr, strtab)) {
		debug_printf("STRTAB is\x01\x17/null");
		return nullptr;
	}

	const auto *str = reinterpret_cast<const char *>((uintptr_t)hdr + strtab->offset + offset);
	if(!IS_IN_BOUNDS(rdr, str)) {
		debug_printf("STR is\x01\x17/null");
		return nullptr;
	}

	static char tmpbuf[80];
	storage_string::copy(tmpbuf, str);
	locale::convert<char, locale::charset::ASCII, locale::charset::NATIVE>(tmpbuf);
	//return str;
	return &tmpbuf[0];
}

static inline elf64::sym *elf64::get_symbol(elf64::reader& rdr, size_t table, size_t idx)
{
	auto *hdr = rdr.hdr;
	auto *symtab = elf64::get_shdr(rdr, table);
	if(!IS_IN_BOUNDS(rdr, symtab)) {
		debug_printf("SYMTAB#%i\x01\x17", (int)table);
		return nullptr;
	}

	auto *sym = reinterpret_cast<elf64::sym *>((uintptr_t)hdr + symtab->offset + (idx * symtab->entsize));

	const auto *name = elf64::get_string(rdr, sym->name);
	debug_printf("GSYM:NAME=%s", name);
	name = elf64::get_string(rdr, sym->section_idx);
	debug_printf("GSYM:NAME=%s", name);
	name = elf64::get_string(rdr, sym->value);
	debug_printf("GSYM:NAME=%s", name);
	return sym;
}

static inline void *elf64::get_symbol_value(elf64::reader& rdr, size_t table, size_t idx)
{
	auto *hdr = rdr.hdr;
	auto *symtab = elf64::get_shdr(rdr, table);
	if(!IS_IN_BOUNDS(rdr, symtab)) {
		debug_printf("SYMTAB#%i\x01\x17", (int)table);
		return nullptr;
	}

	auto *sym = elf64::get_symbol(rdr, table, idx);
	if(!IS_IN_BOUNDS(rdr, sym)) {
		debug_printf("SYM#%i\x01\x17", (int)idx);
		return nullptr;
	}

	if(sym->section_idx == SHN_UNDEF) { // External symbol
		auto *strtab = elf64::get_shdr(rdr, symtab->link);
		if(!IS_IN_BOUNDS(rdr, strtab)) {
			debug_printf("STRTAB#%i\x01\x17", (int)symtab->link);
			return nullptr;
		}

		const auto *name = elf64::get_string(rdr, sym->name);
		if(name == nullptr) {
			debug_printf("Name\x01\x17");
			return nullptr;
		}

		/// @todo Obtain target from symbol lookup
		void *target = rdr.job->get_symbol(name)->address;
		if(target == nullptr) {
			if((sym->info >> 4) & elf::symbol_bindings::WEAK) {
				// Weak symbol is 0
				return nullptr;
			} else {
				debug_printf("Symbol %s not found", name);
				return nullptr;
			}
		} else {
			return target;
		}
	} else if(sym->section_idx == SHN_ABS) {
		return (void *)&sym->value;
	} else {
		auto *tarsect = elf64::get_shdr(rdr, sym->section_idx);
		if(!IS_IN_BOUNDS(rdr, tarsect)) {
			debug_printf("TARSECT#%i\x01\x17", (int)sym->section_idx);
			return nullptr;
		}

		// This target is not subject to any boundaries, however: */
		/// @todo We should probably do this entire elf loading in the user program instead of the kernel.
		auto *target = reinterpret_cast<void *>((uintptr_t)hdr + tarsect->offset + sym->value);
		return target;
	}
	return nullptr;
}

/// @brief Performs a relocation patch upon the specified reference and symvals, it is used by both
/// relocation and relocation with add functions, as to reduce duplication of code.
/// @param rdr Elf reader
/// @param ref Location to patch
/// @param offs Offset
/// @param type Type of patch
/// @return int Return code
static inline int elf64::reloc_patch(elf64::reader& rdr, void *ref, uintptr_t offs, size_t type)
{
	/** @todo Do ALL the relocations */
	switch(type) {
	case elf::reloc_types::S390_NONE:
		debug_printf("S390_NONE");
		break;
	case elf::reloc_types::S390_8:
		debug_printf("S390_8");
		*((uint8_t *)ref) = (uint8_t)((uint8_t)offs + *((uint8_t *)ref));
		break;
	case elf::reloc_types::S390_12:
		debug_printf("S390_12");
		// Clear the lower 12-bits and then OR the required relocation
		*((uint16_t *)ref) &= 0xf000;
		*((uint16_t *)ref) |= (uint16_t)((uint16_t)offs + *((uint16_t *)ref)) & 0xfff;
		break;
	case elf::reloc_types::S390_16:
		debug_printf("S390_16");
		*((uint16_t *)ref) = (uint16_t)((uint16_t)offs + *((uint16_t *)ref));
		break;
	case elf::reloc_types::S390_32:
		debug_printf("S390_32");
		*((uint32_t *)ref) = (uint32_t)((uint32_t)offs + *((uint32_t *)ref));
		break;
	case elf::reloc_types::S390_JMP_SLOT:
		debug_printf("S390_JMP_SLOT");
		debug_printf("plt_base=%p", rdr.plt_base);
		debug_printf("got_base=%p", rdr.got_base);
		// ADDR_SIZE_TYPE can be 64-bits or 32-bits
		break;
	default:
		debug_printf("S390_??? %u", (size_t)type);
		break;
	}
	return 0;
}

static inline int elf64::do_reloc(elf64::reader& rdr, const elf64::rel& rel, const elf64::section_header& reltab)
{
	auto *hdr = rdr.hdr;
	auto *target = elf64::get_shdr(rdr, reltab.info);
	if(!IS_IN_BOUNDS(rdr, target)) {
		debug_printf("TARGET#%i\x01\x17", (int)reltab.info);
		return error::INVALID_SETUP;
	}

	uintptr_t addr = (uintptr_t)hdr + target->offset;
	if(!IS_IN_BOUNDS(rdr, addr)) {
		debug_printf("ADDR+%i\x01\x17", (int)target->offset);
		return error::INVALID_SETUP;
	}

	auto *sym = elf64::get_symbol(rdr, reltab.link, rel.get_symbol_idx());

	/// @todo Obtain symvalue or something idk
	int symval = 0;
	kpanic("TODO: Obtain symvalue");

	auto *ref = reinterpret_cast<uint64_t *>(addr + rel.offset);
	if(elf64::reloc_patch(rdr, ref, (uintptr_t)symval, rel.get_type()) < 0) {
		debug_printf("\x01\x1D%i\x01\x22", (int)rel.info & 0xff);
		return error::INVALID_SETUP;
	}
	return 0;
}

static inline int elf64::do_reloc_add(elf64::reader& rdr, const elf64::rela& rela, const elf64::section_header& reltab)
{
	auto *hdr = rdr.hdr;
	auto *target = elf64::get_shdr(rdr, reltab.info);
	if(!IS_IN_BOUNDS(rdr, target)) {
		debug_printf("TARGET#%i\x01\x17", (int)reltab.info);
		return error::INVALID_SETUP;
	}

	uintptr_t addr = (uintptr_t)hdr + target->offset;
	if(!IS_IN_BOUNDS(rdr, addr)) {
		debug_printf("ADDR+%i\x01\x17", (int)target->offset);
		return error::INVALID_SETUP;
	}

	auto *sym = elf64::get_symbol(rdr, reltab.link, rela.get_symbol_idx());

	/** @todo Obtain symvalue or something idk */
	auto *ref = reinterpret_cast<uint64_t *>(addr + rela.offset);
	if(elf64::reloc_patch(rdr, ref, (uintptr_t)rela.addend, rela.get_type()) < 0) {
		debug_printf("\x01\x1D(Add) %i\x01\x22", (int)rela.info & 0xff);
		return error::INVALID_SETUP;
	}
	return 0;
}

static inline int elf64::load_section(elf64::reader& rdr, const elf64::section_header *shdr)
{
	auto *hdr = rdr.hdr;
	void *addr = nullptr;

	if(shdr->size == 0) return error::INVALID_PARAM;
	debug_printf("Section %s,flags=%p,type=%p,shdr_addr=%p,shdr_size=%u,rdr_size=%u", elf64::get_string(rdr, shdr->name), (uintptr_t)shdr->flags, (uintptr_t)shdr->type, (uintptr_t)shdr->addr, shdr->size, rdr.size);

	// Linked string table
	auto *lstrtab = elf64::get_shdr(rdr, shdr->link);

	// Offset to apply to the section
	auto sect_offset = reinterpret_cast<uintptr_t>(rdr.base);
	if(1) {
		sect_offset = 0;
	}

	// Symbol table
	if(shdr->type == elf::section_types::SYMTAB) {
		if(shdr->size == 0) return error::INVALID_SETUP; // Skip if the section is empty
		if(!IS_IN_BOUNDS_REL(rdr, shdr->offset)) {
			debug_printf("SHDR_OFFSET+%u\x01\x17", (size_t)shdr->offset);
			return error::INVALID_SETUP;
		}

		for(size_t off = 0; off < shdr->size; off += shdr->entsize) {
			const auto *sym = (const elf64::sym *)((uintptr_t)hdr + shdr->offset + off);
			if(!IS_IN_BOUNDS(rdr, sym)) {
				debug_printf("SYM#%i\x01\x17", (int)off);
				continue;
			}

			debug_printf("SYMBOL %s(STR=%u),SIZE=%u,OTHER=%u,VALUE=%p,INFO=%u", elf64::get_string(rdr, sym->name, lstrtab), (size_t)sym->name, (size_t)sym->size, (size_t)sym->other, (uintptr_t)sym->value, (size_t)sym->info);
			if(sym->section_idx == elf::symbol_shndx::UNDEFINED) {
				// Externally resolveable symbol
				debug_printf("TODO: Resolve external symbols");
			}

			// Add symbol into job
			if(sym->get_type() == elf::symbol_types::FUNC || sym->get_type() == elf::symbol_types::OBJECT) {
				storage::symbol as_sym;
				storage_string::copy(as_sym.name, elf64::get_string(rdr, sym->name, lstrtab));
				// Get correct address
				if(sym->section_idx == elf::symbol_shndx::ABSOLUTE) {
					// No change, absolute address
					as_sym.address = reinterpret_cast<void *>(static_cast<uintptr_t>(sym->value));
				} else {
					// Apply base + symbol address for PIC symbols
					as_sym.address = reinterpret_cast<void *>(static_cast<uintptr_t>(sym->value) + reinterpret_cast<uintptr_t>(rdr.base));
				}
				as_sym.size = static_cast<size_t>(sym->size);
				rdr.job->symbols.insert(as_sym);
			} else {
				debug_printf("*** IGNORED ***");
			}
		}
	}
	// These sections are present on the file
	else if(shdr->type == elf::section_types::PROGBITS) {
		if(!IS_IN_BOUNDS_REL(rdr, shdr->offset)) {
			debug_printf("SHDR_OFFSET+%u\x01\x17", (size_t)shdr->offset);
			return error::INVALID_SETUP;
		}

		// Load the section image accordingly
		if(shdr->flags & elf::section_flags::ALLOC) {
			// Source address to copy from (inside the ELF image)
			const auto *src_addr = reinterpret_cast<const void *>((uintptr_t)hdr + shdr->offset);
			if(!IS_IN_BOUNDS(rdr, src_addr)) {
				debug_printf("SRC_ADDR+%p\x01\x17", src_addr);
				return error::INVALID_SETUP;
			}

			// Destination address to copy to
			size_t align = (size_t)shdr->addralign; // Make sure to meet alignment criteria
			if(align < virtual_storage::page_align) align = virtual_storage::page_align;

			void *dest_addr = real_storage::alloc((size_t)shdr->size, align);
			if(dest_addr == nullptr) return error::INVALID_SETUP;
			// Copy to whatever addr is pointing to, no questions asked */
			/// @todo Ask questions and don't let userspace programs overwrite the kernel!
			debug_printf("LOADER-MOVE %p->(VIRT=%p,REAL=%p)", src_addr, reinterpret_cast<void *>((uintptr_t)shdr->addr), dest_addr);
			if(!storage_string::compare(elf64::get_string(rdr, shdr->name), ".plt")) {
				rdr.plt_base = dest_addr;
			} else if(!storage_string::compare(elf64::get_string(rdr, shdr->name), ".got")) {
				rdr.got_base = dest_addr;
			}

			// Move real storage src into dest and then map it so it looks like it's on shdr->addr
			storage::copy(dest_addr, src_addr, (size_t)shdr->size);
			if(rdr.job->aspace != nullptr) {
				auto *vaddr = reinterpret_cast<void *>((sect_offset + static_cast<uintptr_t>(shdr->addr)) & (~(virtual_storage::page_align - 1)));
				auto *paddr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(dest_addr) & (~(virtual_storage::page_align - 1)));
				if((shdr->addr & 0xfff) != 0)
					debug_printf("WARNING: Non-aligned address %p", (uintptr_t)shdr->addr);
				rdr.job->map_range(vaddr, paddr, 0, (size_t)shdr->size);
			}
		}
	}
	// Sections not present on the file
	else if(shdr->type == elf::section_types::NOBITS) {
		if(shdr->size == 0) return error::INVALID_SETUP; // Skip if the section is empty

		/// @todo Map pages to the address in the virtual space
		// Sections that needs to be allocated
		if(shdr->flags & elf::section_flags::ALLOC) {
			// Obtain memory from the real storage
			addr = real_storage::alloc((size_t)shdr->size, (size_t)shdr->addralign);
			if(addr == nullptr) return error::INVALID_SETUP;
			debug_printf("LOADER-CLEAR VIRT=%p,REAL=%p(%u)", (void *)(static_cast<uintptr_t>(shdr->addr)), addr, (size_t)shdr->size);
			if(!storage_string::compare(elf64::get_string(rdr, shdr->name), ".plt")) {
				rdr.plt_base = addr;
			} else if(!storage_string::compare(elf64::get_string(rdr, shdr->name), ".got")) {
				rdr.got_base = addr;
			}

			// Clear the section and then map it on the ASPACE of the job
			storage::fill(addr, 0, (size_t)shdr->size);
			if(rdr.job->aspace != nullptr) {
				auto *vaddr = reinterpret_cast<void *>((sect_offset + static_cast<uintptr_t>(shdr->addr)) & (~(virtual_storage::page_align - 1)));
				auto *paddr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(addr) & (~(virtual_storage::page_align - 1)));
				if((shdr->addr & 0xfff) != 0)
					debug_printf("\x01\x15Non-aligned address %p", (uintptr_t)shdr->addr);
				rdr.job->map_range(vaddr, paddr, 0, (size_t)shdr->size);
			}
		}
	}
	return 0;
}

static inline int elf64::load_rel_section(elf64::reader& rdr, const elf64::section_header *shdr)
{
	auto *hdr = rdr.hdr;
	void *addr = nullptr;

	if(shdr->size == 0) return error::INVALID_PARAM;
	debug_printf("Section %s,FLAGS=%p,TYPE=%p,ADD=%p,SIZE=%u,RSIZE=%u", elf64::get_string(rdr, shdr->name), (uintptr_t)shdr->flags, (uintptr_t)shdr->type, (uintptr_t)shdr->addr, shdr->size, rdr.size);

	// Sections for relocation
	if(shdr->type == elf::section_types::REL) {
		debug_printf("\x01\x1D section!");
		if(shdr->entsize != sizeof(elf64::rel))
			debug_printf("\x01\x15 entsize(%u) is not equal to the rel_ent size(%u)", (size_t)shdr->entsize, sizeof(elf64::rel));
		for(size_t off = 0; off < shdr->size; off += shdr->entsize) {
			const auto *rel = (const elf64::rel *)((uintptr_t)hdr + shdr->offset + off);
			if(!IS_IN_BOUNDS(rdr, rel)) {
				debug_printf("REL#%i\x01\x17", (int)off);
				continue;
			}

			if(elf64::do_reloc(rdr, *rel, *shdr) < 0) {
				debug_printf("REL#%i failed", (int)off);
				continue;
			}
		}
	} else if(shdr->type == elf::section_types::RELA) {
		debug_printf("\x01\x1D (Added) section!");
		if(shdr->entsize != sizeof(elf64::rela))
			debug_printf("\x01\x15 entsize(%u) is not equal to the rel_ent size(%u)", (size_t)shdr->entsize, sizeof(elf64::rela));
		for(size_t off = 0; off < shdr->size; off += shdr->entsize) {
			const auto *rela = (const elf64::rela *)((uintptr_t)hdr + shdr->offset + off);
			if(!IS_IN_BOUNDS(rdr, rela)) {
				debug_printf("RELA#%i\x01\x17", (int)off);
				continue;
			}

			if(elf64::do_reloc_add(rdr, *rela, *shdr) < 0) {
				debug_printf("RELA#%i failed", (int)off);
				continue;
			}
		}
	}
	return 0;
}

int elf64::check_valid(elf64::reader& rdr)
{
	const auto *hdr = rdr.hdr;
	/* Check signature of ELF file */
	if(storage::compare(&hdr->id, ELF_MAGIC, 4) != 0)
		return error::INVALID_SETUP;
	return 0;
}

int elf64::load(timeshare::job *job, void *buffer, size_t n, void **entry)
{
	elf64::reader rdr;
	rdr.hdr = reinterpret_cast<elf64::header *>(buffer);
	rdr.size = n;
	rdr.base = (void *)0x80000;
	rdr.job = job;
	auto *hdr = rdr.hdr;

	debug_printf("\x01\x1E ELF64 buffer=%p,n=%u,entryPtr=%p", rdr.hdr, rdr.size, entry);

	// Check validity of the ELF
	if(elf64::check_valid(rdr) != 0) {
		debug_printf("\x01\x13 ELF64 file");
		return error::INVALID_SETUP;
	}

	for(size_t i = 0; i < hdr->n_prog_tab_entry; i++) {
		const auto *phdr = elf64::get_phdr(rdr, i);
		if(!IS_IN_BOUNDS(rdr, phdr)) {
			debug_printf("PHDR#%i\x01\x17", (int)i);
			return error::INVALID_SETUP;
		}

		// Specifies an interpreter to open, to interpret this file
		if(phdr->flags == elf::prgram_flags::INTERP) {
			char *interp_dsname = storage::alloc<char>(phdr->file_size + 1);
			if(interp_dsname == nullptr) return error::ALLOCATION;
			storage::copy(interp_dsname, (const void *)((uintptr_t)rdr.hdr + phdr->offset), (size_t)phdr->file_size);
			if(interp_dsname[phdr->file_size - 1] != '\0') {
				debug_printf("\x01\x15 interp_dsname does not have a terminating null character");
				interp_dsname[phdr->file_size] = '\0';
			}
			debug_assertm(0, "Interpreter: %s", interp_dsname);
			storage::free(interp_dsname);
		}
	}

	// Iterate over the sections and load them on the storage
	debug_printf("SECT_TAB,N=%u,SIZE=%u,SHDR=%p", (size_t)hdr->n_sect_tab_entry, (size_t)hdr->sect_tab_entry_size, (uintptr_t)hdr->sect_tab);
	for(size_t i = 0; i < hdr->n_sect_tab_entry; i++) {
		const auto *shdr = elf64::get_shdr(rdr, i);
		if(!IS_IN_BOUNDS(rdr, shdr)) {
			debug_printf("SHDR#%i\x01\x17", (int)i);
			return error::INVALID_SETUP;
		}
		elf64::load_section(rdr, shdr);
	}

	// After loading everything, proceed to perform relocations
	debug_printf("\x01\x1C\x01\x1D AFTER\x01\x1E");
	debug_printf("SECT_TAB,N=%u,SIZE=%u,SHDR=%p", (size_t)hdr->n_sect_tab_entry, (size_t)hdr->sect_tab_entry_size, (uintptr_t)hdr->sect_tab);
	for(size_t i = 0; i < hdr->n_sect_tab_entry; i++) {
		const auto *shdr = elf64::get_shdr(rdr, i);
		if(!IS_IN_BOUNDS(rdr, shdr)) {
			debug_printf("SHDR#%i\x01\x17", (int)i);
			return error::INVALID_SETUP;
		}
		elf64::load_rel_section(rdr, shdr);
	}

	debug_printf("Entry=%p", (uintptr_t)hdr->entry);
	*entry = (void *)hdr->entry;
	return 0;
}
