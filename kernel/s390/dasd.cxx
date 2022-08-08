#include <s390/dasd.hxx>
#include <printf.hxx>

#include <errcode.hxx>

/// @brief Type of disk
enum disk_type {
	DISK_2305_1,
	DISK_2305_2,
	DISK_2311,
	DISK_2314,
	DISK_3330_1,
	DISK_3330_11,
	DISK_3340_35,
	DISK_3340_70,
	DISK_3344,
};

/// @brief Measurements taken from https://ibmmainframes.com/references/disk.html
constinit static const struct disk_length_info {
	short disk_type; // Type of disk
	short data_cyl; // Data cylinders
	short alt_cyl; // Alternate cylinders
	short tracks_per_cyl; // Tracks per cylinder
	short bytes_per_track; // Bytes per track
	// bytes_per_cylinder = bytes_per_track * tracks_per_cyl
	// total_bytes = bytes_per_cylinder * (data_cylinders + alt_cylinders)
} infos[] = {
	{ DISK_2305_1, 48, 6, 8, 14136 },
	{ DISK_2305_2, 96, 12, 8, 14660 },
	{ DISK_2311, 200, 3, 10, 3625 },
	{ DISK_2314, 200, 3, 20, 7294 },
	{ DISK_3330_1, 404, 7, 19, 13030 },
	{ DISK_3330_11, 808, 7, 19, 13030 },
	{ DISK_3340_35, 348, 1, 12, 8368 },
	{ DISK_3340_70, 696, 2, 12, 8368 },
	{ DISK_3344, 2784, 8, 12, 8368 },
};

constinit static virtual_disk::disk_loc g_disk_loc = {
	.cylinder = 0,
	.track = 0,
	.record = 0,
};

/// @brief DASD number
constinit static int g_dasdnum = 1;

/// @brief Small SEEK structure required by the SEEK CCW and the SEARCH CCW
/// present on both read and write operations
struct dasd_disk_seek {
	uint16_t block; /* Block (set to 0) */
	uint16_t cyl; /* Cylinder */
	uint16_t head; /* Head/Track */
	uint8_t record; /* Record */
} __attribute__((packed, aligned(8)));

namespace dasd {
	static inline int read_single(virtual_disk::handle& dev, const virtual_disk::disk_loc& fdscb, void *buf, size_t n);
	static inline int write_single(virtual_disk::handle& dev, const virtual_disk::disk_loc& fdscb, const void *buf, size_t n);
}

/// @brief Reads a single record/buffer
/// @param hdl The disk device
/// @param loc Location to start reading at
/// @param buf Buffer to write the read into
/// @param n Size of read
/// @return int Number of bytes read, negative is error
static inline int dasd::read_single(virtual_disk::handle& hdl, const virtual_disk::disk_loc& loc, void *buf, size_t n)
{
	auto& dev = *css::get_device((css::device::id)((uintptr_t)hdl.driver_data));
	dasd_disk_seek seek_ptr;

	debug_printf("Single_Reading CYL=%i,HEAD=%i,RECORD=%i", (int)loc.cylinder, (int)loc.track, (int)loc.record);

	auto *req = css::request::create(dev, 4);
	if(req == nullptr) return error::ALLOCATION;

	req->ccws[0].cmd = DASD_CMD_SEEK;
	req->ccws[0].set_addr(&seek_ptr.block);
	req->ccws[0].flags = CSS_CCW_CC;
	req->ccws[0].length = 6;

	req->ccws[1].cmd = DASD_CMD_SEARCH;
	req->ccws[1].set_addr(&seek_ptr.cyl);
	req->ccws[1].flags = CSS_CCW_CC;
	req->ccws[1].length = 5;

	req->ccws[2].cmd = css::cmd::TIC;
	req->ccws[2].set_addr(&req->ccws[1]);
	req->ccws[2].flags = 0x00;
	req->ccws[2].length = 0;
	
	req->ccws[3].cmd = DASD_CMD_LD;
	req->ccws[3].set_addr(buf);
	req->ccws[3].flags = CSS_CCW_SLI;
	req->ccws[3].length = (uint16_t)n;

	seek_ptr.block = 0;
	seek_ptr.cyl = static_cast<uint16_t>(loc.cylinder);
	seek_ptr.head = static_cast<uint16_t>(loc.track);
	seek_ptr.record = static_cast<uint8_t>(loc.record);

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
	if(r < 0) {
		debug_printf("Not operational - drive was unplugged?");
		return error::RESOURCE_UNAVAILABLE;
	}
	return (int)n - (int)dev.irb.scsw.count;
}

