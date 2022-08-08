#include <s390/handlers.hxx>
#include <types.hxx>
#include <vdisk.hxx>
#include <storage.hxx>
#include <timeshr.hxx>
#include <printf.hxx>
#include <s390/asm.hxx>
#include <user.hxx>
#include <s390/css.hxx>

struct s390_gcc_call_stack {
	uint32_t backchain;
	uint32_t end_of_stack;
	uint32_t glue[2];
	uint32_t scratch[2];
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t r13;
	uint32_t r14;
	uint32_t r15;
	uint32_t f4;
	uint32_t f6;
	uint8_t undefined[16];
	/* After this comes the local variables, alloca, etc */
} PACKED;

struct s390x_gcc_call_stack {
	uint64_t backchain;
	uint64_t end_of_stack;
	uint64_t glue[2];
	uint64_t scratch[2];
	uint64_t r6;
	uint64_t r7;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t f4;
	uint64_t f6;
} PACKED;

void asc_svc_handler();
void asc_io_handler();
void asc_pc_handler();
void asc_mc_handler();
void asc_external_handler();

static inline void debug_frame_print(const volatile arch_dep::processor_context& frame)
{
	debug_printf("GR00: %p GR01: %p GR02: %p GR03: %p", (void *)frame.r0, (void *)frame.r1, (void *)frame.r2, (void *)frame.r3);
	debug_printf("GR04: %p GR05: %p GR06: %p GR07: %p", (void *)frame.r4, (void *)frame.r5, (void *)frame.r6, (void *)frame.r7);
	debug_printf("GR08: %p GR09: %p GR10: %p GR11: %p", (void *)frame.r8, (void *)frame.r9, (void *)frame.r10, (void *)frame.r11);
	debug_printf("GR12: %p GR13: %p GR14: %p GR15: %p", (void *)frame.r12, (void *)frame.r13, (void *)frame.r14, (void *)frame.r15);
	arch_dep::register_t cr[16];
#if MACHINE >= M_ZARCH
	asm volatile("STCTG 0,15,%0" : "=m"(cr) : );
#else
	asm volatile("STCTL 0,15,%0" : "=m"(cr) : );
#endif
	debug_printf("CR00: %p CR01: %p CR02: %p CR03: %p", (void *)cr[0], (void *)cr[1], (void *)cr[2], (void *)cr[3]);
	debug_printf("CR04: %p CR05: %p CR06: %p CR07: %p", (void *)cr[4], (void *)cr[5], (void *)cr[6], (void *)cr[7]);
	debug_printf("CR08: %p CR09: %p CR10: %p CR11: %p", (void *)cr[8], (void *)cr[9], (void *)cr[10], (void *)cr[11]);
	debug_printf("CR12: %p CR13: %p CR14: %p CR15: %p", (void *)cr[12], (void *)cr[13], (void *)cr[14], (void *)cr[15]);

	// Stack frame dump
	debug_printf("Stack frame:");
	const auto *job = timeshare::get_current_job();
	const auto *task = &job->tasks[job->current_task];
	const auto *thread = &task->threads[task->current_thread];

	size_t i = 0;
	const auto *stack_frame = reinterpret_cast<const s390x_gcc_call_stack *>(job->virtual_to_real(reinterpret_cast<void *>(frame.r15)));
	while(stack_frame != nullptr) {
		// s390: 56, z/arch: 112
		if(i >= 20) break;

		// Obtain the frame symbol
		const auto *framesym = job->get_symbol((const void *)job->virtual_to_real((void *)stack_frame->r14));
		const auto return_addr = stack_frame->r14;
		if(framesym != nullptr) {
			debug_printf("FRAME(%p)#%u%p -> %s+%u", stack_frame, i, return_addr, framesym->name, static_cast<size_t>(return_addr - reinterpret_cast<uintptr_t>(framesym->address)));
		} else {
			debug_printf("FRAME(%p)#%u%p -> ???", stack_frame, i, return_addr);
		}

		stack_frame = reinterpret_cast<decltype(stack_frame)>(static_cast<uintptr_t>(stack_frame->backchain)); // Go to the next backchain
		i++;
	}
}

#include <service.hxx>
void asc_svc_handler()
{
	volatile auto& frame = *reinterpret_cast<volatile arch_dep::processor_context *>(PSA_FLCGRSAV);
	const uint16_t code = g_psa.svcint_code;
#if defined DEBUG
	auto *old_psw = &g_psa.svc_old_psw;
	const uint16_t ilc = g_psa.svcint_ilc;
	debug_printf("SVC call (id %i) (len=%i) from %p", (int)code, (int)ilc, (uintptr_t)old_psw->address);
	debug_frame_print(frame);
#endif
	// uDOS native applications
	frame.r4 = service::common(code, frame.r1, frame.r2, frame.r3, frame.r4);
}

