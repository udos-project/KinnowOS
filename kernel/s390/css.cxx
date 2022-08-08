#include <s390/css.hxx>
#include <s390/asm.hxx>
#include <storage.hxx>
#include <printf.hxx>

#include <mutex.hxx>
#include <errcode.hxx>

/**
 * @brief The global request queue, here all requests of the entire system are stored
 * so the spooler can schedule them correctly
 */
/* @todo SMP support and parallelization of these requests (i.e offload spooler to multiple cores) */
constinit static storage::global_wrapper<storage::concurrent_dynamic_list<css::request *>> g_queue;
constinit static storage::global_wrapper<storage::concurrent_dynamic_list<css::device>> g_devlist;
constinit static storage::global_wrapper<storage::concurrent_dynamic_list<css::request>> g_request_pool;

int css::init()
{
	debug_printf("DEVLIST,SIZE=%u", g_devlist->size());
	debug_assert(g_devlist->size() == 0);
	debug_assert(g_request_pool->size() == 0);
	return 0;
}

/**
 * @brief Creates a new request to be placed on the css request queue, do not fill the IRB or the SCHIB
 * of these requests, since the request queue manager spooler will bzero them out
 * 
 * @param dev The device to create the request for
 * @param n_ccws Number of CCWs attached to this specific request
 * @return struct css::request* The request object itself, either to be processed-on-demand
 * or to be sent to a queue, depending on the caller's needs
 */
css::request *css::request::create(css::device& dev, size_t n_ccws)
{
	auto& request_pool = *(g_request_pool.operator->());
	const base::scoped_mutex lock(request_pool.lock);
	debug_printf("g_request_pool->size=%u", request_pool.size());
	auto *req = request_pool.insert();
	if(req == nullptr) return nullptr;
	req->schid = dev.schid;
	if(req->ccws.resize(n_ccws) != 0) {
		request_pool.resize(request_pool.size() - 1);
		return nullptr;
	}
	return req;
}

/**
 * @brief Delete the specified request (not from the queue!)
 * 
 * @param req Request to delete
 */
void css::request::destroy(css::request *req)
{
	req->lock.lock();
	req->flags = 0;
	req->lock.unlock();
	const base::scoped_mutex lock(g_request_pool->lock);
	g_request_pool->remove(*req);
}

/**
 * @brief Sends a CSS request to the main queue. Note that this releases ownership of
 * the request object since the allocated memory is now freed
 * 
 * @param req The request to send to the queue
 * @return int Return code of operation, negative denotes failure
 */
int css::request::send()
{
	const base::scoped_mutex lock1(g_queue->lock);
	// Apply changes and put the request on the requests linked list
	if(g_queue->insert(this) == nullptr)
		return error::ALLOCATION;
	return 0;
}

#if defined DEBUG
#include <s390/dasd.hxx>

struct dasd_seek_info {
	uint16_t block; /* Block (set to 0) */
	uint16_t cyl; /* Cylinder */
	uint16_t head; /* Head/Track */
	uint8_t record; /* Record */
} PACKED;

struct dasd_search_info {
	uint16_t cyl; /* Cylinder */
	uint16_t head; /* Head/Track */
	uint8_t record; /* Record */
} PACKED;

