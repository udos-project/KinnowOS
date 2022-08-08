#ifndef ASM_HXX
#define ASM_HXX

#include <types.hxx>

// s390 manual describes bits as MSB - so for sake of readability we do this

/// @brief Convert a bit from normal LSB notation to MSB
/// @param n Size of the object
/// @param x Bit to access
#define S390_BIT(n, x) (((n) - 1) - (x))

/// @brief Convert a bit from normal LSB notation to MSB (usage for multi)
/// @param n Size of the object
/// @param x Bit to access (0-based)
/// @param sz Size of access
#define S390_BIT_MULTI(n, x, sz) S390_BIT((n), (x) + ((sz) - 1))

// Note that AM24 and AM31 have to be along with the instruction address (in a
// 128-bit PSW they have to be on the low part of the flags
#define PSW_AM24 0x00000000
#define PSW_AM31 0x80000000
#define PSW_AM64 0x00000001

// This varies depending on the given system build
#if MACHINE > M_S360
#	define PSW_DEFAULT_AMBIT (PSW_AM31)
#else
#	define PSW_DEFAULT_AMBIT (PSW_AM24)
#endif

#if MACHINE >= M_ZARCH
/* Tracing Time-Of-Day control */
#define S390_CR0_TRACE_TOD ((1) << S390_BIT(64, 32))

/* Set system mask supression control */
#define S390_CR0_SSM ((1) << S390_BIT(64, 33))

/* Time-Of-Day clock synchronization control */
#define S390_CR0_TOD_CLOCK_SYNC ((1) << S390_BIT(64, 34))

/* Low address protection */
#define S390_CR0_LA_PROTECT ((1) << S390_BIT(64, 35))

/* Extraction instruction authorization control */
#define S390_CR0_EXA ((1) << S390_BIT(64, 36))

/* Secondary space control instruction authorization control */
#define S390_CR0_SSPACE ((1) << S390_BIT(64, 37))

/* Fetch protection override control */
#define S390_CR0_FETCH_PROTECT(x) ((x) << S390_BIT(64, 38))

/* Enhanced DAT enablement control */
#define S390_CR0_EDATED ((1) << S390_BIT(64, 40))

/* A-Floating-Point register control */
#define S390_CR0_AFP_REGISTER ((1) << S390_BIT(64, 45))

/* Enable z/Arch vector facility */
#define S390_CR0_ENABLE_VECTOR ((1) << S390_BIT(64, 46))

/* CPU-Timer subclass mask */
#define S390_CR0_TIMER_MASK ((1) << S390_BIT(64, 53))
#endif

#if MACHINE >= M_ZARCH
/* Primary subspace group control */
#define S390_CR1_PSG ((1) << S390_BIT(64, 54))

/* Primary private space control */
#define S390_CR1_PPS ((1) << S390_BIT(64, 55))

/* Primary storage alteration event control */
#define S390_CR1_PSAE ((1) << S390_BIT(64, 56))

/* Primary space-switch event control */
#define S390_CR1_PSSE ((1) << S390_BIT(64, 57))

/* Primary real-space control, if 1 it means the ASCE should be treated as if it was a real
 * address space, meaning no translation is to be done */
#define S390_CR1_PRS (1 << S390_BIT(64, 58))

/* Primary designation-type control */
#define S390_CR1_PDT(x) ((x & 0x3) << S390_BIT(64, 61))

/* Table length (in multiples of 4096 bytes or 512 entries) */
#define S390_CR1_TABLE_LEN(x) ((x & 0x3) << S390_BIT(64, 63))

/* Primary segment table origin */
#define S390_CR1_PSGT_ORIGIN(x) ((x) << S390_BIT_MULTI(64, 0, 52))
#else
/* Primary segment table origin */
#define S390_CR1_PSGT_ORIGIN(x) ((x) << S390_BIT_MULTI(32, 1, 20))
#endif

/* Storage key bits, see the Principles of Operation Page 111 */

/* Access bits */
#define S390_SKEY_ACC(x) (1 << S390_BIT_MULTI(7, 0, 4))

/* Fetch bit, protects the storage from fetches */
#define S390_SKEY_FETCH (1 << S390_BIT(7, 4))

/* Reference bit */
#define S390_SKEY_REFERENCE (1 << S390_BIT(7, 5))

/* Change bit (set when something is stored here) */
#define S390_SKEY_CHANGE (1 << S390_BIT(7, 6))