/// @brief Writes a single record to a disk
/// @param hdl The disk device
/// @param loc Location of write
/// @param buf Buffer to write
/// @param n Size of write
/// @return int Number of characters written, negative is error
static inline int dasd::write_single(virtual_disk::handle& hdl, const virtual_disk::disk_loc& loc, const void *buf, size_t n)
{
	auto& dev = *css::get_device((css::device::id)((uintptr_t)hdl.driver_data));
	dasd_disk_seek seek_ptr;
	int r;

	auto *req = css::request::create(dev, 4);
	if(req == nullptr) return error::ALLOCATION;

	req->ccws[0].cmd = DASD_CMD_SEEK;
	req->ccws[0].set_addr(&seek_ptr.block);
	req->ccws[0].flags = CSS_CCW_CC;
	req->ccws[0].length = 6;

	req->ccws[1].cmd = DASD_CMD_SEARCH;
	req->ccws[1].set_addr(&seek_ptr.cyl);
	req->ccws[1].flags = CSS_CCW_CC;
	req->ccws[1].length = 5;

	req->ccws[2].cmd = css::cmd::TIC;
	req->ccws[2].set_addr(&req->ccws[1]);
	req->ccws[2].flags = 0x00;
	req->ccws[2].length = 0;
	
	req->ccws[3].cmd = 0x0D; /* Write count key and data */
	req->ccws[3].set_addr(buf);
	req->ccws[3].flags = CSS_CCW_SLI;
	req->ccws[3].length = (uint16_t)n;

	seek_ptr.block = 0;
	seek_ptr.cyl = static_cast<uint16_t>(loc.cylinder);
	seek_ptr.head = static_cast<uint16_t>(loc.track);
	seek_ptr.record = static_cast<uint8_t>(loc.record);
	
	req->send();
	long long timer = 0;
	while(!(req->flags & css::request_flags::DONE)) {
		/** @todo This kludge is a very bad way to do this, we MUST find a way to not do this
		 * like come on we have an spooler why can't we use it? */
		/* ... */
		timer++;
		if(timer >= 0xffff * 16) {
			css::request_perform();
		}
	}
	r = req->retcode;
	css::request::destroy(req);
	if(r != 0) {
		debug_printf("Not operational - drive was unplugged?");
		return error::RESOURCE_UNAVAILABLE;
	}
	return (int)n - (int)dev.irb.scsw.count;
}

int dasd::init(css::device::id id)
{
	debug_printf("\x01\x09 dasd driver");
	auto *driver = virtual_disk::driver::create();
	debug_assert(driver != nullptr);

	/// @brief Writes until it's sucessful
	/// @param dev The disk device
	/// @param diskloc Location to start writting at
	/// @param buf Buffer to write
	/// @param size Size of write
	/// @return int Number of bytes written
	driver->write_disk = [](virtual_disk::handle& hdl, const virtual_disk::disk_loc& diskloc, const void *buf, size_t size) -> int {
		auto loc = diskloc;
		int errcnt = 0;
		debug_printf("\x01\x20 CYL=%i,HEAD=%i,RECORD=%i", (int)loc.cylinder, (int)loc.track, (int)loc.record);
		while(errcnt < 4) {
			int r;
			r = dasd::write_single(hdl, loc, buf, size);
			if(r < 0) {
				errcnt++;
				if(errcnt == 1) {
					loc.record++;
				} else if(errcnt == 2) {
					loc.record = 1;
					loc.track++;
				} else if(errcnt == 3) {
					loc.record = 1;
					loc.track = 0;
					loc.cylinder++;
				}
				continue;
			}
			g_disk_loc = loc;
			return r;
		}
		// Errcnt exceeded 4 tries
		return error::RESOURCE_EXPECTED;
	};
	
	/// @brief Perform a read until it's sucessful (modifying the fdscb as nescesary) 
	/// @param dev Disk device
	/// @param diskloc Location to start reading at
	/// @param buf The buffer to read into
	/// @param size Size of read
	/// @return int Return code
	driver->read_disk = [](virtual_disk::handle& hdl, const virtual_disk::disk_loc& diskloc, void *buf, size_t size) -> int {
		auto loc = diskloc;
		int errcnt = 0;
		debug_printf("\x01\x1F CYL=%i,HEAD=%i,RECORD=%i", (int)loc.cylinder, (int)loc.track, (int)loc.record);
		while(errcnt < 4) {
			int r = dasd::read_single(hdl, loc, buf, size);
			debug_printf("\x01\x1F single DASD record ret=%i,errcnt=%i", r, errcnt);
			if(r < 0) {
				errcnt++;
				if(errcnt == 1) {
					loc.record++;
				} else if(errcnt == 2) {
					loc.record = 1;
					loc.track++;
				} else if(errcnt == 3) {
					loc.record = 1;
					loc.track = 0;
					loc.cylinder++;
				}
				continue;
			}
			loc.record++;
			g_disk_loc = loc;
			return r;
		}
		// Errcnt exceeded 4 tries
		return error::RESOURCE_EXPECTED;
	};

	driver->get_last_disk_loc = [](virtual_disk::handle& hdl) {
		return g_disk_loc;
	};

	char namebuf[8];
	storage::copy(namebuf, "DASD", 4);
	namebuf[4] = '0' + static_cast<char>((g_dasdnum / 100) % 10);
	namebuf[5] = '0' + static_cast<char>((g_dasdnum / 10) % 10);
	namebuf[6] = '0' + static_cast<char>((g_dasdnum / 1) % 10);
	namebuf[7] = '\0';

	auto *node = virtual_disk::node::create("/SYSTEM/DEVICES", namebuf);
	node->driver_data = reinterpret_cast<void *>(static_cast<uintptr_t>(id));
	debug_assert(node != nullptr);
	driver->add_node(*node);
	return 0;
}
