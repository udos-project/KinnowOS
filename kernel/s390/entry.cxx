// ESA 390 boot code, this entry code is independent from z/Arch or 360
// it should run on whatever machine flawlessly

#include <s390/asm.hxx>
#include <s390/handlers.hxx>
#include <s390/css.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <timeshr.hxx>

// The first function called (kinit) uses a prologue and epilogue to perform the stack stuff
// however this uses an additional 72+REGAREA bytes which overwrites important data, sometimes

extern uint8_t bss_start[], bss_end[];
extern void main() noexcept;
void init_smp() noexcept;

struct facl_check_elem {
	const char *name;
	char index; /* Index on the FACL[] array */
	char and_mask; /* The AND mask used to deduce if the element is active or not */
};

constinit static const facl_check_elem facl_check_list[25] = {
	{ "N3\x01\x18", 0, PSA_FLCFACL0_N3 },
	{ "z/Arch Install", 0, PSA_FLCFACL0_ZARCH_INSTALL },
	{ "z/Arch Active", 0, PSA_FLCFACL0_ZARCH_ACTIVE },
#if MACHINE >= M_ZARCH
	{ "IDTE\x01\x18", 0, PSA_FLCFACL0_IDTE },
	{ "IDTE Clear Segment", 0, PSA_FLCFACL0_IDTE_CLEAR_SEGMENT },
	{ "IDTE Clear Region", 0, PSA_FLCFACL0_IDTE_CLEAR_REGION },
#endif
	{ "ASN and LX Reuse\x01\x18", 0, PSA_FLCFACL0_ASN_LX_REUSE },
	{ "STFLE\x01\x18", 0, PSA_FLCFACL0_STFLE },
	{ "DAT\x01\x18", 1, PSA_FLCFACL1_DAT },
	{ "Sense Running Status", 1, PSA_FLCFACL1_SRS },
	{ "SSKE Instruction Installed", 1, PSA_FLCFACL1_SSKE },
	{ "STSI Enhancement", 1, PSA_FLCFACL1_CTOP },
#if MACHINE >= M_ZARCH
	{ "110524\x01\x18", 1, PSA_FLCFACL1_QCIF },
	{ "IPTE\x01\x18", 1, PSA_FLCFACL1_IPTE },
	{ "NQ-Key Setting\x01\x18", 1, PSA_FLCFACL1_NQKEY },
	{ "APFT\x01\x18", 1, PSA_FLCFACL1_APFT },
#endif
	{ "\x01\x19\x01\x03 2\x01\x18", 2, PSA_FLCFACL2_ETF2 },
	{ "\x01\x1A Assist\x01\x18", 2, PSA_FLCFACL2_CRYA },
	{ "Long\x01\x1B\x01\x18", 2, PSA_FLCFACL2_LONGDISP },
	{ "Long\x01\x1B (High Performance)\x01\x18", 2, PSA_FLCFACL2_LONGDISPHP },
	{ "HFP Multiply-Subtraction", 2, PSA_FLCFACL2_HFP_MULSUB },
	{ "\x01\x19 Immediate\x01\x18", 2, PSA_FLCFACL2_EIMM },
	{ "\x01\x19\x01\x03 3\x01\x18", 2, PSA_FLCFACL2_ETF3 },
	{ "HFP\x01\x08 Extension", 2, PSA_FLCFACL2_HFP_UN },
	{ nullptr, 0, 0 }
};

SECTION(".head.bss") ALIGNED(8) uint8_t int_stack[2048];
SECTION(".head.bss") ALIGNED(8) uint8_t stack[2048];

//
// Code
//
void asc_svc_handler();
void asc_io_handler();
void asc_pc_handler();
void asc_mc_handler();
void asc_external_handler();
void asm_svc_handler() noexcept;
void asm_io_handler() noexcept;
void asm_pc_handler() noexcept;
void asm_mc_handler() noexcept;
void asm_ext_handler() noexcept;

ALIGNED(4) NAKED_FUNC void asm_svc_handler() noexcept
{
	asm volatile(
		"STMG %%r0,%%r15,%0\r\n" // 0 - save area
		"LARL %%r15,%1\r\n" // 1 - stack
		"BRASL %%r14,%2\r\n" // 2 - handler
		"LMG %%r0,%%r15,%0\r\n"
		"LPSWE %3\r\n" // 3 - old psw
		:
		: "i"((uintptr_t)PSA_FLCGRSAV), "i"(STACK_TOP(int_stack)), "i"((uintptr_t)&asc_svc_handler), "i"((uintptr_t)&g_psa.svc_old_psw)
		:
	);
}