#define MAX_CPUS 248

/* Program event recording - this is for debugging stuff */
#define PSW_PER ((1) << S390_BIT(32, 1))

/* Controls dynamic address translation */
#define PSW_DAT ((1) << S390_BIT(32, 5))

/* I/O interrupt mask */
#define PSW_IO_INT ((1) << S390_BIT(32, 6))

/* External interrupt mask stuff like processor signals and clocks */
#define PSW_EXTERNAL_INT ((1) << S390_BIT(32, 7))

/* Archmode that the PSW will run in */
#define PSW_ENABLE_ARCHMODE ((1) << S390_BIT(32, 12))

/* Aliases where 1 = ESA, 0 = z/Arch */
#define PSW_S390_ARCHMODE PSW_ENABLE_ARCHMODE
#define PSW_ZARCH_ARCHMODE 0ul
/* Helper for assignment of PSW */
#if MACHINE >= M_ZARCH
#   define PSW_DEFAULT_ARCHMODE PSW_ZARCH_ARCHMODE
#else
#   define PSW_DEFAULT_ARCHMODE PSW_S390_ARCHMODE
#endif

/* Enable machine check interrupts */
#define PSW_ENABLE_MCI ((1) << S390_BIT(32, 13))

/* Makes the processor halt until an interrupt comes */
#define PSW_WAIT_STATE ((1) << S390_BIT(32, 14))

/* Problem state - aka. userland switch */
#define PSW_PROBLEM_STATE ((1) << S390_BIT(32, 15))

//
// In S/390 there is an area starting at 0, with a length of 8192 (z/Arch) or
// 1024 (S/390) in that area there are various elements which can be tinkered
// with. It has PSWs which helps the machine to jump to an address when
// something happens, normally fields with FLC* are S/390 and the ones with
// FLCE* are z/Arch exclusively.
//
// The PSWs can be tought of a traditional interrupt vector table, however the
// table is not linear and it's spread out everywhere. And there is only about 8
// interrupts you can program.
//
// The IRQ routing system helps to mitigate this hardware problem but it gives
// more work to the I/O interrupt due to the work of routing every single device
// wanting to do I/O.
#define PSA_FLCFACL0_N3 0x80
#define PSA_FLCFACL0_ZARCH_INSTALL 0x40
#define PSA_FLCFACL0_ZARCH_ACTIVE 0x20
#if MACHINE >= M_ZARCH
// (Only on z/Arch) IDTE facility installed */
#	define PSA_FLCFACL0_IDTE 0x10
// (Only on z/Arch) clear segment upon invalidation */
#	define PSA_FLCFACL0_IDTE_CLEAR_SEGMENT 0x08
// (Only on z/Arch) clear region upon invalidation */
#	define PSA_FLCFACL0_IDTE_CLEAR_REGION 0x04
#endif
// ASN and LX reuse facility is installed */
#define PSA_FLCFACL0_ASN_LX_REUSE 0x02
// STFLE instruction is available */
#define PSA_FLCFACL0_STFLE 0x01
// Dynamic Address Translation facility is installed */
#define PSA_FLCFACL1_DAT 0x80
// Sense running status */
#define PSA_FLCFACL1_SRS 0x40
// SSKE instruction is installed */
#define PSA_FLCFACL1_SSKE 0x20
// STSI enhancement */
#define PSA_FLCFACL1_CTOP 0x10
// 110524 */
#define PSA_FLCFACL1_QCIF 0x08
// IPTE Range facility is installed */
#define PSA_FLCFACL1_IPTE 0x04
// NQ Key setting */
#define PSA_FLCFACL1_NQKEY 0x02
// APFT Facility is installed
#define PSA_FLCFACL1_APFT 0x01
#define PSA_FLCFACL2_ETF2 0x80
#define PSA_FLCFACL2_CRYA 0x40
#define PSA_FLCFACL2_LONGDISP 0x20
#define PSA_FLCFACL2_LONGDISPHP 0x10
#define PSA_FLCFACL2_HFP_MULSUB 0x08
#define PSA_FLCFACL2_EIMM 0x04
#define PSA_FLCFACL2_ETF3 0x02
#define PSA_FLCFACL2_HFP_UN 0x01

