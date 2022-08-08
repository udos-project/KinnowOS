/* zdsfs.c
 *
 */

#include <zdsfs.hxx>
#include <vdisk.hxx>
#include <storage.hxx>
#include <printf.hxx>
#include <locale.hxx>

#include <errcode.hxx>

// ds_driver global for VFS
constinit static virtual_disk::driver *ds_driver;
constinit static virtual_disk::driver *pds_driver;
constinit static storage::dynamic_list<zdsfs::driver_data> g_disks;

namespace zdsfs {
	static inline int get_vtoc_chain_fdscb(virtual_disk::handle& dev, zdsfs::fdscb& fdscb);
	static inline int next_dscb(virtual_disk::handle& dev, zdsfs::fdscb& fdscb, zdsfs::dscb_fmt1& dscb);
	static inline int get_fdscb(virtual_disk::handle& dev, zdsfs::dscb_fmt1& out_fdscb, const char *name);
	static inline int read_file(virtual_disk::handle& dev, zdsfs::dscb_fmt1& zds_fdscb, void *buf, size_t *size);
	static inline int find_free_space(virtual_disk::handle& dev, zdsfs::fdscb& fdscb, int *lastcyl, int *lasthead);
	static inline int new_file(virtual_disk::handle& dev, const char *name);
}

/// @brief Fill the FDSCB with the position of the VTOC
/// @param dev The DASD to locate the VTOC from
/// @param fdscb The FDSCB to fill the data into
/// @return int Nonzero on error
static inline int zdsfs::get_vtoc_chain_fdscb(virtual_disk::handle& dev, zdsfs::fdscb& fdscb)
{
	// The VTOC is at 0:0:3
	// TODO: Find the VTOC dynamically uwu
	constinit static virtual_disk::disk_loc loc = {
		.cylinder = 0,
		.track = 0,
		.record = 3,
	};
	zdsfs::vtoc vtoc;
	int r = dev.read_disk(loc, &vtoc, sizeof(vtoc));
	if(r < 0) {
		debug_printf("Unable to read disk");
		return error::RESOURCE_UNAVAILABLE;
	}
	debug_printf("Read data = %i", sizeof vtoc);

	/* Obtain where the VTOC chain starts */
	fdscb.cyl = vtoc.chain_cyl;
	fdscb.head = vtoc.chain_head;
	fdscb.rec = vtoc.chain_rec;
	debug_printf("Chain CYL=%i,HEAD=%i,REC=%i", (int)fdscb.cyl, (int)fdscb.head, (int)fdscb.rec);
	return 0;
}

/// @brief Obtain the next DSCB
/// @param dev DASD device to read from
/// @param fdscb The current VTOC FDSCB
/// @param dscb The output DSCB
/// @return int Nonzero on error
static inline int zdsfs::next_dscb(virtual_disk::handle& dev, zdsfs::fdscb& fdscb, zdsfs::dscb_fmt1& dscb)
{
	virtual_disk::disk_loc loc = {
		.cylinder = fdscb.cyl,
		.track = fdscb.head,
		.record = fdscb.rec,
	};

	int r = dev.read_disk(loc, &dscb, sizeof(dscb));
	if(r < 0) {
		debug_printf("Unable to read any further");
		return error::RESOURCE_UNAVAILABLE;
	}
	loc = dev.node->driver->get_last_disk_loc(dev);
	fdscb.cyl = loc.cylinder;
	fdscb.head = loc.track;
	fdscb.rec = loc.record;

	if(r >= (int)sizeof(dscb)) {
		debug_printf("Dataset P=%p,DSNAME=%s,CYL=%i,HEAD=%i", &dscb, dscb.name, (int)dscb.start_cc, (int)dscb.start_hh);
		if(dscb.format_id == '1') {
			return 0;
		} else if(dscb.name[0] == '\0') {
			debug_printf("End of VTOC chain found");
			return error::RESOURCE_EXPECTED;
		}
	}
	return 0;
}

