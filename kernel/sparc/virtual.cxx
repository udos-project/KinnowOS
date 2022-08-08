#include <sparc/virtual.hxx>
#include <storage.hxx>
#include <printf.hxx>

int virtual_storage::init()
{
	return 0;
}

virtual_storage::address_space *virtual_storage::address_space::create()
{
	virtual_storage::address_space *aspace = (virtual_storage::address_space *)storage::allocz(sizeof(virtual_storage::address_space));

	// Allocate structures
	aspace->segtab = (virtual_storage::page_entry *)real_storage::alloc(4096, virtual_storage::page_align);
	if(aspace->segtab == nullptr) return nullptr; // Can't allocate segment table for KASPACE

	kpanic("Not implemented!");
	return aspace;
}

void virtual_storage::address_space::destroy(virtual_storage::address_space* aspace)
{
	real_storage::free(aspace);
}

int virtual_storage::address_space::map_page(void *virtaddr, void *physaddr, int flags)
{
	kpanic("Not implemented!");
	return 0;
}

void *virtual_storage::address_space::virtual_to_real(void *virtaddr)
{
	kpanic("Not implemented!");
	return nullptr;
}

void *virtual_storage::address_space::phys2virt(void *physaddr)
{
	kpanic("Not implemented!");
	return nullptr;
}

int virtual_storage::address_space::map_range(void *virtaddr, void *physaddr, int flags, size_t size)
{
	kpanic("Not implemented!");
	return 0;
}
