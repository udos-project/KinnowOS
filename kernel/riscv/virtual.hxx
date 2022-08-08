#ifndef VIRTUAL_STORAGE_HXX
#define VIRTUAL_STORAGE_HXX

#include <types.hxx>
#include <mutex.hxx>
#include <arch/asm.hxx>

namespace virtual_storage {
	constexpr auto page_align = 4096; // Page alignment

	struct page_entry {
		using entry_size = uint32_t;
		entry_size entry = 0;

		constexpr page_entry() = default;
		~page_entry() = default;
		page_entry& operator=(page_entry&) = delete;
		const page_entry& operator=(const page_entry&) = delete;
	};

	struct address_space {
		constexpr address_space() = default;
		~address_space() = default;
		address_space& operator=(address_space&) = delete;
		const address_space& operator=(const address_space&) = delete;

		static virtual_storage::address_space *create();
		static void destroy(virtual_storage::address_space* aspace);
		int map_page(void *virtaddr, void *physaddr, int flags);
		int map_range(void *virtaddr, void *physaddr, int flags, size_t size);
		void *virtual_to_real(void *virtaddr);
		void *phys2virt(void *physaddr);
		
		inline void set_primary() const
		{

		}

		inline void set_secondary() const
		{

		}

		inline void set_home() const
		{

		}

		inline void flush_tlb() const
		{

		}

		arch_dep::register_t cr1 = 0;
		virtual_storage::page_entry *segtab = nullptr;
		base::mutex lock;
	};

	int init();
}

#endif
