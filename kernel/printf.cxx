#include <types.hxx>
#include <printf.hxx>
#include <storage.hxx>
#include <vdisk.hxx>
#include <arch/asm.hxx>

#ifdef TARGET_S390
#   include <s390/hercns.hxx>
#endif
#include <uart.hxx>

#include <mutex.hxx>
#include <errcode.hxx>

#define MAX_DIGITS 24

constinit static const char *common_dict[0x100] = {
	"", // 0x00
	"operation", // 0x01
	"specification", // 0x02
	"translation", // 0x03
	"authority", // 0x04
	"sequence", // 0x05
	"stack", // 0x06
	"transaction", // 0x07
	"unnormalized", // 0x08
	"init", // 0x09
	"end", // 0x0A
	"out of memory", // 0x0B
#ifdef TARGET_S390
#	if MACHINE == M_S360
	"UDOS/370", // 0x0C
#	elif MACHINE == M_S370
	"UDOS/370", // 0x0C
#	elif MACHINE == M_S370_XA
	"UDOS/370-XA", // 0x0C
#	elif MACHINE == M_S380
	"UDOS/380", // 0x0C
#	elif MACHINE == M_S390
	"UDOS/390", // 0x0C
#	elif MACHINE == M_ZARCH
	"z/UDOS", // 0x0C
#	else
	"UDOS", // 0x0C
#	endif
#elif defined TARGET_OR1K
	"UDOS/OR1K", // 0x0C
#elif defined TARGET_SPARC
#if MACHINE >= M_SPARC64
	"UDOS/SPARC64", // 0x0C
#elif MACHINE >= M_SPARC32
	"UDOS/SPARC32", // 0x0C
#else
	"UDOS/SPARC", // 0x0C
#endif
#elif defined TARGET_IA64
	"UDOS/IA64"
#elif defined TARGET_RISCV
#if MACHINE >= M_RISCV64
	"UDOS/RISCV64", // 0x0C
#elif MACHINE >= M_RISCV32
	"UDOS/RISCV32", // 0x0C
#else
	"UDOS/RISCV", // 0x0C
#endif
#elif defined TARGET_X86
#if MACHINE >= M_X86_64
	"UDOS/X86_64", // 0x0C
#elif MACHINE >= M_X86
	"UDOS/X86", // 0x0C
#else
	"UDOS/86", // 0x0C
#endif
#else
	"UDOS", // 0x0C
#endif
	"memory manager", // 0x0D
	"overflow", // 0x0E
	"underflow", // 0x0F
	"libubsan", // 0x10
	"dataset", // 0x11
	"virtual disk", // 0x12
	"invalid", // 0x13
	"without", // 0x14
	"WARNING", // 0x15
	"ERROR", // 0x16
	"out of bounds", // 0x17
	"facility", // 0x18
	"extended", // 0x19
	"cryptographic", // 0x1A
	"displacement", // 0x1B
	"performing", // 0x1C
	"relocation", // 0x1D
	"loading", // 0x1E
	"reading", // 0x1F
	"writing", // 0x20
	"success", // 0x21
	"failure", // 0x22
	"started", // 0x23
	"finished", // 0x24
};

// Protects prints from calling infinitely (device which prints info on kprintf for example)
static base::mutex print_lock;

