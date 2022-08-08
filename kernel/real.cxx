#include <types.hxx>
#include <real.hxx>
#include <storage.hxx>

extern uint8_t heap_start[];
constinit static storage::global_wrapper<real_storage::table> g_real_storage_regions;

/// @brief (WIP) Obtains the total real storage size
/// @todo Maybe create a map (i.e non-contigous real storage) instead of assuming contiguity
/// @return size_t The total real storage size
size_t real_storage::get_memsize()
{
	// We are going to read in pairs of 1MiB and when we hit the memory limit TPROT will
	// simply tell us
	uintptr_t addr = reinterpret_cast<uintptr_t>(&heap_start[0]) /*0x0 + PSA_SIZE*/;
#ifdef TARGET_S390
	while(1) {
		// TPROT returns a proper cc code
		int cc = -1;
		asm volatile("TPROT 0(%1),0\r\nIPM %0\r\n" : "+d"(cc) : "a"(addr) : "cc");
		cc = cc >> 28;
		debug_printf("cc=%i,p=%p", cc, addr);
		if(cc != 0) break;
		// Go to next MiB
		addr += 1048576;
	}
#endif
	return (size_t)addr;
}

/// @brief Initialize the physical memory manager
/// @return Result code
int real_storage::init()
{
	size_t mem_size = 0;
#if defined DEBUG
	void *ptrs[PMM_TEST_NPTR];
#endif

	/*mem_size = real_storage::get_memsize();*/
	debug_printf("HeapStart=%p,Size=%u", &heap_start[0], mem_size);
	auto *region = real_storage::region::create(&heap_start, (32758 * 100));
	if(region == nullptr)
		kpanic("Can't create region");
	debug_assert(region != nullptr);
#if defined DEBUG
	debug_printf("RealAlloc Unit Testing - stacked allocation");
	for(size_t i = 0; i < PMM_TEST_NPTR; i++) { // Allocate memory and set it with data
		ptrs[i] = real_storage::alloc(PMM_TEST_PTRSZ, (i * i) + 2);
		for(size_t j = 0; j < PMM_TEST_PTRSZ; j++)
			((volatile uint8_t *)ptrs[i])[j] = static_cast<uint8_t>(j * i);
	}
	for(size_t i = 0; i < PMM_TEST_NPTR; i++) { // Verify data integrity
		volatile const auto *u8_ptr = (volatile const uint8_t *)ptrs[i];
		for(size_t j = 0; j < PMM_TEST_PTRSZ; j++)
			debug_assert(u8_ptr[j] == static_cast<uint8_t>(j * i));
	}

	debug_printf("Realloc, same size");
	for(size_t i = 0; i < PMM_TEST_NPTR; i++) // Reallocate with same size
		ptrs[i] = real_storage::realloc(ptrs[i], PMM_TEST_PTRSZ, (i * i) + 2);
	for(size_t i = 0; i < PMM_TEST_NPTR; i++) { // Verify data integrity
		volatile const uint8_t *u8_ptr = (volatile const uint8_t *)ptrs[i];
		for(size_t j = 0; j < PMM_TEST_PTRSZ; j++)
			debug_assert(u8_ptr[j] == static_cast<uint8_t>(j * i));
	}

	debug_printf("Realloc, half size");
	/* Reallocate with half the size */
	for(size_t i = 0; i < PMM_TEST_NPTR; i++)
		ptrs[i] = real_storage::realloc(ptrs[i], PMM_TEST_PTRSZ / 2, (i * i) + 2);
	/* Verify data integrity */
	for(size_t i = 0; i < PMM_TEST_NPTR; i++) {
		volatile const uint8_t *u8_ptr = (volatile const uint8_t *)ptrs[i];
		for(size_t j = 0; j < PMM_TEST_PTRSZ / 2; j++)
			debug_assert(u8_ptr[j] == static_cast<uint8_t>(j * i));
	}

	for(size_t i = 0; i < PMM_TEST_NPTR; i++) // Free the allocated ptrs for the testing
		real_storage::free(ptrs[i]);
	debug_printf("RealAlloc: reliable");
#endif
	return 0;
}


