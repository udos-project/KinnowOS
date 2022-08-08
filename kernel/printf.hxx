#ifndef PRINTF_HXX
#define PRINTF_HXX

#include <stdarg.h>
#include <types.hxx>

int kprintf(const char *fmt, ...) /*__attribute__((format(printf, 1, 2)))*/;
void kpanic(const char *fmt, ...);

#if defined DEBUG
#	define debug_printf(...) kprintf(__VA_ARGS__)
#else
#	define debug_printf(...)
#endif

#define debug_css_print(schid, fmt) debug_printf("css:%i:%i: " fmt, (int)schid.id, (int)schid.num)
#define debug_css_printf(schid, fmt, ...) debug_printf("css:%i:%i: " fmt, (int)schid.id, (int)schid.num, __VA_ARGS__)

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define debug_file_source(f, l) f ":" STRINGIZE2(l)

#if defined DEBUG
#	include <printf.hxx>
#	if !defined debug_assert
#		define debug_assert(expr) \
if(!(expr)) { \
	kpanic(debug_file_source(__FILE__, __LINE__) ": " #expr);\
}
#	endif
#	if !defined debug_assertm
#		define debug_assertm(expr, ...) \
if(!(expr)) { \
	kpanic(debug_file_source(__FILE__, __LINE__) ": " #expr ": " __VA_ARGS__);\
}
#	endif
#else
#	if !defined debug_assert
#		define debug_assert(expr)
#	endif
#	if !defined debug_assertm
#		define debug_assertm(expr, ...)
#	endif
#endif

#endif
