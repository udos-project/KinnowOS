#ifndef TIMESHARE_HXX
#define TIMESHARE_HXX

#include <types.hxx>
#include <storage.hxx>
#include <arch/asm.hxx>
#include <arch/handlers.hxx>
#include <arch/virtual.hxx>
#include <mutex.hxx>
#include <user.hxx>
#include <abi_bits.h>

namespace timeshare {
	class job;
	class task;
	class thread;
	class table;

	enum thread_flags {
		ACTIVE = 0x00,
		SLEEP = 0x01,
	};

	struct thread {
		using thread_t = unsigned short;
		static timeshare::thread *create(timeshare::job& job, timeshare::task& task, size_t stack_size);
		void set_pc(void *pc, bool privileged);

		void *stack;
		arch_dep::processor_context context;
		int status;
	};

	struct task {
		using task_t = unsigned short;
		static timeshare::task *create(timeshare::job& job, const char& name);
		int remove(timeshare::thread& thread);

		storage::dynamic_list<timeshare::thread> threads;
		size_t current_thread;
		char name[8];
		program_data_block pdb;
	};

	struct job {
		using job_t = unsigned short;
		enum flag {
			// ASPACE mode
			REAL = 0x00,
			VIRTUAL = 0x01,
			// Bitage
			BITS_64 = 0x02,
			BITS_31 = 0x04,
			BITS_24 = 0x08,
			// Operation
			SLEEP = 0x10,
		};

		static timeshare::job *create(const char& name, signed char priority, timeshare::job::flag flags, size_t max_mem);
		int remove(const timeshare::task& task);

		inline void *virtual_to_real(void *vaddr) const {
			if(this->aspace != nullptr)
				return this->aspace->virtual_to_real(reinterpret_cast<void *>(vaddr));
			return reinterpret_cast<void *>(vaddr);
		}

		inline void map_range(void *vaddr, void *paddr, int _flags, size_t len) {
			if(this->aspace != nullptr)
				this->aspace->map_range(vaddr, paddr, _flags, len);
		}

		inline const storage::symbol *get_symbol(const void *ptr) const {
			for(size_t i = 0; i < this->symbols.size(); i++) {
				const auto& symbol = this->symbols[i];
				// Check it's inside the symbol
				if(reinterpret_cast<uintptr_t>(ptr) >= reinterpret_cast<uintptr_t>(symbol.address) && reinterpret_cast<uintptr_t>(ptr) < reinterpret_cast<uintptr_t>(symbol.address) + static_cast<uintptr_t>(symbol.size))
					return &symbol;
			}
			return nullptr;
		}

		timeshare::job::flag flags = timeshare::job::SLEEP;
		storage::dynamic_list<timeshare::task> tasks;
		size_t current_task;
		signed char priority; // Priority of the job
		size_t max_mem; // Max memory to be used by job
		// Address space of job
		/// @todo Make it so the aspace is not a pointer but embedded directly onto the job struct
		virtual_storage::address_space *aspace;
		char name[8];
		storage::dynamic_list<storage::symbol> symbols;
		usersys::user::id user_id;
	};

	struct table {
		constexpr table() = default;
		~table() = default;

		storage::dynamic_list<timeshare::job> jobs;
		size_t current_job = 0;
	};

	void init();
	timeshare::job *get_current_job();
	timeshare::job::job_t get_current_jobid();
	void next(timeshare::job **_job, timeshare::task **_task, timeshare::thread **_old_thread, timeshare::thread **_new_thread);
	void schedule();

	inline void enable()
	{
#ifdef TARGET_S390
		s390_intrin::enable_int();
#endif
	}

	inline void disable()
	{
#ifdef TARGET_S390
		s390_intrin::disable_int();
#endif
	}
}

#endif
