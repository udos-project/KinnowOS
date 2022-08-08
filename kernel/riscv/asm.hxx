#ifndef ASM_HXX
#define ASM_HXX

#include <types.hxx>

namespace arch_dep {
	typedef uintptr_t register_t;

	struct processor_context {
		union {
			arch_dep::register_t gp_regs[31];
			struct {
				arch_dep::register_t ra;
				arch_dep::register_t sp;
				arch_dep::register_t gp;
				arch_dep::register_t tp;
				arch_dep::register_t t0;
				arch_dep::register_t t1;
				arch_dep::register_t t2;
				arch_dep::register_t s0;
				arch_dep::register_t s1;
				arch_dep::register_t a0;
				arch_dep::register_t a1;
				arch_dep::register_t a2;
				arch_dep::register_t a3;
				arch_dep::register_t a4;
				arch_dep::register_t a5;
				arch_dep::register_t a6;
				arch_dep::register_t a7;
				arch_dep::register_t s2;
				arch_dep::register_t s3;
				arch_dep::register_t s4;
				arch_dep::register_t s5;
				arch_dep::register_t s6;
				arch_dep::register_t s7;
				arch_dep::register_t s8;
				arch_dep::register_t s9;
				arch_dep::register_t s10;
				arch_dep::register_t s11;
				arch_dep::register_t t3;
				arch_dep::register_t t4;
				arch_dep::register_t t5;
				arch_dep::register_t t6;
			};
		};
		uintptr_t pc;

		/// @brief Saves the current context into the local scratch area
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

namespace riscv_intrin {

}

#endif
