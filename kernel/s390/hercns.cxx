// hdebug.cxx
//
// Hercules debug facility driver, implements a basic diagnostic output function
// that does not work on real 3X0 machines, but it works on the emulator so it's
// used for debugging purpouses

#include <printf.hxx>
#include <storage.hxx>
#include <locale.hxx>
#include <s390/css.hxx>
#include <s390/hercns.hxx>

int hercns::init()
{
	debug_printf("\x01\x09 hercns driver");
	auto *driver = virtual_disk::driver::create();
	debug_assert(driver != nullptr);
	driver->write = [](virtual_disk::handle&, const void *buf, size_t n) -> int {
		static char tmpbuf[80] = { '/', '\0' };
		constexpr size_t off = 1;
		n++;
		n = (n >= sizeof(tmpbuf)) ? sizeof(tmpbuf) - off : n;
		storage::copy(tmpbuf + off, buf, n - off);
		
		// Blank out newlines (so HERCULES logs are not messed up)
		for(size_t i = off; i < n; i++) {
			// Required for multilanguage settings
			tmpbuf[i] = locale::convert<char, locale::charset::NATIVE, locale::charset::EBCDIC_1047>(locale::toupper(tmpbuf[i]));
		}
		asm volatile("DIAG %0, %1, 8" : : "r"(tmpbuf), "r"(n) : "cc", "memory");
		return 0;
	};
	auto *node = virtual_disk::node::create("/", "HWDEBUG");
	debug_assert(node != nullptr);
	driver->add_node(*node);
	return 0;
}