#if MACHINE >= M_ZARCH
// After 0x200 it's available for use by our OS, 32-bit general register save area and
// the control register save area respectively
# define PSA_FLCGRSAV &g_psa.unused10[0]
# define PSA_FLCCRSAV &g_psa.unused10[16 * sizeof(arch_dep::register_t)]
#else
// On S/390 and before we can use lower PSA's!
# error Save areas not implemented yet
#endif

/// @brief Signal Processor codes, according to the z/Architecture principles of operation, page 4-85:
/// http://publibfp.dhe.ibm.com/epubs/pdf/a227832c.pdf
enum sigp_codes {
	/* Sense data */
	S390_SIGP_SENSE = 0x01,
	/* External call */
	S390_SIGP_EXTCALL = 0x02,
	/* Emergency call */
	S390_SIGP_EGCY_CALL = 0x03,
	/* Start */
	S390_SIGP_START = 0x04,
	/* Stop */
	S390_SIGP_STOP = 0x05,
	/* Restart */
	S390_SIGP_RESTART = 0x06,
	/* Stop and store status */
	S390_SIGP_STOP_AND_STORE = 0x09,
	/* Initial CPU reset */
	S390_SIGP_INIT_RESET = 0x0B,
	/* Initial microprogram load */
	S390_SIGP_INIT_MICROPROG = 0x0A,
	/* CPU reset */
	S390_SIGP_CPU_RESET = 0x0C,
	/* Set prefix */
	S390_SIGP_SET_PREFIX = 0x0D,
	/* Store status at address */
	S390_SIGP_STORE_STATUS = 0x0E,
	/* Set operational architecture */
	S390_SIGP_SET_ARCH = 0x12,
	/* z/Arch exclusive signal codes */
#if MACHINE >= M_ZARCH
	/* Conditional emergency */
	S390_SIGP_EGCY_COND = 0x13,
	/* Sense running status */
	S390_SIGP_SENSE_RUN = 0x15,
	/* Set multithreading */
	S390_SIGP_MULTITHREADING = 0x16,
	/* Store additional status at address */
	S390_SIGP_STORE_EXTRA = 0x17,
#endif
};

// See Figure 4.2 of Chapter 4. (page 141) of the z/Arch Principles of Operation
// for a more detailed overview about the structure of the PSW

/// @brief 64-bit PSW for ESA/390 and earlier
struct s390_psw {
	constexpr s390_psw() noexcept
	{

	}

	template<typename T>
	constexpr s390_psw(uint32_t _flags, T *_address) noexcept
		: flags{ _flags },
		address{ static_cast<uint32_t>((uintptr_t)_address & 0xffffffff) }
	{

	}

	uint32_t flags = 0;
	uint32_t address = 0;
} PACKED ALIGNED(8);

/// @brief Extended 128-bit PSW for z/Arch
struct s390_pswe {
	constexpr s390_pswe() noexcept
	{

	}

	template<typename T>
	constexpr s390_pswe(uint32_t _flags, T *_address) noexcept
		: hi_flags{ _flags | PSW_AM64 },
		lo_flags{ PSW_AM31 },
		address{ static_cast<uint64_t>((uintptr_t)_address & 0xffffffffffffffff) }
	{

	}

	uint32_t hi_flags = 0;
	uint32_t lo_flags = 0; /* It's all zero except for the MSB (in S/390 order) */
	uint64_t address = 0;
} PACKED ALIGNED(8);

// Helper function to create a PSW adjusted to the current machine
#if MACHINE >= M_ZARCH
# define s390_default_psw s390_pswe
#else
# define s390_default_psw s390_psw
#endif