static inline int zdsfs::get_fdscb(virtual_disk::handle& dev, zdsfs::dscb_fmt1& dscb, const char *name)
{
	zdsfs::fdscb fdscb;
	int r = zdsfs::get_vtoc_chain_fdscb(dev, fdscb);
	if(r < 0) {
		debug_printf("Can't obtain VTOC chain");
		return error::RESOURCE_UNAVAILABLE;
	}

	virtual_disk::disk_loc loc = {
		.cylinder = fdscb.cyl,
		.track = fdscb.head,
		.record = fdscb.rec,
	};
	while(1) {
		r = dev.read_disk(loc, &dscb, sizeof(dscb));
		if(r < 0)
			return error::RESOURCE_UNAVAILABLE;
		loc = dev.node->driver->get_last_disk_loc(dev);
		fdscb.cyl = loc.cylinder;
		fdscb.head = loc.track;
		fdscb.rec = loc.record;

		if(r >= (int)sizeof(dscb)) {
			debug_printf("Dataset %s", dscb.name);
			if(dscb.format_id == '1') {
				dscb.format_id = ' ';
				if(storage::compare(dscb.name, name, storage_string::length(name)) == 0) {
					debug_printf("Dataset %s @ CYL=%i-%i,HEAD=%i-%i", name, (int)dscb.start_cc, (int)dscb.end_cc, (int)dscb.start_hh, (int)dscb.end_hh);
					break;
				}
			} else if(dscb.name[0] == '\0') {
				return error::RESOURCE_EXPECTED;
			}
		}
	}
	return error::RESOURCE_EXPECTED;
}

static inline int zdsfs::read_file(virtual_disk::handle& dev, zdsfs::dscb_fmt1& zds_fdscb, void *buf, size_t *size)
{
	const zdsfs::fdscb fdscb = {
		.cyl = zds_fdscb.start_cc,
		.head = zds_fdscb.start_hh,
		.rec = 1,
	};
	debug_printf("zdsfs: Reading dataset @ CYL=%i,HEAD=%i,REC=%i", (int)fdscb.cyl, (int)fdscb.head, (int)fdscb.rec);
	
	// Support multi-track files
	virtual_disk::disk_loc loc = {
		.cylinder = zds_fdscb.start_cc,
		.track = zds_fdscb.start_hh,
		.record = 1,
	};
	*size = 0;
	while(1) {
		int r = dev.read_disk(loc, buf, 3450);
		debug_printf("zdsfs: Read ret=%i", r);
		if(r < 0) {
			return error::RESOURCE_UNAVAILABLE;
		}
		// No elements read = end of the dataset
		else if(r == 0) break;
		loc = dev.node->driver->get_last_disk_loc(dev);

		buf = reinterpret_cast<void *>((uintptr_t)buf + (size_t)r);
		*size += (size_t)r;

		// Check if we're finished reading
		if(loc.track > zds_fdscb.end_hh) {
			if(loc.cylinder >= zds_fdscb.end_cc)
				break;
		}
	}
	return 0;
}

/// @brief Finds free space
/// @param dev Disk device containing a zdsfs filesystem
/// @param fdscb FDSCB of the new entry
/// @param lastcyl Upper bound of data cylinder
/// @param lasthead Upper bound of data head
/// @return int Return code
static inline int zdsfs::find_free_space(virtual_disk::handle& dev, zdsfs::fdscb& fdscb, int *lastcyl, int *lasthead)
{
	int r = zdsfs::get_vtoc_chain_fdscb(dev, fdscb);
	if(r < 0) {
		debug_printf("Can't obtain VTOC chain");
		return error::RESOURCE_UNAVAILABLE;
	}

	virtual_disk::disk_loc loc = {
		.cylinder = fdscb.cyl,
		.track = fdscb.head,
		.record = fdscb.rec,
	};

	*lastcyl = 0;
	*lasthead = 0;
	while(1) {
		zdsfs::dscb_fmt1 dscb;
		r = dev.read_disk(loc, &dscb, sizeof(dscb));
		if(r < 0)
			return error::RESOURCE_UNAVAILABLE;
		loc = dev.node->driver->get_last_disk_loc(dev);

		if(r >= (int)sizeof(dscb)) {
			debug_printf("Dataset %s", &dscb.name);
			// Update lastcyl to upper bound
			if(dscb.end_cc > *lastcyl) {
				*lastcyl = dscb.end_cc + 1;
				*lasthead = 0;
			}
			if(dscb.end_hh > *lasthead) {
				*lasthead = dscb.end_hh + 1;
			}
			// Break on end of chain
			if(dscb.name[0] == '\0') return 0;
		}
	}
	// Couldn't find the end of the chain - VTOC probably corrupt
	return error::RESOURCE_EXPECTED;
}

