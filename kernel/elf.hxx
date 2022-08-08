#ifndef ELF_HXX
#define ELF_HXX

#include <types.hxx>
#include <arch/asm.hxx>
#include <timeshr.hxx>

#define ELF_MAGIC "\x7F\x45\x4c\x46" // The string "ELF" in ASCII

#define SHN_UNDEF 0
#define SHN_ABS 0xFFF1

namespace elf {
	template<typename T>
	struct header {
		using width_t = T;
		uint8_t id[4];
		uint8_t bits;
		uint8_t endian;
		uint8_t hdr_version;
		uint8_t abi;
		uint64_t unused;
		uint16_t type;
		uint16_t instr_set;
		uint32_t version;
		width_t entry;
		width_t prog_tab;
		width_t sect_tab;
		uint32_t flags;
		uint16_t hdr_size;
		uint16_t prog_tab_entry_size;
		uint16_t n_prog_tab_entry;
		uint16_t sect_tab_entry_size;
		uint16_t n_sect_tab_entry;
		uint16_t str_shtab_idx;
	} PACKED;

	namespace section_types {
		enum {
			NONE = 0,
			PROGBITS = 1,
			SYMTAB = 2,
			STRTAB = 3,
			RELA = 4,
			NOBITS = 8,
			REL = 9,
		};
	}

	namespace section_flags {
		enum {
			WRITE = 1 << 0, // Writeable
			ALLOC = 1 << 1, // Requires to be allocated
			EXECUTABLE = 1 << 2, // Executable
			MERGE = 1 << 3, // Might be merged with other sections
			STRINGS = 1 << 5, // Contains null-terminated strings
			EXCLUDE = 1U << 31, // Section is excluded unless referenced or allocated (Solaris?)
		};
	}

	template<typename T>
	struct section_header {
		using width_t = T;
		uint32_t name;
		uint32_t type;
		width_t flags;
		width_t addr;
		width_t offset;
		width_t size;
		uint32_t link;
		uint32_t info;
		width_t addralign;
		width_t entsize;
	} PACKED;

	template<typename T>
	struct rel {
		using width_t = T;
		width_t offset;
		uint32_t info;

		constexpr int get_symbol_idx(void) const {
			if constexpr(sizeof(T) >= sizeof(uint64_t)) {
				return static_cast<int>(this->info >> 32);
			} else {
				return static_cast<int>(this->info >> 8);
			}
		}

		constexpr int get_type(void) const {
			if constexpr(sizeof(T) >= sizeof(uint64_t)) {
				return static_cast<int>(this->info & 0xffffffff);
			} else {
				return static_cast<int>(this->info & 0xff);
			}
		}
	} PACKED;

	template<typename T>
	struct rela {
		using width_t = T;
		width_t offset;
		width_t info;
		int32_t addend;

		constexpr int get_symbol_idx(void) const {
			if constexpr(sizeof(T) >= sizeof(uint64_t)) {
				return static_cast<int>(this->info >> 32);
			} else {
				return static_cast<int>(this->info >> 8);
			}
		}

		constexpr int get_type(void) const {
			if constexpr(sizeof(T) >= sizeof(uint64_t)) {
				return static_cast<int>(this->info & 0xffffffff);
			} else {
				return static_cast<int>(this->info & 0xff);
			}
		}
	} PACKED;

	enum machine_types {
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

	namespace symbol_bindings {
		enum {
			LOCAL = 0,
			GLOBAL = 1,
			WEAK = 2,
			GNU_UNIQUE = 10,
		};
	}

	namespace symbol_types {
		enum {
			NOTYPE = 0,
			OBJECT = 1,
			FUNC = 2,
			SECTION = 3, // Symbol is a section
			FILE = 4, // Name is a filename
			COMMON = 5,
			TLS = 6, // Thread-local-storage
			GNU_IFUNC = 10,
		};
	}

	namespace symbol_shndx {
		enum {
			UNDEFINED = 0, // Undefined
			ABSOLUTE = 0xfff1, // Absolute positioned symbol
			COMMON = 0xfff2, // Common symbol
			XINDEX = 0xffff, // Index is in another table
		};
	};

	namespace prgram_flags {
		enum {
			NONE = 0,
			LOAD = 1,
			DYNAMIC = 2,
			INTERP = 3,
			NOTE = 4,
			SHLIB = 5,
			PHDR = 6
		};
	}

	namespace reloc_types {
		enum {
			S390_NONE = 0,
			S390_8 = 1,
			S390_12 = 2,
			S390_16 = 3,
			S390_32 = 4,
			S390_PC32 = 5,
			S390_GOT12 = 6,
			S390_GOT32 = 7,
			S390_PLT32 = 8,
			S390_COPY = 9,
			S390_GLOB_DAT = 10,
			S390_JMP_SLOT = 11,
			S390_RELATIVE = 12,
			S390_GOTOFF = 13,
			S390_GOTPC = 14,
			S390_GOT16 = 15,
			S390_PC16 = 16,
			S390_PC16DBL = 17,
			S390_PLT16DBL = 18
		};