ALIGNED(4) NAKED_FUNC void asm_io_handler() noexcept
{
	asm volatile(
		"STMG %%r0,%%r15,%0\r\n" // 0 - save area
		"LARL %%r15,%1\r\n" // 1 - stack
		"BRASL %%r14,%2\r\n" // 2 - handler
		"LMG %%r0,%%r15,%0\r\n"
		"LPSWE %3\r\n" // 3 - old psw
		:
		: "i"((uintptr_t)PSA_FLCGRSAV), "i"(STACK_TOP(int_stack)), "i"((uintptr_t)&asc_io_handler), "i"((uintptr_t)&g_psa.io_old_psw)
		:
	);
}

ALIGNED(4) NAKED_FUNC void asm_pc_handler() noexcept
{
	asm volatile(
		"STMG %%r0,%%r15,%0\r\n" // 0 - save area
		"LARL %%r15,%1\r\n" // 1 - stack
		"BRASL %%r14,%2\r\n" // 2 - handler
		"LMG %%r0,%%r15,%0\r\n"
		"LPSWE %3\r\n" // 3 - old psw
		:
		: "i"((uintptr_t)PSA_FLCGRSAV), "i"(STACK_TOP(int_stack)), "i"((uintptr_t)&asc_pc_handler), "i"((uintptr_t)&g_psa.pc_old_psw)
		:
	);
}

ALIGNED(4) NAKED_FUNC void asm_mc_handler() noexcept
{
	asm volatile(
		"STMG %%r0,%%r15,%0\r\n" // 0 - save area
		"LARL %%r15,%1\r\n" // 1 - stack
		"BRASL %%r14,%2\r\n" // 2 - handler
		"LMG %%r0,%%r15,%0\r\n"
		"LPSWE %3\r\n" // 3 - old psw
		:
		: "i"((uintptr_t)PSA_FLCGRSAV), "i"(STACK_TOP(int_stack)), "i"((uintptr_t)&asc_mc_handler), "i"((uintptr_t)&g_psa.mc_old_psw)
		:
	);
}

ALIGNED(4) NAKED_FUNC void asm_ext_handler() noexcept
{
	asm volatile(
		"STMG %%r0,%%r15,%0\r\n" // 0 - save area
		"LARL %%r15,%1\r\n" // 1 - stack
		"BRASL %%r14,%2\r\n" // 2 - handler
		"LMG %%r0,%%r15,%0\r\n"
		"LPSWE %3\r\n" // 3 - old psw
		:
		: "i"((uintptr_t)PSA_FLCGRSAV), "i"(STACK_TOP(int_stack)), "i"((uintptr_t)&asc_external_handler), "i"((uintptr_t)&g_psa.external_old_psw)
		:
	);
}

// Static initialization does not allow some tomfooleries.
static s390_default_psw svc_psw = s390_default_psw(PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI, &asm_svc_handler);
static s390_default_psw pc_psw = s390_default_psw(PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI, &asm_pc_handler);
// Disallowing external interruptions on the external interruption handler is required
// because otherwise the CPU timer will continue spamming external interruptions causing an infinite
// hang, and we don't want that!
static s390_default_psw ext_psw = s390_default_psw(PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI, &asm_ext_handler);
static s390_default_psw mc_psw = s390_default_psw(PSW_DEFAULT_ARCHMODE, &asm_mc_handler);
static s390_default_psw io_psw = s390_default_psw(PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI, &asm_io_handler);

