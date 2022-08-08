// scheduler.c
//
// Implements the scheduling algorithms for the scheduler

#include <timeshr.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <arch/asm.hxx>
#include <arch/handlers.hxx>
#include <errcode.hxx>

static storage::global_wrapper<timeshare::table> g_scheduler;

void timeshare::init()
{
	auto *sys_job = timeshare::job::create(*"SYSMAIN", 1, static_cast<timeshare::job::flag>(timeshare::job::REAL | timeshare::job::BITS_64), 65535);
	auto *kern_task = timeshare::task::create(*sys_job, *"KERNEL");
	auto *kern_thread = timeshare::thread::create(*sys_job, *kern_task, 8192);
	kern_thread->set_pc(nullptr, true);
	// Allow scheduling and un-sleep
	sys_job->flags = static_cast<timeshare::job::flag>(sys_job->flags & (~timeshare::job::SLEEP));
	io_svc(SVC_SCHED_YIELD, 0, 0, 0);
}

timeshare::job *timeshare::job::create(const char& name, signed char priority, timeshare::job::flag _flags, size_t max_mem)
{
	timeshare::disable();
	auto *job = g_scheduler->jobs.insert();
	if(job == nullptr) {
		timeshare::enable();
		return nullptr;
	}
	*job = timeshare::job{};
	job->flags = static_cast<timeshare::job::flag>(_flags | timeshare::job::flag::SLEEP);
	timeshare::enable();

	// Initialize the address space for this job
	if(job->flags & timeshare::job::VIRTUAL) {
		job->aspace = virtual_storage::address_space::create();
		if(job->aspace == nullptr) return nullptr;
	} else {
		job->aspace = nullptr;
	}
	storage_string::copy(job->name, &name);

	job->priority = priority;
	job->max_mem = max_mem;
	job->user_id = usersys::user::get_current();
	return job;
}

int timeshare::job::remove(const timeshare::task& task)
{
	for(size_t i = 0; i < this->tasks.size(); i++) {
		if(&this->tasks[i] == &task) {
			this->tasks.remove(i);
			if(this->current_task >= this->tasks.size())
				this->current_task = 0;
			return 0;
		}
	}
	// No tasks found
	return error::RESOURCE_EXPECTED;
}

timeshare::task *timeshare::task::create(timeshare::job& job, const char& name)
{
	auto *task = job.tasks.insert();
	if(task == nullptr) return nullptr;
	*task = timeshare::task{};
	storage_string::copy(task->name, &name);
	return task;
}

int timeshare::task::remove(timeshare::thread& thread)
{
	for(size_t i = 0; i < this->threads.size(); i++) {
		if(&this->threads[i] == &thread) {
			if(thread.stack != nullptr)
				storage::free(thread.stack);
			this->threads.remove(i);
			if(this->current_thread >= this->threads.size())
				this->current_thread = 0;
			return 0;
		}
	}
	// No threads found
	return error::RESOURCE_EXPECTED;
}

timeshare::thread *timeshare::thread::create(timeshare::job& job, timeshare::task& task, size_t stack_size)
{
	auto *thread = task.threads.insert();
	if(thread == nullptr) return nullptr;
	*thread = timeshare::thread{};

	if(stack_size) {
		// Allocate stack for this thread (the stack is local to each thread)
		thread->stack = storage::allocz(stack_size, virtual_storage::page_align);
		if(thread->stack == nullptr) return nullptr;

		// R15 is used as a stack pointer, now we have to setup a few things up
#if defined TARGET_S390
		thread->context.r15 = (uintptr_t)thread->stack + (stack_size - STACK_FRAME_SIZE);
		debug_printf("Thread.Stack=%p", (uintptr_t)thread->context.r15);
#elif defined TARGET_X86
		thread->context.rsp = (uintptr_t)thread->stack + (stack_size - STACK_FRAME_SIZE);
		debug_printf("Thread.Stack=%p", (uintptr_t)thread->context.rsp);
#elif defined TARGET_RISCV
		thread->context.sp = (uintptr_t)thread->stack + (stack_size - STACK_FRAME_SIZE);
		debug_printf("Thread.Stack=%p", (uintptr_t)thread->context.sp);
#else
#   error No stack defined
#endif
#if 0
		// stack+76 should point to stack+180 (because this would be the next frame!)
		*((uint32_t *)(&((uint8_t *)thread->stack)[76])) = (uint32_t)(&((uint8_t *)thread->stack)[180]);
		*((uint32_t *)(&((uint8_t *)thread->stack)[8])) = (uint32_t)(&((uint8_t *)thread->stack)[180]);
		// Set backchain to 0 for stack unwinding
		*((uint32_t *)(&((uint8_t *)thread->stack)[4])) = 0;
		*((uint32_t *)(&((uint8_t *)thread->stack)[8])) = 0;
#endif
		if(job.aspace != nullptr) {
			/// @todo Make sure the mapping does not clash with the job ASPACE
			job.map_range(thread->stack, thread->stack, 0, stack_size);
		}
	} else {
		// Kernel will manage the stack, so we will do nothing!
	}
	return thread;
}

