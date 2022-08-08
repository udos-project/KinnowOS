#include <s390/x3270.hxx>
#include <s390/css.hxx>
#include <vdisk.hxx>
#include <errcode.hxx>

constinit static int g_termnum = 0;
constinit static virtual_disk::driver *gs_driver = nullptr;

static inline int enable_term(css::device::id id) {
	auto *req = css::request::create(*css::get_device(id), 1);
	if(req == nullptr) return error::ALLOCATION;

	req->flags = css::request_flags::MODIFY | css::request_flags::IGNORE_CC;

	req->ccws[0].cmd = x3270::cmd::ENABLE;
	req->ccws[0].set_addr(&req->ccws[0]);
	req->ccws[0].flags = 0;
	req->ccws[0].length = 0;

	req->send();
	long long timer = 0;
	while(!(req->flags & css::request_flags::DONE)) {
		/** @todo This kludge is a very bad way to do this, we MUST find a way to not do this
		 * like come on we have an spooler why can't we use it? */
		/* ... */
		timer++;
		if(timer >= 0xffff * 16)
			css::request_perform();
	}
	int r = req->retcode;
	css::request::destroy(req);
	return r;
}

void x3270::init(css::device::id id)
{
	base::scoped_mutex lock(css::get_device(id)->lock);

	enable_term(id);

	gs_driver = virtual_disk::driver::create();
	debug_assert(gs_driver != nullptr);
	gs_driver->read = [](virtual_disk::handle& hdl, void *buf, size_t n) -> int {
		auto& data = *static_cast<x3270::node_data *>(hdl.node->driver_data);
		auto *req = css::request::create(*css::get_device(data.id), 1);
		if(req == nullptr) return error::ALLOCATION;

		req->flags = css::request_flags::MODIFY | css::request_flags::IGNORE_CC | css::request_flags::WAIT_ATTENTION;

		req->ccws[0].cmd = x3270::cmd::READ_MOD;
		req->ccws[0].set_addr(buf);
		req->ccws[0].flags = CSS_CCW_SLI;
		req->ccws[0].length = (uint16_t)n;

		req->send();
		long long timer = 0;
		while(!(req->flags & css::request_flags::DONE)) {
			/** @todo This kludge is a very bad way to do this, we MUST find a way to not do this
			 * like come on we have an spooler why can't we use it? */
			/* ... */
			timer++;
			if(timer >= 0xffff * 16)
				css::request_perform();
		}
		
		int r;
		if(req->retcode != css::request_flags::DONE) {
			r = error::EXPLICIT_FAILURE;
		} else {
			r = (int)n - (int)css::get_device(data.id)->irb.scsw.count;
		}
		css::request::destroy(req);
		return r;
	};
	gs_driver->write = [](virtual_disk::handle& hdl, const void *buf, size_t n) -> int {
		auto& data = *static_cast<x3270::node_data *>(hdl.node->driver_data);
		auto *req = css::request::create(*css::get_device(data.id), 1);
		if(req == nullptr) return error::ALLOCATION;

		req->flags = css::request_flags::MODIFY;

		req->ccws[0].cmd = x3270::cmd::WRITE_NOCR;
		req->ccws[0].set_addr(buf);
		req->ccws[0].flags = 0;
		req->ccws[0].length = (uint16_t)n;

		req->send();
		long long timer = 0;
		while(!(req->flags & css::request_flags::DONE)) {
			/** @todo This kludge is a very bad way to do this, we MUST find a way to not do this
			 * like come on we have an spooler why can't we use it? */
			/* ... */
			timer++;
			if(timer >= 0xffff * 16)
				css::request_perform();
		}

		int r;
		if(req->retcode != css::request_flags::DONE) {
			r = error::EXPLICIT_FAILURE;
		} else {
			r = (int)n - (int)css::get_device(data.id)->irb.scsw.count;
		}
		css::request::destroy(req);
		return r;
	};

	// Add the terminal
	char namebuf[8];
	storage::copy(namebuf, "TERM", 4);
	namebuf[4] = '0' + (char)((g_termnum / 100) % 10);
	namebuf[5] = '0' + (char)((g_termnum / 10) % 10);
	namebuf[6] = '0' + (char)((g_termnum / 1) % 10);
	namebuf[7] = '\0';

	auto *node = virtual_disk::node::create("/SYSTEM/DEVICES", namebuf);
	debug_assert(node != nullptr);
	node->driver_data = storage::alloc(sizeof(x3270::node_data));
	auto *node_data = (x3270::node_data *)node->driver_data;
	node_data->id = id;
	gs_driver->add_node(*node);
}
