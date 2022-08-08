#ifndef HANDLERS_HXX
#define HANDLERS_HXX

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LIBC_ABI
#	include <types.hxx>
#   include <abi_bits.h>
#endif

/**
 * @brief Perform a service call to the kernel
 * 
 * @param code Call code
 * @param arg1
 * @param arg2 
 * @param arg3 
 * @return uintptr_t The return value (defaults to 0)
 */
static inline uintptr_t io_svc(uintptr_t code, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
	/* R1 is used as a code, then later reused */
	register uintptr_t r1 asm("rax") = arg1;
	register uintptr_t r2 asm("rbx") = arg2;
	register uintptr_t r3 asm("rcx") = arg3;
	register uintptr_t r4 asm("rdx") = code;
	asm volatile(
		"int $26\r\n"
		: "+r"(r4)
		: "r"(r1), "r"(r2), "r"(r3)
		: );
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
