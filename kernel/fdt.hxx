#ifndef FDT_HXX
#define FDT_HXX

#include <types.hxx>
#include <byteswap.hxx>

namespace fdt {
#define FDT_MAGIC 0xD00DFEED
	namespace token_type {
		enum {
			BEGIN_NODE = 0x01,
			END_NODE = 0x02,
			PROP = 0x03,
			NOP = 0x04,
			END = 0x09,
		};
	}

	struct header {
		uint32_t magic;
		uint32_t total_size;
		uint32_t off_dt_struct;
		uint32_t off_dt_strings;
		uint32_t off_mem_rsvmap;
		uint32_t version;
		uint32_t last_comp_version;
		uint32_t boot_cpuid_phys;
		uint32_t size_dt_strings;
		uint32_t size_dt_struct;
	};

	struct reserve_entry {
		uint64_t address;
		uint64_t size;
	};

	struct property_entry {
		uint32_t len;
		uint32_t off_name;
		uint8_t data[];
	};

	extern void *fdt_address;

	static inline void *get_system_fdt(void) noexcept
	{
		return fdt_address;
	}

	template<typename T>
	constexpr const T *padding(const void *fdt)
	{
		const uintptr_t offs = reinterpret_cast<uintptr_t>(fdt);
		const uintptr_t rem = offs % 4;
		fdt = rem ? reinterpret_cast<const void *>(offs + 4 - rem) : fdt;
		return reinterpret_cast<const T *>(fdt);
	}

	constexpr uint32_t get_token(const void *off)
	{
		return be_to_cpu32(*reinterpret_cast<const uint32_t *>(off));
	}
	
	const void *get_node(const fdt::header *hdr, const char *key);
	void *get_node_address(const void *node);
	const fdt::property_entry *get_prop(const fdt::header *hdr, const void *off, const char *key);
	const void *get_prop_value(const fdt::property_entry *prop);
	uint32_t get_prop_size(const fdt::property_entry *prop);
}

#endif
