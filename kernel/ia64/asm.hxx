#ifndef ASM_HXX
#define ASM_HXX

#include <types.hxx>

namespace arch_dep {
	typedef uintptr_t register_t;

	struct processor_context {
		union {
			arch_dep::register_t gp_regs[16];
			struct {
				arch_dep::register_t r0;
				arch_dep::register_t r1;
				arch_dep::register_t r2;
				arch_dep::register_t r3;
				arch_dep::register_t r4;
				arch_dep::register_t r5;
				arch_dep::register_t r6;
				arch_dep::register_t r7;
				arch_dep::register_t r8;
				arch_dep::register_t r9;
				arch_dep::register_t r10;
				arch_dep::register_t r11;
				arch_dep::register_t r12;
				arch_dep::register_t r13;
				arch_dep::register_t r14;
				arch_dep::register_t r15;
			};
		};
		uintptr_t pc;

		/// @brief Saves the current context into the local scratch area
		inline void save_scratch_local() const {
			volatile arch_dep::register_t *grsav = reinterpret_cast<volatile arch_dep::register_t *>((uintptr_t)0);
			for(auto i = 0; i < 16; i++)
				grsav[i] = this->gp_regs[i];
		}

		/// @brief Save context to current thread (obtained from the scratch frame)
		inline void load_scratch_local() {
			volatile arch_dep::register_t *grsav = reinterpret_cast<volatile arch_dep::register_t *>((uintptr_t)0);
			for(auto i = 0; i < 16; i++)
				this->gp_regs[i] = grsav[i];
		}
	};
}

namespace ia64_intrin {

}

#endif
