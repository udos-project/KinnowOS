#include <ubsan.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <locale.hxx>

/// @brief The ubsan implementation on GCC and (apparently) clang is not
/// aware of encodings beyond the ASCII default. So in order to properly display the
/// undefined error messages we have to manually convert them into their EBCDIC counterparts
/// take note that UDOS supports being compiled with different EBCDIC encodings so the
/// mileage may vary.
/// @param msg Message to encode into EBCDIC
static UBSAN_FUNC void ubsan_ascii_to_ebcdic(char *dst, const char *src, size_t len)
{
	for(size_t i = 0; i < len; i++)
		dst[i] = locale::convert<char, locale::charset::ASCII, locale::charset::NATIVE>(src[i]);
}

static UBSAN_FUNC void ubsan_print_location(const char *msg, struct ubsan_source *loc)
{
	if(loc != nullptr && loc->file != nullptr) {
		static char tmpbuf[20];
		ubsan_ascii_to_ebcdic(tmpbuf, loc->file, sizeof(tmpbuf));
		debug_printf("\x01\x10 %s @ %s:%i:%i", msg, tmpbuf, (int)loc->line, (int)loc->col);
	} else {
		debug_printf("\x01\x10 %s @ (corrupt location %p)", msg, loc);
	}
}

extern "C" UBSAN_FUNC void __ubsan_handle_add_overflow(struct ubsan_overflow *data)
{
	ubsan_print_location("addition\x01\x0E", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_sub_overflow(struct ubsan_overflow *data)
{
	ubsan_print_location("subtraction\x01\x0E", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_mul_overflow(struct ubsan_overflow *data)
{
	ubsan_print_location("multiple\x01\x0E", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_divrem_overflow(struct ubsan_overflow *data)
{
	ubsan_print_location("division\x01\x0E", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_negate_overflow(struct ubsan_overflow *data)
{
	ubsan_print_location("negative\x01\x0E", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_pointer_overflow(struct ubsan_overflow *data)
{
	ubsan_print_location("pointer\x01\x0E", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_shift_out_of_bounds(struct ubsan_shift_oob *data)
{
	ubsan_print_location("shift\x01\x17", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_load_invalid_value(struct ubsan_invalid_value *data)
{
	ubsan_print_location("load\x01\x13 value", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_out_of_bounds(struct ubsan_array_oob *data)
{
	ubsan_print_location("\x01\x17", &data->loc);
}

constinit static const char *type_check_types[10] = {
	"load of",
	"store to",
	"reference binding to",
	"member access within",
	"member call on",
	"constructor call on",
	"downcast of",
	"downcast of",
	"upcast of",
	"cast to virtual base of",
};

extern "C" UBSAN_FUNC void __ubsan_handle_type_mismatch_v1(struct ubsan_type_mismatch *data, uintptr_t ptr)
{
	if(ptr == 0) {
		ubsan_print_location("null pointer access", &data->loc);
	} else if(data->align != 0 && !(ptr & (data->align - 1))) {
		ubsan_print_location("unaligned access", &data->loc);
	} else {
		if(data->check_kind >= 10) {
			ubsan_print_location("type mismatch", &data->loc);
		} else {
			static char tmpbuf[20];
			ubsan_ascii_to_ebcdic(tmpbuf, data->type->name, sizeof(tmpbuf));
			debug_printf("%s address of %p\x01\x14 enough space for an object %s", type_check_types[data->check_kind], (uintptr_t)ptr, tmpbuf);
			ubsan_print_location("type mismatch", &data->loc);
		}
	}
}

extern "C" UBSAN_FUNC void __ubsan_handle_vla_bound_not_positive(struct ubsan_negative_vla *data)
{
	ubsan_print_location("negative vla index", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_nonnull_return(struct ubsan_non_null_return *data)
{
	ubsan_print_location("non-null return has null", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_nonnull_arg(struct ubsan_non_null_argument *data)
{
	ubsan_print_location("non-null argument has null", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_builtin_unreachable(struct ubsan_unreachable *data)
{
	ubsan_print_location("unreachable code", &data->loc);
}

extern "C" UBSAN_FUNC void __ubsan_handle_invalid_builtin(struct ubsan_invalid_builtin *data)
{
	ubsan_print_location("\x01\x13 builtin", &data->loc);
}