ALIGNED(4) void init() noexcept
{
	/* Compile-time assertions (hopefully) */
#if MACHINE >= M_ZARCH
	static_assert(sizeof(uintptr_t) == 8);
#else
	static_assert(sizeof(uintptr_t) == 4);
#endif
	s390_intrin::lcreg0(S390_CR0_AFP_REGISTER);
	
	// Register the interrupt handler PSWs so they are used when something happens and we
	// can handle that accordingly
	kprintf("Setting interrupts\r\n");
	g_psa.svc_new_psw = svc_psw;
	g_psa.pc_new_psw = pc_psw;
	g_psa.external_new_psw = ext_psw;
	g_psa.mc_new_psw = mc_psw;
	g_psa.io_new_psw = io_psw;

	// Read FLCCAW schid
	css::schid ipl_schid = *(css::schid *)g_psa.ipl_ccw1;

	// Print statments from this point onwards should go to a console or a device
	// Recopile some information about the system.
	debug_printf("CPU#%uIPL.schid=%i:%i", (size_t)s390_intrin::cpuid(), (int)ipl_schid.num, (int)ipl_schid.id);
	{
		volatile const uint8_t *facl = (volatile const uint8_t *)&g_psa.stfl_facility_list;
#if MACHINE >= M_ZARCH
		register uintptr_t r0 asm("0") = 2048;
		// stfle 0(r1)
		asm volatile(".insn s,0xB2B00000,0(%0)" : : "r"(r0) : "memory", "cc");
#else
		register uintptr_t r0 asm("0") = 0;
		asm volatile(
			"STFL 0(%0)" /* stfl 0(r1) */ : : "r"(r0) : "memory", "cc");
#endif
		debug_printf("FLCFACL: %p (%x:%x:%x:%x:%x:%x)", facl, (unsigned int)facl[0], (unsigned int)facl[1], (unsigned int)facl[2], (unsigned int)facl[3], (unsigned int)facl[4], (unsigned int)facl[5], (unsigned int)facl[6]);
		auto *elem = &facl_check_list[0];
		while(elem->name != nullptr) {
			if(facl[(unsigned int)elem->index] & elem->and_mask) {
				debug_printf("%s", elem->name);
			}
			elem++;
		}
		debug_assert(elem->name == nullptr && elem->index == 0 && elem->and_mask == 0);
	}

#if 0
	// Protect areas of the kernel.
	{
		extern uint8_t text_start[], text_end[];
		extern uint8_t rodata_start[], rodata_end[];
		extern uint8_t data_start[], data_end[];
		extern uint8_t bss_start[], bss_end[];
		uintptr_t ptr;

		ptr = (uintptr_t)&text_start;
		while(ptr < (uintptr_t)&text_end) {
			/* Disallow store */
			s390_intrin::set_skey((void *)ptr, 0xFF);
			ptr += 4096;
		}

		/*
		*((volatile uint8_t *)((uintptr_t)&text_start + 0x45a)) = 'A';
		while(1);
		*/
	}
#endif
	main();
}

extern "C" {
	SECTION(".head.text") NAKED_FUNC void start() noexcept
	{
		// Clear the .bss section
		extern uint8_t bss_start[], bss_end[];
		// Hopefully it gets synthetized into a register!
		for(register auto ptr = reinterpret_cast<uintptr_t>(bss_start); ptr < reinterpret_cast<uintptr_t>(bss_end); ptr++)
			*(reinterpret_cast<uint8_t *>(ptr)) = 0x00;

		asm volatile(
			"LARL %%r15,%0\r\n" // R15=Stack pointer
			"BRASL %%r14,%1\r\n" // R14=Return Address, and jump to init
			:
			: "i"(STACK_TOP(stack)), "i"((uintptr_t)&init)
			:
		);
	}
}

ALIGNED(4) NAKED_FUNC void smp_cpu_stub()
{
	asm volatile("J .");
}

static void spooler_thread_fn() {
	while(1) {
		/// @todo This is required because mutexes deadlock because the SVC can't switch
		/// tasks due to the fact that we don't support nested interrupts
		s390_intrin::disable_int();
		size_t rem_reqs = 0;
		while((rem_reqs = css::request_perform()) != 0) {
			// ...
			debug_printf("rem_reqs=%u", rem_reqs);
		}
		s390_intrin::enable_int();
		io_svc(SVC_SCHED_YIELD, 0, 0, 0);
	}
}

void init_smp() noexcept {
	kprintf("Waking up the other CPUs\r\n");
	// The INITIAL RESET signal will make the CPU start at address 0, so we have to copy our little stub
	// so it does not immediately recognize a PC exception.
	storage::copy((void *)0x00, (void *)&smp_cpu_stub, 8);
	for(size_t i = 0; i < MAX_CPUS; i++) {
		int r;
		if(i == s390_intrin::cpuid()) continue;

		r = s390_intrin::signal((unsigned int)i, S390_SIGP_INIT_RESET);
		if(r == 3) continue;
		debug_printf("\x01\x21 CPU#%usigp=%i", i, r);
	}

	// Multitasking engine
	kprintf("\x01\x09 the scheduler\r\n");
	timeshare::init();
	auto *sys_job = timeshare::get_current_job();
	auto *spooler_task = timeshare::task::create(*sys_job, *"SPOOLER");
	auto *spooler_thread = timeshare::thread::create(*sys_job, *spooler_task, 8192);
	spooler_thread->set_pc((void *)&spooler_thread_fn, true);
	// Allow scheduling and un-sleep
	sys_job->flags = static_cast<timeshare::job::flag>(sys_job->flags & (~timeshare::job::SLEEP));

	// Kickstart the scheduler for the device spooler to start running.
	s390_intrin::set_timer_delta(0xF420000);
	s390_intrin::enable_io();
}
