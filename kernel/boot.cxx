/**
 * @file boot.cxx
 * @author wxwisiasdf
 * @brief Program stub for bootloading other OSes
 * @version 0.1
 * @date 2022-06-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <boot.hxx>
#include <vdisk.hxx>
#include <arch/virtual.hxx>
#include <printf.hxx>
#include <errcode.hxx>
#include <elf.hxx>
#include <locale.hxx>

namespace boot {
	static inline int limine_parse_request(void *req);
	static inline void limine_parse_config(const char *buf, size_t size);
	static inline int limine(virtual_disk::handle& hdl);
	static inline int stivale1(virtual_disk::handle& hdl);
	static inline int stivale2(virtual_disk::handle& hdl);
}

//
// Limine
//
constinit static limine::bootloader_info_response g_boot_info_resp = {
	.revision = 0,
	.name = "Limine", // Let's pretend to be limine
	.version = "4.0",
};

constinit static limine::stack_size_response g_stack_size_resp = {
	.revision = 0,
};

constinit static storage::global_wrapper<storage::concurrent_dynamic_list<limine::terminal>> g_terminals;
constinit static limine::terminal_callback g_terminal_cb = nullptr;
constinit static limine::terminal_response g_terminal_resp = {
	.revision = 0,
	.terminal_count = 0,
	.terminals = nullptr,
	.write = [](limine::terminal *term, const char *data, uint64_t len) -> void {
		if(g_terminal_cb != nullptr && 0) { // TODO: Use term
			g_terminal_cb(term, 0, 0, 0, 0);
		}
		term->hdl->write(data, len);
	},
};

constinit static struct {
	uint64_t stack_size;
	bool quiet;
	bool use_serial;
	bool verbose;
	bool kaslr;
} g_limine_opts = {
	.stack_size = 8192,
	.quiet = false,
	.use_serial = false,
	.verbose = false,
	.kaslr = false,
};

constinit static limine::request_parser parsers[] = {
	{ LIMINE_BOOTLOADER_INFO_REQUEST, [](void *req) -> int {
		auto *request = reinterpret_cast<limine::bootloader_info_request *>(req);
		request->revision = 0;
		request->response = &g_boot_info_resp;
		return 0;
	} },
	{ LIMINE_STACK_SIZE_REQUEST, [](void *req) -> int {
		auto *request = reinterpret_cast<limine::stack_size_request *>(req);
		request->revision = 0;
		request->response = &g_stack_size_resp;
		g_limine_opts.stack_size = static_cast<uint64_t>(request->stack_size);
		return 0;
	} },
	{ LIMINE_HHDM_REQUEST, nullptr },
	{ LIMINE_FRAMEBUFFER_REQUEST, nullptr },
	{ LIMINE_FRAMEBUFFER_LEGACY_REQUEST, nullptr },
	{ LIMINE_TERMINAL_REQUEST, [](void *req) ->int {
		auto *request = reinterpret_cast<limine::terminal_request *>(req);
		request->revision = 0;
		request->response = &g_terminal_resp;
		request->callback = [](limine::terminal *, uint64_t, uint64_t, uint64_t, uint64_t) -> void {
			
		};
		return 0;
	} },
	{ LIMINE_TERMINAL_LEGACY_REQUEST, nullptr },
	{ LIMINE_5_LEVEL_PAGING_REQUEST, nullptr },
	{ LIMINE_SMP_REQUEST, nullptr },
	{ LIMINE_MEMMAP_REQUEST, nullptr },
	{ LIMINE_ENTRY_POINT_REQUEST, nullptr },
	{ LIMINE_KERNEL_FILE_REQUEST, nullptr },
	{ LIMINE_MODULE_REQUEST, nullptr },
	{ LIMINE_RSDP_REQUEST, nullptr },
	{ LIMINE_SMBIOS_REQUEST, nullptr },
	{ LIMINE_EFI_SYSTEM_TABLE_REQUEST, nullptr },
	{ LIMINE_BOOT_TIME_REQUEST, nullptr },
	{ LIMINE_KERNEL_ADDRESS_REQUEST, nullptr },
};

static inline int boot::limine_parse_request(void *_req)
{
	auto *request = reinterpret_cast<limine::request<void> *>(_req);	
	return 0;
}

static inline void boot::limine_parse_config(const char *buf, size_t size) {
	// Parse the limine configuration
	const auto *str = reinterpret_cast<const char *>(buf);
	while(*str != '\0') {
		// TIMEOUT [time]
		if(!storage::compare(str, "TIMEOUT", 7)) {
			str += 7;
		} else if(!storage::compare(str, "QUIET", 5)) {
			str += 5;
			g_limine_opts.quiet = true;
		} else if(!storage::compare(str, "SERIAL", 6)) {
			str += 6;
			g_limine_opts.use_serial = true;
		} else if(!storage::compare(str, "GRAPHICS", 8)) {
			str += 8;
		} else if(!storage::compare(str, "DEFAULT_ENTRY", 13)) {
			str += 13;
		} else if(!storage::compare(str, "RANDOMISE_MEMORY", 15) || !storage::compare(str, "RANDOMIZE_MEMORY", 15)) {
			str += 15;
			g_limine_opts.kaslr = true;
		} else {
			
		}
		while(*str != '\0' && *str != '\n') str++;
		if(*str == '\n') str++; // Make the pointer of the next iteration be at text
	}	
}

/// @brief Initialize limine boot services
/// @param File that contains the limine.cfg configuration file
static inline int boot::limine(virtual_disk::handle& hdl)
{
	// Fill out data that can't be const-evaled
	g_terminal_resp.terminals = g_terminals->get_as_array();

	size_t size = 32767 * 10;
	void *buf = storage::alloc(size);
	if(buf == nullptr) return error::ALLOCATION;
	int r = hdl.read(buf, size);
	if(r < 0) {
		storage::free(buf);
		return r;
	}
	size = static_cast<size_t>(r);
	buf = storage::realloc(buf, size);
	if(buf == nullptr) return error::ALLOCATION;
	limine_parse_config(reinterpret_cast<const char *>(buf), size);
	debug_printf("\x01\x0A,SIZE=%u", size);
	
	// Open handles to terminals
	auto *sys_hdl = virtual_disk::handle::open_path("/SYSTEM/DEVICES/TERM001", virtual_disk::node_flags::READ);
	if(sys_hdl == nullptr) {
		debug_printf("No terminals");
		return -1;	
	}

	auto *term_resp = &g_terminal_resp;
	term_resp->terminal_count = 1;
	term_resp->terminals = reinterpret_cast<limine::terminal **>(storage::allocza(term_resp->terminal_count, sizeof(limine::terminal *)));
	if(term_resp->terminals == nullptr) goto end_err;
	term_resp->terminals[0] = reinterpret_cast<limine::terminal *>(storage::allocz(sizeof(limine::terminal)));
	term_resp->terminals[0]->hdl = sys_hdl;

	// Disable the scheduler temporarily & add a new PROBLEM-STATE jobtask basically a userland stuff
	{
		auto *user_job = timeshare::job::create(*"BOOTMAIN", 1, static_cast<timeshare::job::flag>(timeshare::job::VIRTUAL | timeshare::job::BITS_64), 65535);
		void *entry;
		elf64::load(user_job, buf, size, &entry);
		storage::free(buf);
		auto *new_task = timeshare::task::create(*user_job, *"BOOT");
		auto *new_thread = timeshare::thread::create(*user_job, *new_task, 8192 * 16);
		new_thread->set_pc(entry, false);
		user_job->flags = static_cast<timeshare::job::flag>(user_job->flags & (~timeshare::job::SLEEP));
	}
	return 0;
end_err:
	virtual_disk::handle::close(sys_hdl);
	return -1;
}

/// @brief Stivale1 initialization
static inline int boot::stivale1(virtual_disk::handle& hdl)
{
	return 0;
}

/// @brief Stivale2 initialization
static inline int boot::stivale2(virtual_disk::handle& hdl)
{
	return 0;
}

/// @brief Initialize boot services and probe for any OSes that might want to be booted
int boot::init()
{
	kprintf("\x01\x09 \x01\x0C-boot services\r\n");
	
	// Limine booting services :^)
	auto* hdl = virtual_disk::handle::open_path("/RDISK/LIMINE", virtual_disk::node_flags::READ);
	if(hdl != nullptr) {
		boot::limine(*hdl);
		goto end;
	}

	// Else, check for stivale configuration
	hdl = virtual_disk::handle::open_path("/RDISK/STIVALE", virtual_disk::node_flags::READ);
	if(hdl != nullptr) {
		boot::stivale1(*hdl);
		goto end;
	}

	// Or an stivale2 configuration
	hdl = virtual_disk::handle::open_path("/RDISK/STIVALE2", virtual_disk::node_flags::READ);
	if(hdl != nullptr) {
		boot::stivale2(*hdl);
		goto end;
	}
end:
	virtual_disk::handle::close(hdl);
	return 0;
}