/// @brief Permanent storage assign is a memory area, something like the 8086 IVT table
/// but with more fun stuff
struct processor_storage_area {
	s390_psw ipl_psw; // 0x00
	uint8_t ipl_ccw1[24 - 8];
#if MACHINE >= M_ZARCH
	uint8_t unused1[128 - 24];
#else
	s390_psw external_old_psw; // 0x18
	s390_psw svc_old_psw; // 0x20
	s390_psw pc_old_psw; // 0x28
	s390_psw mc_old_psw; // 0x30
	s390_psw io_old_psw; // 0x38
	uint8_t unused1[88 - 64]; // 0x40
	s390_psw external_new_psw; // 0x58
	s390_psw svc_new_psw; // 0x60
	s390_psw pc_new_psw; // 0x68
	s390_psw mc_new_psw; // 0x70
	s390_psw io_new_psw; // 0x78
#endif
	uint32_t extint_param; // 0x80
	uint16_t cpu_addr; // 0x84
	uint16_t extint_code; // 0x86
	uint16_t svcint_ilc; // 0x88
	uint16_t svcint_code; // 0x8A
	uint16_t pcint_ilc; // 0x8C
	uint16_t pcint_code; // 0x8E
	uint32_t data_exc_code; // 0x90 - Data-Exception Code or Vector-Exception Code
	uint16_t monitor_class_num; // 0x94
	uint8_t per_code; // 0x96
	uint8_t atmid; // 0x97
#if MACHINE >= M_ZARCH
	uint64_t per_addr; // 0x98
#else
	uint32_t per_addr; // 0x98
	uint32_t monitor_code; // 0x9C
#endif
	uint8_t exc_acc_id; // 0xA0 - Exception Access ID
	uint8_t per_acc_id; // 0xA1 - PER Access ID
#if MACHINE >= M_ZARCH
	uint8_t op_acc_id; // 0xA2 - Operand Access ID
#else
	uint8_t unused2; // 0xA2
#endif
	uint8_t arch_mode_id; // 0xA3 - SS Arch Mode ID, MC Arch ID
#if MACHINE >= M_ZARCH
	uint8_t unused3[4]; // 0xA4
	uint64_t trans_exc_id; // 0xA8
	uint64_t monitor_code; // 0xB0
#else
	uint8_t unused3[184 - 164]; // 0xA4
#endif
	uint32_t subsystem_id; // 0xB8
	uint32_t ioint_param; // 0xBC
	uint32_t ioint_id; // 0xC0
	uint8_t unused4[4]; // 0xC4
	uint32_t stfl_facility_list; // 0xC8
#if MACHINE >= M_ZARCH
	uint8_t unused5[232 - 204]; // 0xCC
#else
	uint8_t unused5[208 - 204]; // 0xCC
	uint32_t ss_ext_save_area; // 0xD4
	uint64_t ss_cpu_timer; // 0xD8
	uint64_t ss_clock_comp; // 0xE0
#endif
	uint64_t mcint_code; // 0xE8
	uint8_t unused6[4]; // 0xF0
	uint32_t ext_damage_code; // 0xF4
#if MACHINE >= M_ZARCH
	uint64_t fail_storage_addr; // 0xF8
#else
	uint32_t fail_storage_addr; // 0xF8
	uint8_t unused7[4]; // 0xFC
#endif
	uint8_t unused8[288 - 256];
#if MACHINE >= M_ZARCH
	s390_pswe restart_old_psw; // 0x120
	s390_pswe external_old_psw; // 0x130
	s390_pswe svc_old_psw; // 0x140
	s390_pswe pc_old_psw; // 0x150
	s390_pswe mc_old_psw; // 0x160
	s390_pswe io_old_psw; // 0x170
	uint8_t unused9[416 - 384]; // 0x180
	s390_pswe restart_new_psw; // 0x1A0
	s390_pswe external_new_psw; // 0x1B0
	s390_pswe svc_new_psw; // 0x1C0
	s390_pswe pc_new_psw; // 0x1D0
	s390_pswe mc_new_psw; // 0x1E0
	s390_pswe io_new_psw; // 0x1F0
#endif
	uint8_t unused10[4528 - 512]; // 0x200
};

extern processor_storage_area g_psa;

namespace arch_dep {
#if MACHINE >= M_ZARCH
	typedef uint64_t register_t;
#else
	// TODO: This is technically wrong
	typedef uint32_t register_t;
#endif

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
		s390_default_psw psw;

		/// @brief Saves the current context into the local scratch area
		inline void save_scratch_local() const {
			volatile arch_dep::register_t *grsav = reinterpret_cast<volatile arch_dep::register_t *>(PSA_FLCGRSAV);
			for(auto i = 0; i < 16; i++)
				grsav[i] = this->gp_regs[i];
		}

		/// @brief Save context to current thread (obtained from the scratch frame)
		inline void load_scratch_local() {
			volatile arch_dep::register_t *grsav = reinterpret_cast<volatile arch_dep::register_t *>(PSA_FLCGRSAV);
			for(auto i = 0; i < 16; i++)
				this->gp_regs[i] = grsav[i];
		}
	};
}

