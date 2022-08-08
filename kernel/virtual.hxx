#ifndef VIRTUAL_STORAGE_HXX
#define VIRTUAL_STORAGE_HXX 1

#include <types.hxx>
#include <storage.hxx>
#include <arch/virtual.hxx>

namespace virtual_storage {
	struct block {
		enum type {
			INVALID,
			FREE,
			USED,
		} type;
		size_t length;
		virtual_storage::block *next;
	};
	
	struct heap {
		storage::concurrent_dynamic_list<virtual_storage::block> blocks;
	};
};

#endif