static inline int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
	size_t i = 0;
	
	s[i] = '\0';
	while(*fmt != '\0' && i < n - 1) {
		if(*fmt == '%') {
			fmt++;

			bool is_signed = false;
			bool is_number = false;
			int base = 10;
			unsigned long long val;

			if(*fmt == 's') {
				const char *str = va_arg(args, const char *);
				if(str == nullptr) str = "nullptr";
				// Obtain the length and truncate
				auto len = storage_string::length(str);
				if(len >= n - i) len = n - i;
				
				for(size_t j = 0; j < len; j++) {
					if(str[j] == '\x01') {
						j++;
						uint8_t idx = static_cast<uint8_t>(str[j]);
						j++;
						size_t alen = storage_string::length(common_dict[idx]);
						if(alen >= n - i) alen = n - i;
						// Put an space to separate common dictionary words
						if(s[i] != ' ') s[i++] = ' ';
						storage::copy(&s[i], common_dict[idx], alen);
						i += alen;
					} else {
						s[i++] = str[j];
					}
				}
				fmt++;
			} else if(*fmt == 'c') {
				s[i++] = (char)va_arg(args, int);
				fmt++;
			} else if(*fmt == 'u') {
				is_signed = true;
				base = 10;
				val = static_cast<unsigned long long>(va_arg(args, unsigned int));
				is_number = true;
			} else if(*fmt == 'i') {
				is_signed = true;
				base = 10;
				val = static_cast<unsigned long long>(va_arg(args, signed int));
				is_number = true;
			} else if(*fmt == 'p') {
				is_signed = true;
				base = 16;
				val = static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(va_arg(args, void *)));
				is_number = true;
			} else if(*fmt == 'x') {
				is_signed = true;
				base = 16;
				val = static_cast<unsigned long long>(va_arg(args, unsigned int));
				is_number = true;
			}

			if(is_number) {
				i += ([is_signed, base](unsigned long long val, char *str, size_t maxlen) -> size_t {
					char numbuf[MAX_DIGITS];
					size_t l = 0;

					debug_assert(str != nullptr && base != 0);
					if(val == 0) {
						str[l++] = '0';
						str[l] = '\0';
						return 1;
					}

					if(is_signed && (signed long long)val < 0) {
						str[l++] = '-'; // Negative sign
						val = (unsigned long long)(-((signed long long)val));
					}
					
					size_t j = 0;
					while(val) {
						int rem = (int)(val % (unsigned long long)base);
						numbuf[j++] = (char)((rem >= 10) ? rem - 10 + 'A' : rem + '0');
						val /= (unsigned long long)base;
					}

					for(size_t k = 0 ; k <= j - 1 && l < maxlen; k++)
						str[l++] = numbuf[j - k];
					str[l] = '\0';
					return l;
				})(val, &s[i], n - i);
				fmt++;
			}
		} else if(*fmt == '\x01') {
			// Expansion of common words (for saving space lmao)
			fmt++;
			auto idx = static_cast<uint8_t>(*fmt);
			auto len = storage_string::length(common_dict[idx]);
			if(len >= n - i) len = n - i;
			// Put an space to separate common dictionary words
			if(s[i] != ' ') s[i++] = ' ';
			storage::copy(&s[i], common_dict[idx], len);
			i += len;
			fmt++;
		} else {
			s[i++] = *(fmt++);
		}
		s[i] = '\0';
	}
end:
	// Truncate index
	if(i >= n) i = n - 1;
	s[i] = '\0'; // Null terminate
	return 0;
}

#include <locale.hxx>
constinit virtual_disk::handle *g_stdout = nullptr;
static inline void dputs(char *buf, size_t n)
{
	if(g_stdout != nullptr) {
		g_stdout->write(buf, n);
	} else {
#ifdef TARGET_S390
		static char tmpbuf[80];
		constexpr size_t off = 1;
		n++;
		n = (n >= sizeof(tmpbuf)) ? sizeof(tmpbuf) - off : n;
		storage::copy(tmpbuf + off, buf, n - off);
		
		// Blank out newlines (so HERCULES logs are not messed up)
		for(size_t i = off; i < n; i++) {
			// Required for multilanguage settings
			tmpbuf[i] = locale::convert<char, locale::charset::NATIVE, locale::charset::EBCDIC_1047>(tmpbuf[i]);
		}
		tmpbuf[0] = 0x61; // X'61' is '/' in EBCDIC
		asm volatile("DIAG %0, %1, 8" : : "r"(tmpbuf), "r"(n) : "cc", "memory");
#elif defined TARGET_RISCV
		// Translation
		for(size_t i = 0; i < n; i++) {
			*((volatile uint8_t *)0x10000000) = locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>(buf[i]);
		}
		*((volatile uint8_t *)0x10000000) = '\n';
#endif
	}
}

static inline int kvprintf(const char *fmt, va_list args)
{
	debug_assert(fmt != nullptr);
	static char tmpbuf[100];
	vsnprintf(tmpbuf, sizeof(tmpbuf), fmt, args);
	dputs(tmpbuf, storage_string::length(tmpbuf));
	return 0;
}

int kprintf(const char *fmt, ...)
{
	va_list args;
	debug_assert(fmt != nullptr);
	va_start(args, fmt);
	if(print_lock.try_lock() == 0) {
		va_end(args);
		return error::RESOURCE_BUSY;
	}
	int r = kvprintf(fmt, args);
	print_lock.unlock();
	va_end(args);
	return r;
}

#include <timeshr.hxx>
static base::mutex panic_lock;
void kpanic(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if(!panic_lock.try_lock()) {
		kprintf("Already panicked... a panic has been caused in the panic handling!\r\n");
		while(1);
	}

	kvprintf(fmt, args);
	kprintf("Taking down CPU... good night!\r\n");
	timeshare::disable();
#ifdef TARGET_S390
	s390_intrin::signal(s390_intrin::cpuid(), S390_SIGP_STOP);
#endif
	while(1);
}