extern volatile int is_io_fire;
namespace s390_intrin {
	static inline unsigned int cpuid()
	{
		uint16_t cpuid;
		asm volatile("STAP %0\r\n" : "=m"(cpuid) : : "cc");
		return (unsigned int)cpuid;
	}

	static inline int signal(unsigned int cpu_addr, unsigned int param)
	{
		// The s390 spec says that the next odd register number (in short, r1 + 1)
		// shall contain the parameter for the processor signal
		register unsigned int r1 asm("0") = 0; // Status
		register unsigned int r2 asm("1") = param; // Parameter (only lower 32 bits)
		unsigned int order_code = 0;
		int cc = -1;

		asm volatile("SIGP %3, %1, %2\r\n"
			"IPM %0"
			: "+d"(cc)
			: "r"(cpu_addr), "r"(order_code), "r"(r1), "r"(r2)
			: "cc", "memory");
		return cc >> 28;
	}

	/// @brief Wait for an I/O response (overrides the I/O PSW)
	static inline int wait_io()
	{
		size_t timer = 0;
		while(!is_io_fire) {
			timer++;
			if(timer >= 0xffff) return -1;
		}
		is_io_fire = 0;
		return 0;
	}

	static inline int set_timer_delta(intptr_t ms)
	{
		// Must be aligned to a doubleword boundary
		int64_t aligned_ms __attribute__((aligned(8))) = ms;
		asm volatile("SPT %0\r\n" : : "m"(aligned_ms) : );
		return 0;
	}

	static inline void lcreg0(arch_dep::register_t value)
	{
#if MACHINE >= M_ZARCH
		asm volatile("LCTLG 0, 0, %0\r\n" : : "m"(value) : );
#else
		asm volatile("LCTL 0, 0, %0\r\n" : : "m"(value) : );
#endif
	}

	static inline void lcreg1(arch_dep::register_t value)
	{
#if MACHINE >= M_ZARCH
		asm volatile("LCTLG 1, 1, %0\r\n" : : "m"(value) : );
#else
		asm volatile("LCTL 1, 1, %0\r\n" : : "m"(value) : );
#endif
	}

	static inline void lcreg13(arch_dep::register_t value)
	{
#if MACHINE >= M_ZARCH
		asm volatile("LCTLG 13, 13, %0\r\n" : : "m"(value) : );
#else
		asm volatile("LCTL 13, 13, %0\r\n" : : "m"(value) : );
#endif
	}

	static inline void wait()
	{
		s390_default_psw wait_psw = s390_default_psw(PSW_DEFAULT_ARCHMODE | PSW_WAIT_STATE, (void *)nullptr);
#if MACHINE >= M_ZARCH
		asm volatile("LPSWE %0\r\n" : : "m"(wait_psw) : );
#else
		asm volatile("LPSW %0\r\n" : : "m"(wait_psw) : );
#endif
		__builtin_unreachable();
	}

	static inline void set_skey(const void *addr, uint8_t skey)
	{
		// asm volatile("SSKE %0,%1\r\n" : "+r"(skey) : "r"((uintptr_t)addr));
		asm volatile(".insn rrf,0xB22B0000,%0,%1,0b1111,0b1111\r\n" : "+r"(skey) : "r"((uintptr_t)addr));
	}

	/// @brief Enable all the interruptions available for various programs to correctly
	/// operate upon CSS device
	static inline void enable_io()
	{
		s390_default_psw io_default_psw = s390_default_psw(PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT, &&after_enable);
		const arch_dep::register_t new_cr6 = 0xFF000000;
		asm goto("LPSWE %0\r\n" : : "m"(io_default_psw) : : after_enable);
	after_enable:
		// We don't know how the IPL got us here, but we will just
		// turn on everything required just in case.
		s390_intrin::lcreg0(S390_CR0_TIMER_MASK | S390_CR0_AFP_REGISTER);
#if MACHINE >= M_ZARCH
		asm volatile("LCTLG 6, 6, %0\r\n" : : "m"(new_cr6) : );
#else
		asm volatile("LCTL 6, 6, %0\r\n" : : "m"(new_cr6) : );
#endif
	}

	static inline void disable_int()
	{
		s390_intrin::lcreg0(0x00);
	}

	static inline void enable_int()
	{
		s390_intrin::lcreg0(S390_CR0_TIMER_MASK | S390_CR0_AFP_REGISTER);
	}
}

#endif
