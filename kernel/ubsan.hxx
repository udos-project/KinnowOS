#ifndef UBSAN_HXX
#define UBSAN_HXX

#include <types.hxx>

extern "C" {
	struct ubsan_source {
		const char *file;
		uint32_t line;
		uint32_t col;
	};

	enum ubsan_type_kind {
		UBSAN_KIND_INT = 0x000,
		UBSAN_KIND_FLOAT = 0x0001,
		UBSAN_KIND_UNKNOWN = 0xFFFF
	};

	struct ubsan_type {
		uint16_t kind;
		uint16_t info;
		char name[];
	};

	struct ubsan_overflow {
		struct ubsan_source loc;
		const struct ubsan_type *type;
	};

	struct ubsan_invalid_value {
		struct ubsan_source loc;
		const struct ubsan_type *type;
	};

	struct ubsan_type_mismatch {
		struct ubsan_source loc;
		const struct ubsan_type *type;
		unsigned char align;
		unsigned char check_kind;
	};

	struct ubsan_shift_oob {
		struct ubsan_source loc;
		const struct ubsan_type *left_type;
		const struct ubsan_type *right_type;
	};

	struct ubsan_array_oob {
		struct ubsan_source loc;
		const struct ubsan_type *array_type;
		const struct ubsan_type *index_type;
	};

	struct ubsan_unreachable {
		struct ubsan_source loc;
	};

	struct ubsan_invalid_builtin {
		struct ubsan_source loc;
		unsigned char kind;
	};

	struct ubsan_non_null_return {
		struct ubsan_source loc;
	};

	struct ubsan_non_null_argument {
		struct ubsan_source loc;
	};

	struct ubsan_negative_vla {
		struct ubsan_source loc;
		const struct ubsan_type *type;
	};

	void __ubsan_handle_add_overflow(struct ubsan_overflow *data);
	void __ubsan_handle_sub_overflow(struct ubsan_overflow *data);
	void __ubsan_handle_mul_overflow(struct ubsan_overflow *data);
	void __ubsan_handle_divrem_overflow(struct ubsan_overflow *data);
	void __ubsan_handle_negate_overflow(struct ubsan_overflow *data);
	void __ubsan_handle_pointer_overflow(struct ubsan_overflow *data);
	void __ubsan_handle_shift_out_of_bounds(struct ubsan_shift_oob *data);
	void __ubsan_handle_load_invalid_value(struct ubsan_invalid_value *data);
	void __ubsan_handle_out_of_bounds(struct ubsan_array_oob *data);
	void __ubsan_handle_type_mismatch_v1(struct ubsan_type_mismatch *data, uintptr_t ptr);
	void __ubsan_handle_vla_bound_not_positive(struct ubsan_negative_vla *data);
	void __ubsan_handle_nonnull_return(struct ubsan_non_null_return *data);
	void __ubsan_handle_nonnull_arg(struct ubsan_non_null_argument *data);
	void __ubsan_handle_builtin_unreachable(struct ubsan_unreachable *data);
	void __ubsan_handle_invalid_builtin(struct ubsan_invalid_builtin *data);
}

#endif
