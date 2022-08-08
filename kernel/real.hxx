#ifndef REAL_STORAGE_HXX
#define REAL_STORAGE_HXX

#include <types.hxx>
#include <arch/asm.hxx>
#include <mutex.hxx>

namespace real_storage {
#define MAX_PMM_REGIONS 8 // Regions possible
#if defined DEBUG
#	define PMM_TEST_NPTR 24 // Number of pointers used for alloation
#	define PMM_TEST_PTRSZ 32 // Size of each allocation unit
#endif

	struct block;
	struct region;

	struct block {
		block& operator=(block&) = delete;
		const block& operator=(const block&) = delete;

		enum flag {
			NOT_PRESENT = 0x00,
			FREE = 0x01,
			USED = 0x02,
		} flags = real_storage::block::NOT_PRESENT;
		static real_storage::block *create(real_storage::region *region, size_t size, real_storage::block::flag flags, real_storage::block *next);
		static real_storage::block *merge(real_storage::block *prev, real_storage::block *block, real_storage::block::flag flags);

		size_t size;
		real_storage::block *next;
	};

	struct region {
		region& operator=(region&) = delete;
		const region& operator=(const region&) = delete;

		static real_storage::region *create(void *base, size_t size);
		static void destroy(real_storage::region *region);

		enum flag {
			NOT_PRESENT = 0x00,
			PUBLIC = 0x01,
		} flags = real_storage::region::NOT_PRESENT;
		void *base = nullptr;
		size_t size = 0;
		real_storage::block *head = nullptr;
		base::mutex lock;
	};

	size_t get_memsize();
	int init();

#if defined DEBUG
	void check_heap();
#endif
	ATTRIB_MALLOC void *alloc(size_t size, size_t align);
	void  free(void *ptr);
	void *realloc(void *ptr, size_t size, size_t align);

	struct stats {
		size_t free_size = 0;
		size_t used_size = 0;
		size_t n_regions = 0;
	};
	int get_stats(real_storage::stats *stats);

	struct table {
		real_storage::region regions[MAX_PMM_REGIONS];
	};
}

#endif