void css::request::dump() const
{
	for(size_t i = 0; i < this->ccws.size(); i++) {
		const char *msg = nullptr;

		if(this->ccws[i].cmd == DASD_CMD_LD) {
			msg = "LD";
		} else if(this->ccws[i].cmd == DASD_CMD_SEARCH) {
			const auto *info = reinterpret_cast<const dasd_search_info *>((uintptr_t)this->ccws[i].addr);
			msg = "SEARCH";
			debug_printf("CYL=%uHEAD=%%uEC=%u%u", info->cyl, info->head, info->record);
		} else if(this->ccws[i].cmd == DASD_CMD_SEEK) {
			const auto *info = reinterpret_cast<const dasd_seek_info *>((uintptr_t)this->ccws[i].addr);
			msg = "SEEK";
			debug_printf("BLOCK=%uCYL=%%uEAD=%u%uC=%u", info->block, info->cyl, info->head, info->record);
		} else if(this->ccws[i].cmd == DASD_CMD_WR_LD) {
			msg = "WR-LD";
		} else if(this->ccws[i].cmd == css::cmd::CONTROL) {
			msg = "CONTROL";
		} else if(this->ccws[i].cmd == css::cmd::READ) {
			msg = "READ";
		} else if(this->ccws[i].cmd == css::cmd::READ_BACKWARDS) {
			msg = "READ-BW";
		} else if(this->ccws[i].cmd == css::cmd::SENSE) {
			msg = "SENSE";
		} else if(this->ccws[i].cmd == css::cmd::SENSE_ID) {
			msg = "SENSE-ID";
		} else if(this->ccws[i].cmd == css::cmd::TIC) {
			msg = "TIC";
		} else if(this->ccws[i].cmd == css::cmd::WRITE) {
			msg = "WRITE";
		}

		char flagbuf[80] = {0};
		flagbuf[0] = '\0';
		if(this->ccws[i].flags & CSS_CCW_CC)
			storage_string::concat(flagbuf, "CC,");
		if(this->ccws[i].flags & CSS_CCW_CD)
			storage_string::concat(flagbuf, "CD,");
		if(this->ccws[i].flags & CSS_CCW_IDA)
			storage_string::concat(flagbuf, "IDA,");
		if(this->ccws[i].flags & CSS_CCW_MIDA)
			storage_string::concat(flagbuf, "MIDA,");
		if(this->ccws[i].flags & CSS_CCW_PCI)
			storage_string::concat(flagbuf, "PCI,");
		if(this->ccws[i].flags & CSS_CCW_S)
			storage_string::concat(flagbuf, "S,");
		if(this->ccws[i].flags & CSS_CCW_SLI)
			storage_string::concat(flagbuf, "SLI,");
		if(this->ccws[i].flags & CSS_CCW_SPK)
			storage_string::concat(flagbuf, "SPK,");

		debug_printf("CCW#%u%s %p(%%uFLAGS=%s", i, msg, (uintptr_t)this->ccws[i].addr, (size_t)this->ccws[i].length, flagbuf);
	}
}
#endif

/**
 * @brief Perform one request from the request queue (better used on multithreading CSS/IO stuff) if any part
 * of the process fails the function will return and the queue will be left unaffected, the caller can then
 * retry the request or "drop" the request from the queue 
 * 
 * @return int Return code of the operation, negative means some kind of failure
 */