/// @brief Creates a new file
/// Reference implementation from PDOS/370:
/// https://sourceforge.net/p/pdos/gitcode/ci/master/tree/s370/pdos.c#l2591
/// @param dev DASD device
/// @param name Name of the file
/// @return int Return code
static inline int zdsfs::new_file(virtual_disk::handle& dev, const char *name)
{
	debug_printf("zdsfs: creating new dataset %s", name);
	int datacyl, datahead;
	zdsfs::fdscb fdscb;
	int r = zdsfs::find_free_space(dev, fdscb, &datacyl, &datahead);
	if(r < 0) {
		/* No VTOC chain space found */
		debug_printf("zdsfs: No VTOC chain space found");
		return error::RESOURCE_UNAVAILABLE;
	}

	// TODO: Try to make it work without having to set it to zero
	zdsfs::dscb_fmt1 dscb1;
	storage::fill(&dscb1.name,  ' ', sizeof(dscb1.name));
	storage::copy(reinterpret_cast<char *>(&dscb1.name), name, sizeof(dscb1.name));
	dscb1.format_id = '1';
	storage::copy(&dscb1.serial_volume, "UDOS00", sizeof(dscb1.serial_volume));
	dscb1.vol_seq[1] = 1;
	// CREDT
	// EXPDT
	dscb1.n_extents = 1;
	storage::copy(&dscb1.sys_code, "UDOS		", sizeof(dscb1.sys_code));
	dscb1.dsorg = 0x4000; // PS
	dscb1.recfm = 0xC0; // FU
	dscb1.block_size = 3250;
	dscb1.lrecl = 0;
	dscb1.dsind = 0x80; // DSIND80 = Last volume for this dataset
	dscb1.cal1 = 0xC0; // Cylinder request
	dscb1.scal3[2] = 1; // Secondary allocation
	storage::copy(&dscb1.lstar, "\x00\x0E\x01", sizeof(dscb1.lstar));
	storage::copy(&dscb1.trbal, "\x01\x01", sizeof(dscb1.trbal));
	dscb1.ext1[0] = 0x80; // On a cylinder boundary

	/// @todo Use datahead for head-granular allocation
	dscb1.start_cc = (uint16_t)datacyl;
	dscb1.end_cc = (uint16_t)datacyl + 1;
	dscb1.start_hh = (uint16_t)datahead;
	dscb1.end_hh = (uint16_t)datahead;

	/// @todo Writing on the end of the chain means we need to make sure the chain actually
	/// ends after writing this dataset!
	const virtual_disk::disk_loc loc = {
		.cylinder = fdscb.cyl,
		.track = fdscb.head,
		.record = fdscb.rec,
	};
	dev.write_disk(loc, &dscb1, sizeof(dscb1));

	// Register the new dataset to the VFS
	{
		/// @todo Multiple tape support
		auto *node = virtual_disk::node::create("/TAPE", name);
		debug_assert(node != nullptr);
		ds_driver->add_node(*node);

		// Custom per-node data
		node->driver_data = storage::alloc(sizeof(zdsfs::node_data));
		if(node->driver_data == nullptr) {
			/** @todo Destroy node and undo allocations */
			/*virtual_disk::node_destroy(node);*/
			return error::ALLOCATION;
		}
		auto *data = static_cast<zdsfs::node_data *>(node->driver_data);
		data->dscb1 = dscb1; // Save the dscb
		data->driver_data = &g_disks[0]; // Point to current ds_driver
	}
	return 0;
}