#if defined DEBUG
void real_storage::check_heap()
{
	for(size_t i = 0; i < MAX_PMM_REGIONS; i++) {
		const auto *region = &g_real_storage_regions->regions[i];
		const auto *block = region->head;
		size_t size = 0, free = 0, used = 0;
		// Used for printing sequences of blocks instead of printing every one of them
		// individually - thus not spamming the entire screen with repeated info
		size_t last_cnt = 1, last_size = 0;
		auto last_flags = real_storage::block::NOT_PRESENT;

		if(region->head == nullptr || region->flags != real_storage::region::PUBLIC) continue;

		/* Present regions have to have correct values */
		debug_assert(region->head != nullptr);
		size_t n_blocks = region->head->size / sizeof(real_storage::block);

		debug_printf("Check region %u (%u blocks)", i, n_blocks);
		auto current_ptr = reinterpret_cast<uintptr_t>(region->base);
		while(block != nullptr) {
			/* Block can't be bigger than the region */
			debug_assert(block->size <= region->size - (size_t)(current_ptr - (uintptr_t)region->base));
			/* Blocks can't be outside the region */
			debug_assert((uintptr_t)block >= (uintptr_t)region->base && (uintptr_t)block < (uintptr_t)region->base + region->size);

			size += block->size;
			if(block->flags == real_storage::block::FREE) {
				free += block->size;
			} else if(block->flags == real_storage::block::USED) {
				used += block->size;
			}

			if(last_flags == block->flags) {
				last_size += block->size;
				last_cnt++;
			} else {
				last_flags = block->flags;
				last_size = block->size;
				last_cnt = 1;
			}

#if defined DEBUG_PMM
			debug_printf("%s (%zuB @ %p) %p -> %p", (block->flags == real_storage::block::FREE) ? "Free" : "Used", block->size, (void *)current_ptr, block, block->next);
#endif
			current_ptr += block->size;
			block = block->next;
			if(block != nullptr && block == block->next)
				kpanic("Pointer to self");
		}
		
		if(size != region->size)
			kpanic("Size %u, however it should be %u", size, region->size);
		debug_assert(size == free + used);
		debug_printf("Storage: %u free, %u used", free, used);
	}
}
#endif

/// @brief Create a new physical memory manager region for the manager to administer
/// correctly - take note regions are to represent areas of the memory which may be
/// fragmented due to the uncertain continuity of the storage.
/// @param base Starting real address of the region
/// @param size Total size to manage (the function will automatically account for heap)
/// @return real_storage::region* The region created on the static storage
real_storage::region *real_storage::region::create(void *base, size_t size)
{
	// Memory is uninitialized and unknown by default, so it can cause problems if we don't clear it
	// but note we only have to clear the heap itself and nothing else
	for(size_t i = 0; i < MAX_PMM_REGIONS; i++) {
		auto *region = &g_real_storage_regions->regions[i];

		// Region must not be already taken
		if(region->flags != real_storage::region::NOT_PRESENT) continue;

		// The heap is always created at base for each region
		region->base = base;
		region->size = size;
		region->head = reinterpret_cast<real_storage::block *>(base);
		region->flags = real_storage::region::PUBLIC;

		size_t n_blocks = (region->size / 8192) + 1;
		for(size_t j = 0; j < n_blocks; j++)
			region->head[j].flags = real_storage::block::NOT_PRESENT;
		region->head[0].size = n_blocks * sizeof(real_storage::block);
		region->head[0].flags = real_storage::block::USED;
		region->head[0].next = &region->head[1];

		// Block is created next to the head
		region->head[1].size = region->size - region->head[0].size;
		region->head[1].flags = real_storage::block::FREE;
		region->head[1].next = nullptr;

		debug_assert(region->head[0].size + region->head[1].size == region->size);
		return region;
	}

	kpanic("TODO: Do a method for getting more regions");
	return nullptr;
}

/// @brief Deletes a region
/// @param region The region to delete
void real_storage::region::destroy(real_storage::region *region)
{
	region->flags = real_storage::region::NOT_PRESENT;
}