int css::request_perform()
{
	int r = 0;

	auto& request_queue = *(g_queue.operator->());

	// Must acquire the g_queue lock to proceed :)
	if(!request_queue.lock.try_lock()) return 0;
	if(request_queue.empty()) {
		request_queue.lock.unlock();
		return 0;
	}

	const size_t req_queue_idx = request_queue.size() - 1;
	auto *req = request_queue[req_queue_idx];
	const base::scoped_mutex lock1(req->lock);
	debug_css_printf(req->schid, "Perform request #%u(flags=%x)", req_queue_idx, (unsigned int)req->flags);

	auto irb = css::irb{};
	auto orb = css::orb{};
	// Proper preparation for the requests below, CPA_ADDR should not be set by the caller
	orb.cpa_addr = (uint32_t)((uintptr_t)&req->ccws[0] & 0xffffffff);
	/// @todo Analyze more intelligent ORB usage
	orb.flags = 0x0080FF00;
	/// @todo Is it really required for pointing to the desired CCW address to start at?, doesn't ORB already do this?
	*((volatile uint32_t *)0x48) = (uint32_t)((uintptr_t)&req->ccws[0] & 0xffffffff);

	// Test that the device is actually online
	if(req->flags & css::request_flags::MODIFY) {
		debug_css_print(req->schid, "Test channel (modify)");
		r = css::channel_test(req->schid, &irb);
		if(r == css::status::NOT_PRESENT && !(req->flags & css::request_flags::IGNORE_CC)) {
			debug_css_print(req->schid, "Test channel (modify) failed");
			r = error::EXPLICIT_FAILURE;
			goto end;
		}

		debug_css_print(req->schid, "Modify channel");
		r = css::channel_modify(req->schid, &orb);
		if(r == css::status::NOT_PRESENT && !(req->flags & css::request_flags::IGNORE_CC)) {
			debug_css_print(req->schid, "Modify channel failed");
			r = error::EXPLICIT_FAILURE;
			goto end;
		}
	}

	// Test that our device is good to go
	r = css::channel_test(req->schid, &irb);
	debug_css_print(req->schid, "Test channel");
	if(r == css::status::NOT_PRESENT && !(req->flags & css::request_flags::IGNORE_CC)) {
		debug_css_print(req->schid, "Test channel failed");
		r = error::EXPLICIT_FAILURE;
		goto end;
	}

	// Wait for attention (aka. 3270 specifics, for example READ_CCW)
	if(req->flags & css::request_flags::WAIT_ATTENTION) {
		/// @todo Subsystem for repeated messages
		debug_css_printf(req->schid, "Waiting for attention", 0);
		// Loop until the attention bit is set or the device fails
		long int timeout = 0;
		while(!(irb.scsw.device_status & CSS_SCSW_DS_ATTENTION)) {
			timeout++;
			s390_intrin::wait_io();
			// debug_css_print(req->schid, "Test channel (WA)");
			r = css::channel_test(req->schid, &irb);
			if(r == css::status::NOT_PRESENT && !(req->flags & css::request_flags::IGNORE_CC)) {
				debug_css_print(req->schid, "Test channel failed (WA)");
				r = error::EXPLICIT_FAILURE;
				goto end;
			}
			if(timeout > 65535) {
				r = error::TIMEOUT;
				goto end;
			}
		}
	}

	/* Send the CSS program to the device */
	debug_css_print(req->schid, "Start channel");
	r = css::channel_start(req->schid, &orb);
	if(r == css::status::NOT_PRESENT && !(req->flags & css::request_flags::IGNORE_CC)) {
		debug_css_print(req->schid, "Start channel failed");
		r = error::EXPLICIT_FAILURE;
		goto end;
	}
	
	// Wait for the above start to fully commence, this will send our CSS program to the channel subsystem
	// and thus our target device will perform said CSS program... we will just wait for it to respond.
	/// @todo Do something else while this is being done
	s390_intrin::wait_io();

	// Obtain the results of the previous operation
	debug_css_print(req->schid, "Test channel");
	r = css::channel_test(req->schid, &irb);
	if(r == css::status::NOT_PRESENT && !(req->flags & css::request_flags::IGNORE_CC)) {
		debug_css_print(req->schid, "Test channel failed");
		r = error::EXPLICIT_FAILURE;
		goto end;
	}

	{
		// Compare that the end address of the CSS request is equal to the last channel word, this must be set
		// accordingly to know if the request was completed or not - if an application wants to know where the
		// device failed at they can consult the IRB of their request and the CPA_ADDRESS will be unchanged
		// (because happy reminder that requests are not deleted, they're simply removed from the list of requests).
		/// @todo Test if CPA_ADDR can be used to check where a CSS program failed
		auto *end_ccw = &req->ccws.unsafe_at(req->ccws.size());
		if((uintptr_t)irb.scsw.cpa_addr != (uintptr_t)end_ccw) {
			debug_css_printf(req->schid, "Command chain not completed (CPA=%p,AD=%p)", (uintptr_t)irb.scsw.cpa_addr, end_ccw);
			r = error::EXPLICIT_FAILURE;
			goto end;
		}
	}
end:
#if defined DEBUG
	debug_printf("CSS-%s", (r < 0) ? "Failure" : "Success");
	req->dump();
#endif
	req->flags = css::request_flags::DONE;
	req->retcode = r;
	request_queue.remove(req_queue_idx);
	request_queue.lock.unlock();
	return (int)request_queue.size();
}

/**
 * @brief Add a CSS device to the device list
 * 
 */
css::device::id css::add_device(css::schid schid)
{
	auto& devlist = *(g_devlist.operator->());
	debug_assert(schid.id != 0);

	/// @todo Multithreading safe!   
	auto *dev = devlist.insert();
	if(dev == nullptr) return -1;
	
	const auto id = static_cast<css::device::id>(devlist.size() - 1);

	/// @todo Check device is not duplicated!
	dev->schid.id = schid.id;
	dev->schid.num = schid.num;
	return id;
}

/**
 * @brief Obtains a device by it's schid
 * 
 * @param schid The schid to search for
 * @return struct css::device* nullptr if device is not found
 */
css::device *css::get_device(css::schid schid)
{
	auto& devlist = *(g_devlist.operator->());
	debug_assert(schid.id != 0);
	debug_printf("GETDEV,N=%u", devlist.size());
	for(size_t i = 0; i < devlist.size(); i++) {
		debug_printf("%i:%i==%i:%i", (int)devlist[i].schid.id, (int)devlist[i].schid.num, (int)schid.id, (int)schid.num);
		if(devlist[i].schid.id != schid.id || devlist[i].schid.num != schid.num)
			continue;
		return &devlist[i];
	}
	return nullptr;
}