void timeshare::thread::set_pc(void *pc, bool privileged)
{
#ifdef TARGET_S390
	unsigned int flags = PSW_DEFAULT_ARCHMODE | PSW_ENABLE_MCI | PSW_IO_INT | PSW_EXTERNAL_INT;
	if(!privileged)
		flags |= PSW_DAT | PSW_PROBLEM_STATE;

	this->context.psw = s390_default_psw(flags, pc);
	debug_printf("Thread.Psw.Address=%p", (uintptr_t)this->context.psw.address);
#else
	this->context.pc = reinterpret_cast<uintptr_t>(pc);
#endif
}

timeshare::job *timeshare::get_current_job()
{
	return &g_scheduler->jobs[g_scheduler->current_job];
}

timeshare::job::job_t timeshare::get_current_jobid()
{
	return static_cast<timeshare::job::job_t>(g_scheduler->current_job);
}

void timeshare::next(timeshare::job **_job, timeshare::task **_task, timeshare::thread **_old_thread, timeshare::thread **_new_thread)
{
	// Obtain the old thread
	auto *job = &g_scheduler->jobs[g_scheduler->current_job];
	auto *task = &job->tasks[job->current_task];
	debug_assert(!(task->current_thread > task->threads.size()));
	auto *old_thread = &task->threads[task->current_thread];
	debug_printf("OLD:JId=%i,TId=%i,ThId=%i,NThreads=%u", (int)g_scheduler->current_job, (int)job->current_task, (int)task->current_thread, task->threads.size());
	
	// Job - Now obtain a new thread
	while(1) {
		debug_assert(!(g_scheduler->current_job > g_scheduler->jobs.size()));
		g_scheduler->current_job++;
		if(g_scheduler->current_job >= g_scheduler->jobs.size())
			g_scheduler->current_job = 0;
		job = &g_scheduler->jobs[g_scheduler->current_job];
		if(job->flags & timeshare::job::SLEEP) {
			debug_printf("Skipping sleeping job");
			continue;
		}

		// Task
		if(job->tasks.empty()) {
			debug_printf("Skipping empty tasklist");
			continue;
		}
		debug_assert(!(job->current_task > job->tasks.size()));
		job->current_task++;
		if(job->current_task >= job->tasks.size())
			job->current_task = 0;
		task = &job->tasks[job->current_task];

		// Thread
		if(task->threads.empty()) {
			debug_printf("Skipping empty threadlist");
			continue;
		}
		auto starting_thread = task->current_thread;
		bool tl_wrapped = false;
		while(1) {
			task->current_thread++;
			if(task->current_thread >= task->threads.size())
				task->current_thread = 0;
			// Once we have passed thru were we started, break, no available threads are for us
			if(starting_thread == task->current_thread) {
				if(tl_wrapped) break;
				tl_wrapped = true;
			}
			/// @todo: Add conditions for threads
			if(0) continue;
			debug_assert(!(task->current_thread > task->threads.size()));
			auto *new_thread = &task->threads[task->current_thread];
			debug_printf("NEW:JId=%i,TId=%i,ThId=%i,NThreads=%u", (int)g_scheduler->current_job, (int)job->current_task, (int)task->current_thread, task->threads.size());
			*_new_thread = new_thread;
			goto end;
		}
		debug_printf("\x01\x22 find active thread %u(%u)", task->threads.size(), task->current_thread);
	}
end:
	*_job = job;
	*_task = task;
	*_old_thread = old_thread;
}

#ifdef TARGET_S390
namespace timeshare {
	static inline void switch_context(void *_old_thread, void *_new_thread, s390_default_psw *old_psw);
}
static inline void timeshare::switch_context(void *_old_thread, void *_new_thread, s390_default_psw *old_psw)
{
	auto *old_thread = reinterpret_cast<timeshare::thread *>(_old_thread);
	old_thread->context.load_scratch_local();
	// Save the OLD PSW into the older thread
	debug_printf("OldOld address %p", old_thread->context.psw.address);
	old_thread->context.psw = *old_psw;
	debug_printf("NewOld address %p", old_thread->context.psw.address);

	auto *new_thread = reinterpret_cast<timeshare::thread *>(_new_thread);
	// Set the new reload address
	debug_printf("OldNew address %p", new_thread->context.psw.address);
	*old_psw = new_thread->context.psw;
	debug_printf("NewNew address %p", new_thread->context.psw.address);
	new_thread->context.save_scratch_local();
}
#endif

void timeshare::schedule()
{
	timeshare::job *job;
	timeshare::task *task;
	timeshare::thread *old_thread, *new_thread;

	timeshare::next(&job, &task, &old_thread, &new_thread);
#ifdef TARGET_S390
	timeshare::switch_context(old_thread, new_thread, &g_psa.external_old_psw);
#endif
	if(job->aspace != nullptr) {
		// Set the new ASPACE on CR1 of the current job
		// s390_intrin::lcreg0(0x00800000 | 0x00100000);
		job->aspace->set_primary();
		job->aspace->flush_tlb();
	}
}