/// @brief Create a new block on a region, expands the heap if nescesary
/// @param region Region the block would be in
/// @param size The size the block represents
/// @param flags Flags
/// @param next Next-linked-list block
/// @return real_storage::block* The new allocated block on the heap, nullptr means failure to allocate
real_storage::block *real_storage::block::create(real_storage::region *region, size_t size, real_storage::block::flag flags, real_storage::block *next)
{
	auto *heap = reinterpret_cast<real_storage::block *>(region->base);
	real_storage::block *block;
	
	size_t n_blocks = region->head->size / sizeof(real_storage::block);
	for(size_t i = 0; i < n_blocks; i++) {
		block = &heap[i];
		if(block->flags != real_storage::block::NOT_PRESENT)
			continue;
		goto set_block;
	}

	if(heap[1].flags != real_storage::block::FREE || heap[1].size < sizeof(real_storage::block) * 32)
		return nullptr;

	heap[0].size += sizeof(real_storage::block) * 32;
	heap[1].size -= sizeof(real_storage::block) * 32;
	block = &heap[n_blocks];
	
	// Set all to zero when expanding to prevent spurious blocks
	// n_blocks = region->head->size / sizeof(real_storage::block);
	storage::fill(&heap[n_blocks], 0, sizeof(real_storage::block) * 32);
set_block:
	block->flags = flags;
	block->size = size;
	block->next = next;
	return block;
}

/// @brief Merges various blocks into one by coalscencing them
/// @todo We should probably make block be a pointer to a pointer and set the resulting block
/// or return it via the functor
/// @param prev The block previous to this one, gets invalidated after operation
/// @param block The block to be merged
/// @param flags Flags that needs to match to merge
/// @return real_storage::block * Returned block
real_storage::block *real_storage::block::merge(real_storage::block *prev, real_storage::block *block, real_storage::block::flag flags)
{
	debug_assert(block != nullptr);

	// Coalescence behind
	if(prev != nullptr && prev->flags == flags) {
		block->flags = real_storage::block::NOT_PRESENT;
		prev->size += block->size;
		prev->next = block->next;
		block = prev;
		debug_assert(block != nullptr);
		// prev is now invalidated
	}

	// Coalescence after
	while(block != nullptr && block->next != nullptr && block->next->flags == flags) {
		block->next->flags = real_storage::block::NOT_PRESENT;
		block->size += block->next->size;
		block->next = block->next->next;
		block = block->next; // Continue to next block (probably a multi-merge?)
	}
	return block;
}

/// @brief Allocate a piece of storage/memory
/// @param size The size to allocate. It's the caller's responsability to assert this is a non-zero value
/// @param align Alignment required for allocation, a 0 means "up to the manager/no alignment required"
/// @return void* The pointer to the storage area
void *real_storage::alloc(size_t size, size_t align)
{
	debug_assert(size != 0);
	debug_printf("alloc size=%u,align=%u", size, align);
	for(size_t i = 0; i < MAX_PMM_REGIONS; i++) {
		auto *region = &g_real_storage_regions->regions[i];
		const base::scoped_mutex lock1(region->lock);
		if(region->flags != real_storage::region::PUBLIC)
			continue;
		
		// Some allocations can be extremely big - in such case it isn't worth the effort to iterate
		// everything when the region itself is known to not be able to hold such storage
		if(size > region->size) continue;

		auto *block = region->head;
		uintptr_t current_ptr = reinterpret_cast<uintptr_t>(region->base);
		while(block != nullptr) {
			size_t left_size, right_size;
			// Block can't be bigger than the region
			debug_assert(block->size <= region->size - (size_t)(current_ptr - (uintptr_t)region->base));
			// Blocks can't be outside the region
			debug_assert((uintptr_t)block >= (uintptr_t)region->base && (uintptr_t)block < (uintptr_t)region->base + region->size);

			// Check that the block is not used
			if(block->flags == real_storage::block::USED)
				goto next_block;

			// Check that the block is big enough to hold our aligned object
			// (if there is any align of course)
			if((align && (uintptr_t)block->size < size + (current_ptr % align)) || (block->size < size))
				goto next_block;

			// Create a remaining "free" block
			if(align) {
				uintptr_t left_size_ptr = (current_ptr + block->size - size) - ((current_ptr + block->size - size) % align) - current_ptr;
				left_size = (size_t)left_size_ptr;
				debug_assert((uintptr_t)left_size == left_size_ptr); // Make sure no data was lost
			}
			// No alignment - so only size is took in account
			else {
				debug_assert(block->size >= size);
				left_size = block->size - size;
			}

			// Create a block on the left (previous) to this block
			if(left_size) {
				size_t next_size = block->size - left_size;
				block->size = left_size;
				block->flags = real_storage::block::FREE;
				block->next = real_storage::block::create(region, next_size, real_storage::block::USED, block->next);
				debug_assert(block != block->next && block->next != block->next->next);
				block = block->next;
				current_ptr += (uintptr_t)left_size; // Update pointer
			}

			// It must be aligned by now, otherwise the algorithm is faulty.
			if(align)
				debug_assert(current_ptr % align == 0);

			// Create a block on the right (next) to this block if there are any
			// remaining bytes.
			right_size = block->size - size;
			if(right_size) {
				block->next = real_storage::block::create(region, right_size, real_storage::block::FREE, block->next);
				block->size -= right_size;
			}

			block->flags = real_storage::block::USED;
			debug_printf("alloc %zuB (align %u) @ %p,lsize=%u,rsize=%u", size, align, (void *)current_ptr, left_size, right_size);
#if defined DEBUG
			real_storage::check_heap();
#endif
			return (void *)current_ptr;
		next_block:
			current_ptr += block->size;
			block = block->next;
		}
	}

	debug_printf("Can't alloc %zuB (align %u)", size, align);
#if defined DEBUG
	real_storage::check_heap();
#endif
	return nullptr;
}