		/// @brief RISC-V definitions. See here for more info:
		/// https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#elf-object-file
		enum {
			RISCV_NONE = 0,
			RISCV_32 = 1,
			RISCV_64 = 2,
			RISCV_RELATIVE = 3,
			RISCV_COPY = 4,
			RISCV_JUMP_SLOT = 5,
			RISCV_TLS_DTPMOD32 = 6,
			RISCV_TLS_DTPMOD64 = 7,
			RISCV_TLS_DTPREL32 = 8,
			RISCV_TLS_DTPREL64 = 9,
			RISCV_TLS_TPREL32 = 10,
			RISCV_TLS_TPREL64 = 11,
			RISCV_BRANCH = 16,
			RISCV_JAL = 17,
			RISCV_CALL = 18,
			RISCV_CALL_PLT = 19,
			RISCV_GOT_HI20 = 20,
			RISCV_TLS_GOT_HI20 = 21,
			RISCV_TLS_GD_HI20 = 22,
			RISCV_PCREL_HI20 = 23,
			RISCV_PCREL_LO12_I = 24,
			RISCV_PCREL_LO12_S = 25,
			RISCV_HI20 = 26,
			RISCV_LO12_I = 27,
			RISCV_LO12_S = 28,
			RISCV_TPREL_HI20 = 29,
			RISCV_TPREL_LO12_I = 30,
			RISCV_TPREL_LO12_S = 31,
			RISCV_TPREL_ADD = 32,
			RISCV_ADD8 = 33,
			RISCV_ADD16 = 34,
			RISCV_ADD32 = 35,
			RISCV_ADD64 = 36,
			RISCV_SUB8 = 37,
			RISCV_SUB16 = 38,
			RISCV_SUB32 = 39,
			RISCV_SUB64 = 40,
			RISCV_GNU_VTINHERIT = 41,
			RISCV_GNU_VTENTRY = 42,
			RISCV_ALIGN = 43,
			RISCV_RVC_BRANCH = 44,
			RISCV_RVC_JUMP = 45,
			RISCV_RVC_LUI = 46,
			RISCV_GPREL_I = 47,
			RISCV_GPREL_S = 48,
			RISCV_TPREL_I = 49,
			RISCV_TPREL_S = 50,
			RISCV_RELAX = 51,
			RISCV_SUB6 = 52,
			RISCV_SET6 = 53,
			RISCV_SET8 = 54,
			RISCV_SET16 = 55,
			RISCV_SET32 = 56,
			RISCV_32_PCREL = 57,
			RISCV_IRELATIVE = 58,
			RISCV_RESERVED = 59
		};
	}
}

namespace elf32 {
	struct header : public elf::header<uint32_t> {

	} PACKED;

	struct phdr {
		uint32_t type;
		uint32_t offset;
		uint32_t v_addr;
		uint32_t p_addr;
		uint32_t file_size;
		uint32_t mem_size;
		uint32_t flags;
		uint32_t align;
	} PACKED;

	struct sym {
		uint32_t name;
		uint32_t value;
		uint32_t size;
		uint8_t info;
		uint8_t other;
		uint16_t section_idx;

		constexpr uint8_t get_bind(void) const {
			return static_cast<uint8_t>(info >> 4);
		}

		constexpr uint8_t get_type(void) const {
			return static_cast<uint8_t>(info & 0xf);
		}
	} PACKED;

	struct section_header : public elf::section_header<uint32_t> {

	} PACKED;

	struct rel : public elf::rel<uint32_t> {
		
	} PACKED;

	struct rela : public elf::rela<uint32_t> {

	} PACKED;
}

namespace elf64 {
	struct header : public elf::header<uint64_t> {

	} PACKED;

	struct phdr {
		uint32_t type;
		uint32_t flags;
		uint64_t offset;
		uint64_t v_addr;
		uint64_t p_addr;
		uint64_t file_size;
		uint64_t mem_size;
		uint64_t align;
	} PACKED;

	struct sym {
		uint32_t name;
		uint8_t info;
		uint8_t other;
		uint16_t section_idx;
		uint64_t value;
		uint64_t size;

		inline uint8_t get_bind(void) const {
			return static_cast<uint8_t>(info >> 4);
		}

		inline uint8_t get_type(void) const {
			return static_cast<uint8_t>(info & 0xf);
		}
	} PACKED;

	struct section_header : public elf::section_header<uint64_t> {

	} PACKED;

	struct rel : public elf::rel<uint64_t> {

	} PACKED;

	struct rela : public elf::rela<uint64_t> {

	} PACKED;

	struct reader {
		elf64::header *hdr;
		size_t size;
		void *base;
		timeshare::job *job;
		void *plt_base = nullptr; // Real base of the PLT
		void *got_base = nullptr; // Real base of the GOT
		elf64::section_header *str_shdr = nullptr; // String section
		elf64::section_header *dynstr_shdr = nullptr; // Dynamic string section
	} PACKED;

	int check_valid(elf64::reader& rdr);
	int load(timeshare::job *job, void *buffer, size_t n, void **entry);
}

#endif