css::device *css::get_device(css::device::id id)
{
	auto& devlist = *(g_devlist.operator->());
	if((size_t)id >= devlist.size())
		return nullptr;
	return &devlist[id];
}

/**
 * @brief Probe for devices in the channel subsystem via automatic recognition
 * from the cu_sense data they hand out, probably handcrafted by the VM
 * 
 * @return int Unused
 */
int css::probe()
{
#if defined DEBUG && 0
	css::senseid sensebuf;
	css::schid schid = {0, 0};

	auto *dev = (struct css::device *)storage::allocz(sizeof(css::device));
	if(dev == nullptr) return error::ALLOCATION;

	for(schid.id = 1; schid.id < 2; schid.id++) {
		for(schid.num = 0; schid.num < 80; schid.num++) {
			int r;

			dev = {0}; /// TODO: This fill might not be needed
			dev->schid = schid;

			/* First enable the device so the SenseId can work */
			debug_css_printf(dev->schid, "Enabling device", 0);
			struct css::request *req = css::request::create(*dev, 1);
			req->ccws[0].cmd = 0x27;
			req->ccws[0].set_addr(&sensebuf);
			req->ccws[0].flags = css::request_flags::MODIFY | css::request_flags::IGNORE_CC;
			req->ccws[0].length = (uint16_t)sizeof(sensebuf);
			req->send();
			css::request::destroy(req);
			while(!(req->flags & css::request_flags::DONE)) { /* ... */ }
			r = req->retcode;
			debug_css_printf(dev->schid, "ENABLE_DEV rc=%i", (int)r);

			/* Now send the SenseId request */
			debug_css_printf(dev->schid, "Performing SENSE_ID", 0);
			sensebuf = {
				.reserved = 0,
				.cu_type = 0,
				.cu_model = 0,
				.dev_type = 0,
				.dev_model = 0,
				.unused = 0,
				.ciw = 0,
			};
			req = css::request::create(*dev, 1);
			req->ccws[0].cmd = css::cmd::SENSE_ID;
			req->ccws[0].set_addr(&sensebuf);
			req->ccws[0].flags = 0;
			req->ccws[0].length = (uint16_t)sizeof(sensebuf);
			req->send();
			while(!(req->flags & css::request_flags::DONE)) {
				/* ... */
			}
			r = req->retcode;
			if(r != css::status::OK) {
				debug_css_printf(dev->schid, "SENSE_ID Failed! rc=%i", (int)r);
				continue;
			}
			css::request::destroy(req);

			debug_css_printf(dev->schid, "Type: %x, Model: %x", (unsigned int)sensebuf.cu_type, (unsigned int)sensebuf.cu_model);
			switch(sensebuf.cu_type) {
			case 0x1403:
				debug_css_printf(dev->schid, "%x printer", (unsigned int)sensebuf.cu_type);
				break;
			case 0x2305:
			case 0x2311:
			case 0x2314:
			case 0x3330:
			case 0x3340:
			case 0x3350:
			case 0x3375:
			case 0x3380:
			case 0x3990:
			case 0x9345:
				debug_css_printf(dev->schid, "%x disk", (unsigned int)sensebuf.cu_type);
				break;
			case 0x1052:
			case 0x2703:
				debug_css_printf(dev->schid, "%x console", (unsigned int)sensebuf.cu_type);
				break;
			case 0x3270:
			case 0x3287:
				debug_css_printf(dev->schid, "%x terminal", (unsigned int)sensebuf.cu_type);
				break;
			default:
				debug_css_printf(dev->schid, "type=%x, model=%x", (unsigned int)sensebuf.cu_type, (unsigned int)sensebuf.cu_model);
				break;
			}
		}
	}
	storage::free(dev);
#endif
	return 0;
}

int css::dev_enable(struct css::device& dev)
{
	auto *req = css::request::create(dev, 1);
	if(req == nullptr) return error::ALLOCATION;
	req->flags = css::request_flags::MODIFY | css::request_flags::IGNORE_CC;

	req->ccws[0].cmd = 0x27;
	req->ccws[0].set_addr((void *)nullptr);
	req->ccws[0].flags = 0;
	req->ccws[0].length = 0;

	req->send();
	while(!(req->flags & css::request_flags::DONE)) {}
	css::request::destroy(req);
	return req->retcode;
}

