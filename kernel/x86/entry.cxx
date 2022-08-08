// x86 boot code

#include <x86/asm.hxx>
#include <x86/handlers.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <timeshr.hxx>
#include <boot.hxx>

extern uint8_t bss_start[], bss_end[];
extern void main() noexcept;
void init_smp() noexcept;

SECTION(".multiboot") constinit static volatile multiboot::header mhdr = {
	.magic = 0x1BADB002,
	.flags = 0,
	.checksum = -(0x1BADB002 + 0),
};

//
// Code
//
// Compile-time assertions (hopefully)
static_assert(sizeof(uintptr_t) == 4);

extern "C" {
	SECTION(".bss.tail") ALIGNED(8) uint8_t stack[8192];

	NAKED_FUNC void start() noexcept;
	SECTION(".head.text") NAKED_FUNC void start() noexcept
	{
		// Multiboot already cleared our bss
		// Jump to the kernel, delay slots are extremely important!
		register unsigned stackptr asm("esp") = STACK_TOP(stack);
		asm volatile("call %0" : : "m"(main), "r"(stackptr) : "memory", "cc");
	}
}

NAKED_FUNC void smp_cpu_stub();
ALIGNED(4) NAKED_FUNC void smp_cpu_stub()
{
	asm volatile("jmp .");
}

void init_smp() noexcept {
	kprintf("Waking up the other CPUs\r\n");
}
