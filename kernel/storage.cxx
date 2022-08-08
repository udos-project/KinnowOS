/* storage.cxx */

#include <types.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <timeshr.hxx>
#include <arch/asm.hxx>
#include <errcode.hxx>

// Functions so IA64, OR1K and other RISC architectures don't get angry
extern "C" void memset(void *ptr, int c, unsigned int size);
extern "C" void memcpy(void *dest, const void *src, unsigned int size);
extern "C" void abort();

extern "C" void memset(void *ptr, int c, unsigned int size)
{
	auto *cptr = reinterpret_cast<uint8_t *>(ptr);
	while(size) {
		*(cptr++) = static_cast<uint8_t>(c);
		size--;
	}
}

extern "C" void memcpy(void *dest, const void *src, unsigned int size)
{
	auto *cdest = reinterpret_cast<uint8_t *>(dest);
	const auto *csrc = reinterpret_cast<const uint8_t *>(src);
	while(size) {
		*(cdest++) = *(csrc++);
		size--;
	}
}

extern "C" void abort()
{
	while(1);
}

