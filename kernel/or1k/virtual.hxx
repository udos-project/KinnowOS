#ifndef VIRTUAL_STORAGE_HXX
#define VIRTUAL_STORAGE_HXX

#include <types.hxx>
#include <mutex.hxx>
#include <or1k/asm.hxx>

namespace virtual_storage {
	constexpr auto page_align = 4096; // Page alignment

	struct page_entry {
		using entry_size = uint32_t;
		entry_size entry = 0;

		constexpr page_entry() = default;
		~page_entry() = default;
		page_entry& operator=(page_entry&) = delete;
		const page_entry& operator=(const page_entry&) = delete;

		constexpr bool cache_coherency() { return this->entry & 0x01; }
		constexpr void cache_coherency(const bool val) { this->entry |= val ? 0x01 : 0x00; }
		constexpr bool cache_inhibit() { return this->entry & 0x02; }
		constexpr void cache_inhibit(const bool val) { this->entry |= val ? 0x02 : 0x00; }
		constexpr bool writeback_cache() { return this->entry & 0x04; }
		constexpr void writeback_cache(const bool val) { this->entry |= val ? 0x04 : 0x00; }
		// Weakly ordered memory
		constexpr bool weak_order_mem() { return this->entry & 0x08; }
		constexpr void weak_order_mem(const bool val) { this->entry |= val ? 0x08 : 0x00; }
		constexpr bool accessed() { return this->entry & 0x10; }
		constexpr void accessed(const bool val) { this->entry |= val ? 0x10 : 0x00; }
		// Dirty is set if page was modified
		constexpr bool dirty() { return this->entry & 0x20; }
		constexpr void dirty(const bool val) { this->entry |= val ? 0x20 : 0x00; }
		// Protection bits according to the xMMUCR
		constexpr bool protection() { return this->entry & 0x40; }
		constexpr void protection(const int val) { this->entry |= val << 6; }
		// Whetever thios page entry is the last in the linked list
		constexpr bool last() { return this->entry & 0x200; }
		constexpr void last(const bool val) { this->entry |= val ? 0x200 : 0x00; }
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