/// @brief Free a block of real storage
/// @param ptr The pointer to free up; Note that it is the caller's responsability to assert that ptr != nullptr 
void real_storage::free(void *ptr)
{
	debug_assert(ptr != nullptr);
	debug_printf("free %p", ptr);
	for(size_t i = 0; i < MAX_PMM_REGIONS; i++) {
		auto *region = &g_real_storage_regions->regions[i];
		const base::scoped_mutex lock1(region->lock);
		if(region->flags != real_storage::region::PUBLIC)
			continue;

		/* Pointer must be also inside region */
		if((uintptr_t)ptr < (uintptr_t)region->base || (uintptr_t)ptr > (uintptr_t)region->base + region->size)
			continue;

		auto *block = region->head;
		real_storage::block *prev = nullptr;
		uintptr_t current_ptr = (uintptr_t)region->base;
		while(block != nullptr) {
			// Block can't be bigger than the region
			debug_assert(block->size <= region->size - (size_t)(current_ptr - (uintptr_t)region->base));
			// Blocks can't be outside the region
			debug_assert((uintptr_t)block >= (uintptr_t)region->base && (uintptr_t)block < (uintptr_t)region->base + region->size);

			if((uintptr_t)ptr >= current_ptr && (uintptr_t)ptr < current_ptr + block->size) {
				// Free the requested block. Check for double frees
				debug_assertm(block->flags != real_storage::block::FREE, "Double free");
				// Do not free the heap/genesis blocks
				debug_assert(block != region->head);

				/// @todo Find out whats wrong with our PMM
				block->flags = real_storage::block::FREE;
				block = real_storage::block::merge(prev, block, real_storage::block::FREE);
#if defined DEBUG
				real_storage::check_heap();
#endif
				return;
			}
			
			if(current_ptr > (uintptr_t)ptr) {
				debug_printf("Block %p not found", ptr);
				return;
			}

			current_ptr += block->size;
			prev = block;
			block = block->next;
		}
	}

#if defined DEBUG
	real_storage::check_heap();
#endif
}

