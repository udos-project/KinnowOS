// OpenRISC boot code

#include <or1k/asm.hxx>
#include <or1k/handlers.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <timeshr.hxx>

extern uint8_t bss_start[], bss_end[];
extern void main() noexcept;
void init_smp() noexcept;

//
// Code
//
// Compile-time assertions (hopefully)
static_assert(sizeof(uintptr_t) == 4);

extern "C" {
	SECTION(".bss.tail") ALIGNED(8) uint8_t stack[8192];
	SECTION(".head.text") NAKED_FUNC void start() noexcept
	{
		asm volatile("l.movhi r0,0" : : ); // Clear r0
		
		// Clear the .bss section hopefully it gets synthetized into a register!
		for(auto *ptr = &bss_start[0]; reinterpret_cast<uintptr_t>(ptr) < reinterpret_cast<uintptr_t>(&bss_end[0]); ptr++)
			*ptr = 0;
		
		// Jump to the kernel, delay slots are extremely important!
		register uintptr_t stackptr asm("1") = STACK_TOP(stack);
		register uintptr_t frameptr asm("2") = stackptr; // Frame pointer is optional?
		asm volatile("l.jr %0\r\nl.nop" : : "r"((uintptr_t)&main), "r"(stackptr), "r"(frameptr) : "memory", "cc");
	}
}

ALIGNED(4) NAKED_FUNC void smp_cpu_stub()
{
	asm volatile("l.jal .\r\nl.nop");
}

void init_smp() noexcept {
	kprintf("Waking up the other CPUs\r\n");
}