constinit static const char *pc_code_names[] = {
	"Unknown", /* 0x0000 */
	"\x01\x01", /* 0x0001 */
	"Privileged\x01\x01", /* 0x0002 */
	"Execute", /* 0x0003 */
	"Protection", /* 0x0004 */
	"Addressing", /* 0x0005 */
	"\x01\x02", /* 0x0006 */
	"Data", /* 0x0007 */
	"FP oflow", /* 0x0008 */
	"FP div", /* 0x0009 */
	"Decimal oflow", /* 0x000A */
	"Decimal div", /* 0x000B */
	"HFP oflow", /* 0x000C */
	"HFP uflow", /* 0x000D */
	"HFP significance", /* 0x000E */
	"HFP div", /* 0x000F */
	"Segment\x01\x03", /* 0x0010 */
	"Page\x01\x03", /* 0x0011 */
	"\x01\x03\x01\x02", /* 0x0012 */
	"Special\x01\x02", /* 0x0013 */
#if MACHINE >= M_ZARCH
	"", /* 0x0014 */
	"Operand", /* 0x0015 */
	"Trace Table", /* 0x0016 */
	"ASN\x01\x03", /* 0x0017 */
	"\x01\x07 Constraint", /* 0x0018 */
	"VX\x01\x01", // 0x0019
	"", // 0x001A
	"VX processing", // 0x001B
	"Space switch event", // 0x001C
	"Square root", // 0x001D
	"\x01\x08 Operand", // 0x001E,
	"PC\x01\x03\x01\x02", // 0x001F
	"AFX\x01\x03", // 0x0020
	"ASX\x01\x03", // 0x0021
	"LX\x01\x03", // 0x0022
	"EX\x01\x03", // 0x0023
	"Primary\x01\x04", // 0x0024
	"Secondary\x01\x04", // 0x0025
	"LFX\x01\x03", // 0x0026
	"LSX\x01\x03", // 0x0027
	"ALET\x01\x02", // 0x0028
	"ALEN\x01\x03", // 0x0029
	"ALE\x01\x05", // 0x002A
	"ASTE valid", // 0x002B
	"ASTE\x01\x05", // 0x002C
	"Extended\x01\x04", // 0x002D
	"LSTE\x01\x05", // 0x002E
	"ASTE inst", // 0x002F
	"\x01\x06 full", // 0x0030
	"\x01\x06 empty", // 0x0031
	"\x01\x06\x01\x02", // 0x0032
	"\x01\x06 type", // 0x0033
	"\x01\x06 operation", // 0x0034
	"", // 0x0035
	"", // 0x0036
	"", // 0x0037
	"ASCE type", // 0x0038
	"RT1\x01\x03", // 0x0039
	"RT2\x01\x03", // 0x003A
	"RT3\x01\x03", // 0x003B
	"", // 0x003C
	"", // 0x003D
	"", // 0x003E
	"", // 0x003F
	"Monitor", // 0x0040
#endif
};

static base::mutex pc_handler_lock;
void asc_pc_handler()
{
	base::scoped_mutex lock(pc_handler_lock);
	arch_dep::processor_context& frame = *((arch_dep::processor_context *)PSA_FLCGRSAV);
	while(pc_handler_lock.try_lock()) {}
	auto *old_pc_psw = &g_psa.pc_old_psw;
	uint16_t code = g_psa.pcint_code;
	code &= ~(0x200 | 0x80); // According to the POP, the exceptions get 0x200 bitflag and 0x80 bitflags when PER is used
	const char *codename = (code < sizeof(pc_code_names) / sizeof(pc_code_names[0])) ? pc_code_names[code] : pc_code_names[0];
	auto* job = timeshare::get_current_job();
	debug_printf("PC %s (code=%x) occoured at %p,Job=%uTask=%u codename", code, old_pc_psw->address, timeshare::get_current_jobid(), job->current_task);
#if defined DEBUG
	debug_frame_print(frame);
#endif
	job->remove(job->tasks[job->current_task]); // Kill faulting task
	timeshare::schedule(); // Go to the next task instead
	s390_intrin::set_timer_delta(0xF420000);
}

void asc_mc_handler()
{
	debug_printf("*** Machine check ***");
}

void asc_external_handler()
{
	debug_printf("*** External ***");
	timeshare::schedule();
	s390_intrin::set_timer_delta(0xF420000);
}

volatile int is_io_fire = 0;
void asc_io_handler()
{
	debug_printf("*** I/O ***");
	is_io_fire = 1;
}