/// @brief Reallocates a block of the real storage
/// @param ptr Pointer to reallocate, if nullptr a new allocation is done
/// @param size Size to reallocate
/// @param align Alignment required for reallocation
/// @return void* New reallocated pointer, nullptr indicates that the reallocation was failed
void *real_storage::realloc(void *ptr, size_t size, size_t align)
{
	debug_printf("realloc %p,size=%u,align=%u", ptr, size, align);
	if(size == 0) return nullptr; // Nothing to allocate
	if(ptr == nullptr) // Behave like malloc
		return real_storage::alloc(size, align);
	
	for(size_t i = 0; i < MAX_PMM_REGIONS; i++) {
		auto *region = &g_real_storage_regions->regions[i];
		base::scoped_mutex lock1(region->lock);
		if(region->flags != real_storage::region::PUBLIC)
			continue;

		/* Pointer must be also inside region */
		if((uintptr_t)ptr < (uintptr_t)region->base || (uintptr_t)ptr > (uintptr_t)region->base + region->size)
			continue;

		auto *block = region->head;
		real_storage::block *prev = nullptr;
		uintptr_t current_ptr = (uintptr_t)region->base;
		while(block != nullptr) {
			/// @todo There are ways to optimize aligned block reallocations, for now
			/// we entirely reallocate the entire thing

			// Block can't be bigger than the region
			debug_assert(block->size <= region->size - (size_t)(current_ptr - (uintptr_t)region->base));
			// Blocks can't be outside the region
			debug_assert((uintptr_t)block >= (uintptr_t)region->base && (uintptr_t)block < (uintptr_t)region->base + region->size);
			// Same size means no need to do anything
			if(size == block->size) {
#if defined DEBUG_PMM
				debug_printf("Same size");
#endif
				return reinterpret_cast<void *>(current_ptr);
			}
			/* Expand the reallocated block */
			else if(size > block->size && (uintptr_t)ptr >= current_ptr && (uintptr_t)ptr < current_ptr + block->size) {
				size_t rem_size = size - block->size;
				debug_assertm(block->flags == real_storage::block::USED, "Reallocating free block");

				/* Check that this block has a next block, that is free, and that has the remainder required size */
				if(align == 0 && block->next != nullptr && block->next->flags == real_storage::block::FREE && block->next->size >= rem_size) {
#if defined DEBUG_PMM
					debug_printf("Take from next");
#endif
					debug_assert(block->next->size != 0);

					block->next->size -= rem_size;
					block->size += rem_size;
					if(block->next->size == 0) {
						block->next->flags = real_storage::block::NOT_PRESENT;
						block->next = block->next->next;
					}
					return (void *)current_ptr;
				}
				// Check the previous block
				/// @todo Have a previous-previous block :)
				else if(align == 0 && prev != nullptr && prev->flags == real_storage::block::FREE && prev->size >= rem_size) {
					if(prev->size > rem_size) {
						debug_printf("Take from previous");
						/// @todo Please don't
						// Move to a lower address
						storage::move((void *)(current_ptr - rem_size), (void *)current_ptr, block->size);
						// Apply changes
						prev->size -= rem_size;
						block->size += rem_size;
						current_ptr -= rem_size;
						return (void *)current_ptr;
					} else if(prev->size == rem_size) {
						/// @todo Merge both blocks, that requires prev->prev
						debug_assertm(0, "Can't handle the edge case of psize==size yet!");
						return nullptr;
					}
				}
				// Otherwise we have to completely reallocate from 0
				else {
					// Allocate entirely new block
					void *old_ptr = ptr;
					lock1.~scoped_mutex();

					// Mutex is locked again
					ptr = real_storage::alloc(size, align);
					if(ptr == nullptr) return nullptr;
					storage::move(ptr, old_ptr, size);
					real_storage::free(old_ptr);
					return ptr;
				}
				return nullptr;
			}
			// Shrink the block
			else {
				// Allocate entirely new block
				void *old_ptr = ptr;
				lock1.~scoped_mutex();

				// Mutex is locked again
				ptr = real_storage::alloc(size, align);
				if(ptr == nullptr) return nullptr;
				storage::move(ptr, old_ptr, size);
				real_storage::free(old_ptr);
				return ptr;
			}
			
			if(current_ptr > (uintptr_t)ptr) {
				debug_printf("%p not found", ptr);
				return nullptr;
			}

			current_ptr += block->size;
			prev = block;
			block = block->next;
		}
	}
	return nullptr;
}

/// @brief Obtain the usage and other statistics about the real storage allocator
/// @param stats The structure to write the stats into
/// @return int Return code, negative is error
int real_storage::get_stats(real_storage::stats *stats)
{
	// Zero-out the stats
	*stats = {};
	for(size_t i = 0; i < MAX_PMM_REGIONS; i++) {
		auto *region = &g_real_storage_regions->regions[i];
		const base::scoped_mutex lock1(region->lock);
		if(region->flags != real_storage::region::PUBLIC) continue;

		const auto *block = region->head;
		auto current_ptr = reinterpret_cast<uintptr_t>(region->base);
		stats->n_regions++;
		while(block != nullptr) {
			if(block->flags == real_storage::block::FREE) {
				stats->free_size += block->size;
			} else if(block->flags == real_storage::block::USED) {
				stats->used_size += block->size;
			}
			
			current_ptr += block->size;
			block = block->next;
		}
	}
	return 0;
}
