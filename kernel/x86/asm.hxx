#ifndef ASM_HXX
#define ASM_HXX

#include <types.hxx>

#ifdef __GNUC__
#   define UBSAN_FUNC __attribute__((no_sanitize("undefined")))
#   define ALIGNED(x) __attribute__((aligned(x)))
#   define PACKED __attribute__((packed))
#   define SECTION(x) __attribute__((section(x)))
#   define NAKED_FUNC __attribute__((naked)) UBSAN_FUNC
#   define ATTRIB_MALLOC __attribute__((malloc))
#else
#   define UBSAN_FUNC __attribute__((no_sanitize_undefined))
#   error Define your macros here
#endif

namespace arch_dep {
	typedef uintptr_t register_t;

	struct processor_context {
		union {
			arch_dep::register_t gp_regs[16];
			struct {
				arch_dep::register_t rax;
				arch_dep::register_t rbx;
				arch_dep::register_t rcx;
				arch_dep::register_t rdx;
				arch_dep::register_t rbp;
				arch_dep::register_t rsp;
				arch_dep::register_t rsi;
				arch_dep::register_t rdi;
				// Newer 8-15 registers only in x86_64
#if MACHINE >= M_X86_64
				arch_dep::register_t r8;
				arch_dep::register_t r9;
				arch_dep::register_t r10;
				arch_dep::register_t r11;
				arch_dep::register_t r12;
				arch_dep::register_t r13;
				arch_dep::register_t r14;
				arch_dep::register_t r15;
#endif
			};
		};
		uintptr_t pc;

		/**
		 * @brief Saves the current context into the local scratch area
		 * 
		 */
		inline void save_scratch_local() const {
			volatile arch_dep::register_t *grsav = reinterpret_cast<volatile arch_dep::register_t *>((uintptr_t)0);
			for(auto i = 0; i < 16; i++)
				grsav[i] = this->gp_regs[i];
		}

		/**
		 * @brief Save context to current thread (obtained from the scratch frame)
		 * 
		 */
		inline void load_scratch_local() {
			volatile arch_dep::register_t *grsav = reinterpret_cast<volatile arch_dep::register_t *>((uintptr_t)0);
			for(auto i = 0; i < 16; i++)
				this->gp_regs[i] = grsav[i];
		}
	};
}

namespace x86_intrin {
	inline void outb(uint16_t port, uint8_t val) {
		asm volatile(
			"outb %0, %1"
			:
			: "a"(val), "Nd"(port)
		);
	}

	inline void outw(uint16_t port, uint16_t val) {
		asm volatile(
			"outw %0, %1"
			:
			: "a"(val), "Nd"(port)
		);
	}

	inline void outl(uint16_t port, uint32_t val) {
		asm volatile(
			"outl %0, %1"
			:
			: "a"(val), "Nd"(port)
		);
	}

	inline void outq(uint16_t port, uint64_t val) {
		asm volatile(
			"outq %0, %1"
			:
			: "a"(val), "Nd"(port)
		);
	}

	inline void inb(uint16_t port, uint8_t val) {
		asm volatile(
			"inb %1, %0"
			: "=a"(val)
			: "Nd"(port)
		);
	}

	inline void inw(uint16_t port, uint16_t val) {
		asm volatile(
			"inw %1, %0"
			: "=a"(val)
			: "Nd"(port)
		);
	}

	inline void inl(uint16_t port, uint32_t val) {
		asm volatile(
			"inl %1, %0"
			: "=a"(val)
			: "Nd"(port)
		);
	}

	inline void inq(uint16_t port, uint64_t val) {
		asm volatile(
			"inq %1, %0"
			: "=a"(val)
			: "Nd"(port)
		);
	}
}

#endif
