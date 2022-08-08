#ifndef SERVICE_HXX
#define SERVICE_HXX

#include <arch/asm.hxx>

namespace service {
	arch_dep::register_t common(const uint16_t code, const arch_dep::register_t arg1, const arch_dep::register_t arg2, const arch_dep::register_t arg3, const arch_dep::register_t arg4);
}

#endif
