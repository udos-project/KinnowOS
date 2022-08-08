#ifndef VIRTUAL_STORAGE_HXX
#define VIRTUAL_STORAGE_HXX

#include <types.hxx>
#include <s390/asm.hxx>
#include <mutex.hxx>

#if MACHINE >= M_ZARCH
/* Pages */
#   define S390_PTE_ORIGIN(x) ((x) << S390_BIT_MULTI(64, 0, 52))
#   define S390_PTE_INVALID ((1) << S390_BIT(64, 53))
#   define S390_PTE_RDONLY ((1) << S390_BIT(64, 54))
/* Segments */
/* Page table origin */
#   define S390_STE_PT_ORIGIN(x) ((x) << S390_BIT_MULTI(64, 0, 52))
#   define S390_STE_INVALID ((1) << S390_BIT(64, 58))
#   define S390_STE_COMMON ((1) << S390_BIT(64, 59))
/* Length of the page entry table (in multiples of 16 entries/64 bytes) */
#   define S390_STE_PT_LENGTH(x) ((x & 0x03) << S390_BIT_MULTI(64, 62, 2))
#else
/* Pages */
#   define S390_PTE_ORIGIN(x) ((x) << S390_BIT_MULTI(32, 0, 20))
#   define S390_PTE_ORIGIN_PREFIX(x) ((x) << S390_BIT_MULTI(32, 24, 8))
#   define S390_PTE_INVALID ((1) << S390_BIT(32, 22))
#   define S390_PTE_RDONLY ((1) << S390_BIT(32, 23))
/* Segments */
/* Page table origin */
#   define S390_STE_PT_ORIGIN(x) ((x) << S390_BIT_MULTI(32, 1, 24))
#   define S390_STE_INVALID ((1) << S390_BIT(32, 27))
#   define S390_STE_COMMON ((1) << S390_BIT(32, 28))
/* Length of the page entry table (in multiples of 16 entries/64 bytes) */
#   define S390_STE_PT_LENGTH(x) ((x) << S390_BIT_MULTI(32, 28, 4))
#endif

namespace virtual_storage {
	constexpr auto page_align = 4096; // Page alignment
	constexpr auto max_pages = 256; // Number of pages per segment entry
	constexpr auto max_segments = 2048; // Number of segments in a block
	constexpr auto max_regtabs1 = 2048;
	constexpr auto max_regtabs2 = 2048;
	constexpr auto max_regtabs3 = 2048;

	constexpr auto regtab3_shift = 53;
	constexpr auto regtab2_shift = 42;
	constexpr auto regtab1_shift = 31;
	constexpr auto segment_shift = 20;
	constexpr auto page_shift = 12;

	struct segment_entry {
#if (MACHINE > M_S370 && MACHINE <= M_S390)
		using entry_size = uint32_t;
#elif MACHINE >= M_ZARCH
		using entry_size = uint64_t;
#else
		using entry_size = uint16_t;
#endif
		entry_size entry = 0;
		static constexpr entry_size origin_bitmask = ~(0x7FFU);
		static constexpr entry_size invalid_bitmask = 1 << S390_BIT(64, 58);
		static constexpr entry_size protection_bitmask = 1 << S390_BIT(64, 59);

		constexpr segment_entry() = default;
		~segment_entry() = default;
		segment_entry& operator=(segment_entry&) = delete;
		const segment_entry& operator=(const segment_entry&) = delete;

		// Page table origin
		constexpr void origin(void *org) { this->entry |= (uintptr_t)org & origin_bitmask; }
		constexpr void *origin() const { return (void *)(this->entry & origin_bitmask); }
		// Invalid bit
		constexpr void invalid(bool val) { this->entry |= val ? invalid_bitmask : 0; }
		constexpr bool invalid() const { return (this->entry & invalid_bitmask) != 0; }
		// Protected bit (disallows stores)
		constexpr void protection(bool val) { this->entry |= val ? protection_bitmask : 0; }
		constexpr bool protection() const { return (this->entry & protection_bitmask) != 0; }
	};

	struct page_entry {
#if (MACHINE > M_S370 && MACHINE <= M_S390)
		using entry_size = uint32_t;
#elif MACHINE >= M_ZARCH
		using entry_size = uint64_t;
#else
		using entry_size = uint16_t;
#endif
		entry_size entry = 0;
		static constexpr entry_size origin_bitmask = ~(0xFFFU);
		static constexpr entry_size invalid_bitmask = 1 << S390_BIT(64, 53);
		static constexpr entry_size protection_bitmask = 1 << S390_BIT(64, 54);

		constexpr page_entry() = default;
		~page_entry() = default;
		page_entry& operator=(page_entry&) = delete;
		const page_entry& operator=(const page_entry&) = delete;

		// Page origin
		constexpr void origin(void *org) { this->entry |= (uintptr_t)org & origin_bitmask; }
		constexpr void *origin() const { return (void *)(this->entry & origin_bitmask); }
		// Invalid bit
		constexpr void invalid(bool val) { this->entry |= val ? invalid_bitmask : 0; }
		constexpr bool invalid() const { return (this->entry & invalid_bitmask) != 0; }
		// Protected bit (disallows stores)
		constexpr void protection(bool val) { this->entry |= val ? protection_bitmask : 0; }
		constexpr bool protection() const { return (this->entry & protection_bitmask) != 0; }
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
#if MACHINE >= M_ZARCH
			asm volatile("LCTLG 1,1,%0\r\n" : : "m"(this->cr1) : );
#else
			asm volatile("LCTL 1,1,%0\r\n" : : "m"(this->cr1) : );
#endif
		}

		inline void set_secondary() const
		{
#if MACHINE >= M_ZARCH
			asm volatile("LCTLG 7,7,%0\r\n" : : "m"(this->cr1) : );
#else
			asm volatile("LCTL 7,7,%0\r\n" : : "m"(this->cr1) : );
#endif
		}

		inline void set_home() const
		{
#if MACHINE >= M_ZARCH
			asm volatile("LCTLG 13,13,%0\r\n" : : "m"(this->cr1) : );
#else
			asm volatile("LCTL 13,13,%0\r\n" : : "m"(this->cr1) : );
#endif
		}

		inline void flush_tlb() const
		{
			asm volatile("PTLB\r\n"); // And don't forget to invalidate the TLB
		}

		arch_dep::register_t cr1 = 0;
		virtual_storage::segment_entry *segtab = nullptr;
		base::mutex lock;
	};

	int init();
}

#endif