int zdsfs::init(virtual_disk::handle& dev)
{
	// Regular dataset driver (files)
	ds_driver = virtual_disk::driver::create();
	debug_assert(ds_driver != nullptr);
	ds_driver->open = [](virtual_disk::handle& hdl) -> int {
		auto* driver_data = storage::alloc<zdsfs::handle_data>(sizeof(zdsfs::handle_data));
		debug_assert(driver_data != nullptr);
		driver_data->seekpos = 0;
		hdl.driver_data = driver_data;
		return 0;
	};
	ds_driver->close = [](virtual_disk::handle& hdl) -> int {
		debug_assert(hdl.driver_data != nullptr);
		storage::free(hdl.driver_data);
		return 0;
	};
	ds_driver->read = [](virtual_disk::handle& hdl, void *buf, size_t n) -> int {
		size_t size;
		debug_assert(buf != nullptr);
		zdsfs::node_data& data = *static_cast<zdsfs::node_data *>(hdl.node->driver_data);
		int r = zdsfs::read_file(*data.driver_data->dev, data.dscb1, buf, &size);
		if(r < 0) {
			debug_printf("Can't read\x01\x11");
			return r;
		}
		/// @todo A better way to transmit size_t stuff safely
		debug_printf("size_of_zdsfs=%u", size);
		return (int)size;
	};
	ds_driver->ioctl = [](virtual_disk::handle& hdl, int cmd, va_list args) -> int {
		auto* hdl_data = reinterpret_cast<zdsfs::handle_data *>(hdl.driver_data);
		switch(cmd) {
		case ZDSFS_IOCTL_FTELL: {
			// We can't return because it will be converted onto an int, so we just
			// copy the long over
			long *num = va_arg(args, long *);
			*num = hdl_data->seekpos;
		} break;
		case ZDSFS_IOCTL_SEEK: {
			long offset = va_arg(args, long);
			int whence = va_arg(args, int);
			switch(whence) {
			case ZDSFS_SEEK_CUR:
				hdl_data->seekpos += offset;
				/** @todo Check bounds */
				break;
			case ZDSFS_SEEK_END:
				/** @todo SEEK_END() w bounds check */
				return error::UNIMPLEMENTED;
			case ZDSFS_SEEK_SET:
				hdl_data->seekpos = offset;
				/** @todo Check bounds */
				break;
			default:
				return -1;
			}
		} break;
		default:
			return -1;
		}
		return 0;
	};

	// Partitioned dataset driver (directories)
	pds_driver = virtual_disk::driver::create();
	debug_assert(pds_driver != nullptr);

	// Create parent mountpoint */
	auto *tape_node = virtual_disk::node::create("/", "RDISK");
	debug_assert(tape_node != nullptr);
	pds_driver->add_node(*tape_node);
	/// @todo Node data for the master /TAPE node

	// Add the ds_driver to the g_disks list
	auto *ds_data = g_disks.insert();
	if(ds_data == nullptr)
		return error::ALLOCATION;
	ds_data->driver = ds_driver;
	ds_data->dev = &dev;

	// Register all of the datasets of the disk onto the VFS
	debug_printf("Adding disk\x01\x11 to the\x01\x12");
	zdsfs::dscb_fmt1 dscb1;
	zdsfs::fdscb fdscb;
	zdsfs::get_vtoc_chain_fdscb(dev, fdscb);
	while(zdsfs::next_dscb(dev, fdscb, dscb1) == 0) {
		char sname[sizeof(dscb1.name) + 1]; // Sanitized name
		storage::copy(sname, dscb1.name, sizeof(dscb1.name));
		sname[sizeof(dscb1.name)] = '\0';
		auto *cptr = storage_string::find_char(sname, ' ');
		if(cptr != nullptr) *cptr = '\0';
		if(storage_string::length(sname) == 0) continue;

		// Convert reserved characters
		for(size_t i = 0; i < storage_string::length(sname); i++) {
			if(sname[i] == '.' || sname[i] == '/' || sname[i] == '\\')
				sname[i] = '$';
		}

		debug_printf("DSCB,LRECL=%i,BLKSIZE=%i,RECFM=%i", (int)dscb1.lrecl, (int)dscb1.block_size, (int)dscb1.recfm);
		debug_printf("DSORG=%i,DSORG=%c", (int)dscb1.dsorg, (char)dscb1.dsorg);
		debug_printf("KEYL=%i", (int)dscb1.keylen);

		auto *ds_node = virtual_disk::node::create(*tape_node, sname);
		if(ds_node == nullptr) return error::ALLOCATION;

		// Custom per-node data
		ds_node->driver_data = storage::alloc(sizeof(zdsfs::node_data));
		if(ds_node->driver_data == nullptr) {
			virtual_disk::node::destroy(*ds_node);
			return error::ALLOCATION;
		}

		auto *data = reinterpret_cast<zdsfs::node_data *>(ds_node->driver_data);
		data->dscb1 = dscb1; // Save the dscb
		data->driver_data = ds_data; // Point to current ds_driver
		if(data->driver_data == nullptr) {
			storage::free(ds_node->driver_data);
			virtual_disk::node::destroy(*ds_node);
			return error::ALLOCATION;
		}
		ds_driver->add_node(*ds_node);
	}

	// Once the nodes are added, time to properly put the driver's functions
	// if we call these while we're registering datasets we will have lots of problems
	// since the PDS drivers creates the dataset on the disk when a new node is added
	// so we can already know it's not very smart to create an already existing file
	pds_driver->_add_node = [](virtual_disk::node& node, virtual_disk::node& child) {
		auto& node_data = *(reinterpret_cast<zdsfs::node_data*>(node.driver_data));
		int r = zdsfs::new_file(*node_data.driver_data->dev, child.name);
		if(r < 0) {
			debug_printf("Can't create\x01\x11");
			return r;
		}
		return 0;
	};
	pds_driver->_remove_node = [](virtual_disk::node&, virtual_disk::node& child) {
		if(child.driver_data != nullptr)
			storage::free(child.driver_data);
		return 0;
	};
	debug_printf("\x01\x0A\x01\x11");
	return 0;
}
