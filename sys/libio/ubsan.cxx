#include <stdio.h>
#include <ubsan.h>
#include <string.h>
#include <assert.h>

UBSAN_FUNC static int toupper(int x)
{
    return
        (x) == 'a' ? 'A' : (x) == 'g' ? 'G' : (x) == 'm' ? 'M' : (x) == 's' ? 'S' :
        (x) == 'b' ? 'B' : (x) == 'h' ? 'H' : (x) == 'n' ? 'N' : (x) == 't' ? 'T' :
        (x) == 'c' ? 'C' : (x) == 'i' ? 'I' : (x) == 'o' ? 'O' : (x) == 'u' ? 'U' :
        (x) == 'd' ? 'D' : (x) == 'j' ? 'J' : (x) == 'p' ? 'P' : (x) == 'v' ? 'V' :
        (x) == 'e' ? 'E' : (x) == 'k' ? 'K' : (x) == 'q' ? 'Q' : (x) == 'w' ? 'W' :
        (x) == 'f' ? 'F' : (x) == 'l' ? 'L' : (x) == 'r' ? 'R' : (x) == 'x' ? 'X' :
        (x) == 'y' ? 'Y' : (x) == 'z' ? 'Z' : (x);
}

UBSAN_FUNC constinit static const unsigned char asc2nat[] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\r', ' ', ' ', '\n', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',

    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ',
    /* Higher 128 bytes are not used */
};

/// @brief The ubsan implementation on GCC and (apparently) clang is not
/// aware of encodings beyond the ASCII default. So in order to properly display the
/// undefined error messages we have to manually convert them into their EBCDIC counterparts
/// take note that UDOS supports being compiled with different EBCDIC encodings so the
/// mileage may vary.
/// @param msg Message to encode into EBCDIC
UBSAN_FUNC static inline void ubsan_ascii_to_ebcdic(char *msg)
{
    size_t len = strlen(msg);
    for(size_t i = 0; i < len; i++) {
        if(msg[i] >= 0x80) {
            msg[i] = '.';
            continue;
        }
        msg[i] = (char)toupper(asc2nat[msg[i] & 0x7F]);
    }
}

UBSAN_FUNC static void ubsan_print_location(const char *msg, struct ubsan_source *loc)
{
    if(loc != NULL && loc->file != NULL) {
        char tmpbuf[20];
        strncpy(tmpbuf, loc->file, sizeof tmpbuf);
        ubsan_ascii_to_ebcdic(tmpbuf);
        fprintf(stderr, "ubsan: %s @ %s:%i:%i", msg, tmpbuf, (int)loc->line, (int)loc->col);
    } else {
        fprintf(stderr, "ubsan: %s @ (corrupt location %p)", msg, loc);
    }
}

UBSAN_FUNC void __ubsan_handle_add_overflow(struct ubsan_overflow *data)
{
    ubsan_print_location("addition overflow", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_sub_overflow(struct ubsan_overflow *data)
{
    ubsan_print_location("subtraction overflow", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_mul_overflow(struct ubsan_overflow *data)
{
    ubsan_print_location("multiple overflow", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_divrem_overflow(struct ubsan_overflow *data)
{
    ubsan_print_location("division overflow", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_negate_overflow(struct ubsan_overflow *data)
{
    ubsan_print_location("negative overflow", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_pointer_overflow(struct ubsan_overflow *data)
{
    ubsan_print_location("pointer overflow", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_shift_out_of_bounds(struct ubsan_shift_oob *data)
{
    ubsan_print_location("shift out of bounds", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_load_invalid_value(struct ubsan_invalid_value *data)
{
    ubsan_print_location("load invalid value", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_out_of_bounds(struct ubsan_array_oob *data)
{
    ubsan_print_location("out of bounds", &data->loc);
}

static const char *type_check_types[10] = {
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

UBSAN_FUNC void __ubsan_handle_type_mismatch_v1(struct ubsan_type_mismatch *data, uintptr_t ptr)
{
    if(ptr == 0) {
        ubsan_print_location("null pointer access", &data->loc);
    } else if(data->align != 0 && !(ptr & (data->align - 1))) {
        ubsan_print_location("unaligned access", &data->loc);
    } else {
        if(data->check_kind >= 10) {
            ubsan_print_location("type mismatch", &data->loc);
        } else {
            char tmpbuf[20];
            strncpy(tmpbuf, data->type->name, sizeof tmpbuf);
            ubsan_ascii_to_ebcdic(tmpbuf);
            fprintf(stderr, "%s address of %p with not enough space for an object %s", type_check_types[data->check_kind], (uintptr_t)ptr, tmpbuf);

            ubsan_print_location("type mismatch", &data->loc);
        }
    }
}

UBSAN_FUNC void __ubsan_handle_vla_bound_not_positive(struct ubsan_negative_vla *data)
{
    ubsan_print_location("negative vla index", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_nonnull_return(struct ubsan_non_null_return *data)
{
    ubsan_print_location("non-null return has null", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_nonnull_arg(struct ubsan_non_null_argument *data)
{
    ubsan_print_location("non-null argument has null", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_builtin_unreachable(struct ubsan_unreachable *data)
{
    ubsan_print_location("unreachable code", &data->loc);
}

UBSAN_FUNC void __ubsan_handle_invalid_builtin(struct ubsan_invalid_builtin *data)
{
    ubsan_print_location("invalid builtin", &data->loc);
}
