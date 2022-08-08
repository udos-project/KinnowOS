#include <s390/virtual.hxx>
#include <storage.hxx>
#include <printf.hxx>

int virtual_storage::init()
{
	return 0;
}

virtual_storage::address_space *virtual_storage::address_space::create()
{
	auto *aspace = storage::allocz<virtual_storage::address_space>(sizeof(virtual_storage::address_space));

	// Allocate structures
	aspace->segtab = reinterpret_cast<virtual_storage::segment_entry *>(real_storage::alloc(sizeof(virtual_storage::segment_entry) * virtual_storage::max_segments, virtual_storage::page_align));
	if(aspace->segtab == nullptr) return nullptr; // Can't allocate segment table for KASPACE
	for(size_t i = 0; i < virtual_storage::max_segments; i++)
		aspace->segtab[i].invalid(true); // Mark as invalid
	aspace->cr1 = reinterpret_cast<uintptr_t>(aspace->segtab) | ((virtual_storage::max_segments / 512) - 1);
	debug_printf("ASCPAE.CR1=%p", static_cast<uintptr_t>(aspace->cr1));
	return aspace;
}

void virtual_storage::address_space::destroy(virtual_storage::address_space* aspace)
{
	real_storage::free(aspace);
}

int virtual_storage::address_space::map_page(void *virtaddr, void *physaddr, int flags)
{
	const auto _virtaddr = reinterpret_cast<uintptr_t>(virtaddr);
	const auto _physaddr = reinterpret_cast<uintptr_t>(physaddr);

	debug_printf("Mapping VIRT=%p,REAL=%p,FLAGS=%x", virtaddr, physaddr, (unsigned int)flags);
	debug_assert(_physaddr % virtual_storage::page_align == 0 && _virtaddr % virtual_storage::page_align == 0);

	// Acquire lock for ASPACE
	const base::scoped_mutex lock1(this->lock);

	// Divide the virtual address in sections
	//const size_t regtab3_index = (_virtaddr >> virtual_storage::regtab3_shift) & 0x7ff; // RFX, 11-bits
	//const size_t regtab2_index = (_virtaddr >> virtual_storage::regtab2_shift) & 0x7ff; // RSX, 11-bits
	//const size_t regtab1_index = (_virtaddr >> virtual_storage::regtab1_shift) & 0x7ff; // RTX, 11-bits
	const size_t segment_idx = (_virtaddr >> virtual_storage::segment_shift) & 0x7ff; // SGX, 11-bits
	const size_t page_idx = (_virtaddr >> virtual_storage::page_shift) & 0xff; // PTX, 8-bits

	auto *segment = &this->segtab[segment_idx];
	virtual_storage::page_entry *pagetab = nullptr;

	// Entry is unmapped
	if(segment->invalid()) {
		/// @todo Keep track of VMM allocations with a key
		pagetab = reinterpret_cast<virtual_storage::page_entry *>(real_storage::alloc(sizeof(virtual_storage::page_entry) * virtual_storage::max_pages, virtual_storage::page_align));
		if(pagetab == nullptr) return error::ALLOCATION;
		// Invalidate all page entries
		for(size_t i = 0; i < virtual_storage::max_pages; i++)
			pagetab[i].invalid(true); // Mark as invalid
		// Set the entry to the newly allocated pagetable
		segment->entry = (uintptr_t)pagetab;
	}
	// Entry is mapped already, take address
	else {
		// Take the address component from the segment entry
		pagetab = reinterpret_cast<virtual_storage::page_entry *>(segment->origin());
		if(pagetab == nullptr) return error::ALLOCATION;
	}

	// And set the specific page to mapped
	pagetab[page_idx].entry = _physaddr | flags;
	return 0;
}

void *virtual_storage::address_space::virtual_to_real(void *virtaddr)
{
	const auto _virtaddr = reinterpret_cast<uintptr_t>(virtaddr);
	debug_printf("Translating vaddr=%p", virtaddr);

	// Acquire lock for ASPACE
	const base::scoped_mutex lock1(this->lock);

	// Divide the virtual address in sections
	//const size_t regtab3_index = (_virtaddr >> virtual_storage::regtab3_shift) & 0x7ff; // RFX, 11-bits
	//const size_t regtab2_index = (_virtaddr >> virtual_storage::regtab2_shift) & 0x7ff; // RSX, 11-bits
	//const size_t regtab1_index = (_virtaddr >> virtual_storage::regtab1_shift) & 0x7ff; // RTX, 11-bits
	const size_t segment_idx = (_virtaddr >> virtual_storage::segment_shift) & 0x7ff; // SGX, 11-bits
	const size_t page_idx = (_virtaddr >> virtual_storage::page_shift) & 0xff; // PTX, 8-bits

	auto *segment = &this->segtab[segment_idx];

	// Entry is unmapped
	if(segment->invalid()) return nullptr;

	// Take the address component from the segment entry
	virtual_storage::page_entry *pagetab = (virtual_storage::page_entry *)segment->origin();
	if(pagetab == nullptr) return nullptr;

	// And set the specific page to mapped, don't forget to append the bits at the end
	return reinterpret_cast<void *>((uintptr_t)pagetab[page_idx].origin() | ((uintptr_t)virtaddr & 0xfff));
}

void *virtual_storage::address_space::phys2virt(void *physaddr)
{
	const auto _physaddr = reinterpret_cast<uintptr_t>(physaddr);
	const base::scoped_mutex lock1(this->lock); // Acquire lock for ASPACE
	for(size_t i = 0; i < virtual_storage::max_segments; i++) {
		auto *segment = &this->segtab[i];
		if(!segment->invalid()) {
			for(size_t j = 0; j < virtual_storage::max_pages; j++) {
				auto *pagetab = &(reinterpret_cast<virtual_storage::page_entry *>(segment->origin()))[j];
				if(!pagetab->invalid()) {
					// Physical address pointed here should also be on our side
					if(reinterpret_cast<uintptr_t>(pagetab->origin()) == _physaddr) {
						const uintptr_t virtaddr = (i << virtual_storage::segment_shift) | (j << virtual_storage::page_shift);
						return reinterpret_cast<void *>(virtaddr | (_physaddr & 0xfff));
					}
				}
			}
		}
	}
	return nullptr;
}

int virtual_storage::address_space::map_range(void *virtaddr, void *physaddr, int flags, size_t size)
{
	const auto _virtaddr = reinterpret_cast<uintptr_t>(virtaddr);
	const auto _physaddr = reinterpret_cast<uintptr_t>(physaddr);

	// Align the size to multiples of PAGE_ALIGN
	if(size % virtual_storage::page_align)
		size = size + (virtual_storage::page_align - (size % virtual_storage::page_align));

	// Map the entire range
	for(size_t i = 0; i < size; i += virtual_storage::page_align)
		this->map_page((void *)(_virtaddr + i), (void *)(_physaddr + i), flags);
	return 0;
}
