// RISC-V boot code

#include <riscv/asm.hxx>
#include <riscv/handlers.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <timeshr.hxx>
#include <fdt.hxx>

// RISC-V doesn't clear the upper half so we have to force GCC to do so
#define UPPER_CLEAR(x) reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(x) & 0xFFFFFFFF)

extern uint8_t bss_start[], bss_end[];
extern uint8_t global_pointer[];
extern void main() noexcept;
extern "C" void start() noexcept;
void smp_cpu_stub() noexcept;
void init_smp() noexcept;

//
// Code
//

asm(".option norvc");
extern "C" {
	SECTION(".bss.tail") ALIGNED(8) uint8_t stack[8192];
	SECTION(".head.text") NAKED_FUNC void start() noexcept
	{
		// Set the global pointer
		asm volatile(
			".option push\r\n"
			".option norelax\r\n"
			"\tla gp, global_pointer\r\n"
			".option pop\r\n"
		);

		// Reset SATP
		asm volatile("csrw satp, zero");
		
		// Clear the .bss section hopefully it gets synthetized into a register!
		for(auto *ptr = UPPER_CLEAR(&bss_start[0]); reinterpret_cast<uintptr_t>(ptr) < reinterpret_cast<uintptr_t>(&bss_end[0]); ptr++)
			*ptr = 0;

		// QEMU passes the SystemFDT on reg a1
		asm volatile("sd a1,%0" : "=m"(fdt::fdt_address) : : "a1");

		// Jump to the kernel, delay slots are extremely important!
		register uintptr_t stackptr asm("sp") = STACK_TOP(stack);
		asm volatile("tail %0" : : "X"((uintptr_t)&main), "r"(stackptr) : "memory", "cc");
	}
}

ALIGNED(4) NAKED_FUNC void smp_cpu_stub() noexcept
{
	asm volatile("j .");
}

void init_smp() noexcept {
	kprintf("Waking up the other CPUs\r\n");
}
