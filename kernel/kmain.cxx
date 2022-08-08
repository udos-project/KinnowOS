#include <storage.hxx>
#include <mutex.hxx>
#include <user.hxx>
#include <vdisk.hxx>
#include <printf.hxx>
#include <zdsfs.hxx>
#include <elf.hxx>
#include <timeshr.hxx>
#include <uart.hxx>
#include <boot.hxx>
#include <fdt.hxx>
#include <pci.hxx>

#ifdef TARGET_S390
#   include <s390/css.hxx>
#   include <s390/x3270.hxx>
#   include <s390/hercns.hxx>
#   include <s390/dasd.hxx>
#endif

void csup_init();
void init_smp() noexcept;

extern "C" {
#define ATEXIT_MAX_FUNCS 128
	typedef unsigned uarch_t;
	
	struct atexit_func_entry_t {
		void (*destructor_func)(void *);
		void *obj_ptr;
		void *dso_handle;
	};

	atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
	uarch_t __atexit_func_count = 0;
	void *__dso_handle = 0;

	int __cxa_atexit(void (*destructor)(void *), void *arg, void *dso);
	void __cxa_finalize(void *f);

	int __cxa_atexit(void (*destructor)(void *), void *arg, void *dso)
	{
		if (__atexit_func_count >= ATEXIT_MAX_FUNCS) {
			return -1;
		}
		__atexit_funcs[__atexit_func_count].destructor_func = destructor;
		__atexit_funcs[__atexit_func_count].obj_ptr = arg;
		__atexit_funcs[__atexit_func_count].dso_handle = dso;
		__atexit_func_count++;
		return 0;
	}

	void __cxa_finalize(void *f)
	{
		uarch_t i = __atexit_func_count;
		if(f == nullptr) {
			while (i--) {
				if (__atexit_funcs[i].destructor_func) {
					(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
				}
			}
			return;
		}
		
		while (i--) {
			if(__atexit_funcs[i].destructor_func == f) {
				(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
				__atexit_funcs[i].destructor_func = 0;
			}
		}
	}
}

void csup_init()
{
	virtual_disk::node *node;

	debug_printf("\x01\x09 CSUP driver");

	virtual_disk::node::create("/", "CSUP");

	// STDIN
	virtual_disk::driver *cin_driver = virtual_disk::driver::create();
	debug_assert(cin_driver != nullptr);
	cin_driver->read = [](virtual_disk::handle&, void *buf, size_t n) -> int {
		return -1;
	};
	node = virtual_disk::node::create("/CSUP", "IN");
	debug_assert(node != nullptr);
	cin_driver->add_node(*node);

	// STDOUT
	virtual_disk::driver *cout_driver = virtual_disk::driver::create();
	debug_assert(cout_driver != nullptr);
	cout_driver->write = [](virtual_disk::handle&, const void *buf, size_t n) -> int {
		kprintf("%u:%s", n, buf);
		return 0;
	};
	node = virtual_disk::node::create("/CSUP", "OUT");
	debug_assert(node != nullptr);
	cout_driver->add_node(*node);

	// STDERR
	virtual_disk::driver *cerr_driver = virtual_disk::driver::create();
	debug_assert(cerr_driver != nullptr);
	cerr_driver->write = [](virtual_disk::handle&, const void *buf, size_t n) -> int {
		kprintf("%u:%s", n, buf);
		return 0;
	};
	node = virtual_disk::node::create("/CSUP", "ERR");
	debug_assert(node != nullptr);
	cerr_driver->add_node(*node);

	// TurboC on MSDOS had an standard printer FILE handle done by the libc, while this is
	// nonstandard, it is nice to have nonethless, we can simply use the default printer the
	// system may contain.
	// STDPRN
	virtual_disk::driver *cprn_driver = virtual_disk::driver::create();
	debug_assert(cprn_driver != nullptr);
	cprn_driver->write = [](virtual_disk::handle&, const void *buf, size_t n) -> int {
		kprintf("%u:%s", n, buf);
		return 0;
	};
	node = virtual_disk::node::create("/CSUP", "PRN");
	debug_assert(node != nullptr);
	cprn_driver->add_node(*node);

	// Standard debug write, z/VM does in fact allow the usage of DIAG 8 for debug messages
	// like how Hercules does it too, so we can safely reuse DIAG 8 without caring about the
	// virtualization platform itself.
	// STDDBG
	virtual_disk::driver *cdbg_driver = virtual_disk::driver::create();
	debug_assert(cdbg_driver != nullptr);
	cdbg_driver->write = [](virtual_disk::handle&, const void *buf, size_t n) -> int {
		kprintf("%u:%s", n, buf);
		return 0;
	};
	node = virtual_disk::node::create("/CSUP", "DBG");
	debug_assert(node != nullptr);
	cdbg_driver->add_node(*node);
}

/* Launch the JDA console */
static inline void exec_user()
{
	const auto load_mod = [](const auto *path, size_t *final_size) -> void * {
		size_t size = (32767 * 10);
		void *buf = storage::alloc(size);
		if(buf == nullptr) {
			debug_printf("\x01\x0B");
			return nullptr;
		}

		auto *hdl = virtual_disk::handle::open_path(path, virtual_disk::mode::READ);
		if(hdl == nullptr) {
			debug_printf("Can't open %s", path);
			storage::free(buf);
			return nullptr;
		}
		
		int r = hdl->read(buf, size);
		size = (size_t)r;
		if(r < 0) {
			debug_printf("\x01\x16\x01\x20 tape %i,SIZE=%u", r, size);
			storage::free(buf);
			return nullptr;
		}

		buf = storage::realloc(buf, size);
		if(buf == nullptr) {
			debug_printf("\x01\x0B");
			return nullptr;
		}
		debug_printf("\x01\x0A %s,SIZE=%u", path, size);
		virtual_disk::handle::close(hdl);
		*final_size = size;
		return buf;
	};

	//size_t libio_size;
	//void *libio_buf = load_mod("/RDISK/SYSLIB$IO", &libio_size);
	//if(libio_buf == nullptr)
	//	kpanic("Can't open libio");
	
	size_t jda_size;
	void *jda_buf = load_mod("/RDISK/DSA$EXE$JDA", &jda_size);
	if(jda_buf == nullptr)
		kpanic("Can't open JDA");

	// Disable the scheduler temporarily & add a new PROBLEM-STATE jobtask
	// basically a userland stuff
	auto *user_job = timeshare::job::create(*"SYSMAIN", 1, static_cast<timeshare::job::flag>(timeshare::job::VIRTUAL | timeshare::job::BITS_64), 65535);

	void *entry;
	//elf64::load(user_job, libio_buf, libio_size, &entry);
	//storage::free(libio_buf);
	elf64::load(user_job, jda_buf, jda_size, &entry);
	storage::free(jda_buf);

	auto *new_task = timeshare::task::create(*user_job, *"JDATASK");
	auto *new_thread = timeshare::thread::create(*user_job, *new_task, 8192 * 16);
	new_thread->set_pc(entry, false);
	user_job->flags = static_cast<timeshare::job::flag>(user_job->flags & (~timeshare::job::SLEEP));
}

[[noreturn]] void main() noexcept
{
	// Start the early memory manager - with only one memory partition enough to fit early boot.
	kprintf("\x01\x09 the Real\x01\x0D\r\n");
	real_storage::init();

	// Basic user and group authorization
	kprintf("\x01\x09 users and groups\r\n");
	usersys::init();
	auto uid = usersys::user::create(*"SYSTEM01", usersys::user_flags::OVERRIDE_PERMS);
	usersys::user::set_current(uid);

	// Virtual filesystem
	kprintf("\x01\x09 VFS\r\n");
	virtual_disk::init();

	debug_printf("\x01\x09 the Dynamic address\x01\x03 facility");
	virtual_storage::init();

	init_smp();
	// --- After this point, device I/O is safe to use.
	debug_printf("\x01\x09\x01\x0C kernel");

	// css::probe();
	auto *fdt_hdr = reinterpret_cast<fdt::header *>(fdt::fdt_address);
	if(fdt_hdr != nullptr) {
		auto *node = fdt::get_node(fdt_hdr, "/soc/uart");
		if(node != nullptr) {
			auto *addr = fdt::get_node_address(node);
			if(addr != nullptr) {
				uart::init(addr);

				// stdout
				extern virtual_disk::handle *g_stdout;
				g_stdout = virtual_disk::handle::open_path("/UART", virtual_disk::node_flags::WRITE);
			}
		}

		node = fdt::get_node(fdt_hdr, "/soc/pci");
		if(node != nullptr) {
			auto *ecam = fdt::get_node_address(node);

			const auto *size_prop = fdt::get_prop(fdt_hdr, node, "#size-cells");
			const auto size_cells = be_to_cpu32(*reinterpret_cast<const uint32_t *>(fdt::get_prop_value(size_prop)));
			
			const auto *addr_prop = fdt::get_prop(fdt_hdr, node, "#address-cells");
			const auto addr_cells = be_to_cpu32(*reinterpret_cast<const uint32_t *>(fdt::get_prop_value(addr_prop)));

			const auto *ranges_prop = fdt::get_prop(fdt_hdr, node, "ranges");
			const auto range_size = addr_cells + size_cells + 1;
			const auto range_cnt = fdt::get_prop_size(ranges_prop) / range_size;

			const auto *pci_ranges = fdt::get_prop_value(ranges_prop); // MMIO base, prefetch base and PIO base
			const auto *uc_ranges_array = reinterpret_cast<const uint8_t *>(pci_ranges);
			debug_printf("RANGES CNT=%u,SIZE=%u", range_cnt, range_size);
			for(size_t i = 0; i < range_cnt; i++) {
				uint64_t child_value = 0;
				for(size_t j = 0; j < range_size; j++) {
					child_value <<= 8;
					child_value |= *(uc_ranges_array + j);
				}
				debug_printf("CHILD: %p", child_value);
				uc_ranges_array += range_size;
			}

			debug_printf("PCI ecam=%p,ranges=%p,addr=%u,size=%u", ecam, pci_ranges, static_cast<size_t>(addr_cells), static_cast<size_t>(size_cells));
			auto main_ctl = pci::controller(ecam, PCI_ECAM_SIZE * PCI_MAX_SLOTS * PCI_MAX_FUNCS, nullptr, 0, nullptr, 0);
		}
	} else {
		debug_printf("No FDT found");
	}
	
	// hercns_init();
	csup_init();
#ifdef TARGET_S390
	css::init();
	// Add the IPL disk and hand it out to the ZDSFS driver.
	const auto ipl_dasd_dev = css::add_device((css::schid){1, 0});
	if(ipl_dasd_dev < 0)
		kpanic("Can't add the IPL disk");
	dasd::init(ipl_dasd_dev);

	{
		auto* hdl = virtual_disk::handle::open_path("/SYSTEM/DEVICES/DASD001", virtual_disk::node_flags::READ);
		if(hdl == nullptr)
			kpanic("Can't open SYSDISK");
		zdsfs::init(*hdl);
		virtual_disk::handle::close(hdl);
	}

	// And the operator terminal.
	const auto op_term_dev = css::add_device((css::schid){1, 1});
	if(op_term_dev < 0)
		kpanic("Can't add the OPTERM");
	x3270::init(op_term_dev);
#endif
#if defined DEBUG
	virtual_disk::dump();
#endif
	kprintf("Welcome to\x01\x0C\r\n");
	boot::init();
	exec_user();

	/* Constantly yield to the scheduler */
	debug_printf("Running on loop");
	while(1) {}
}
