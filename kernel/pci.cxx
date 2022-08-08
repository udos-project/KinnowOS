#include <pci.hxx>
#include <storage.hxx>
#include <vdisk.hxx>

constinit static storage::global_wrapper<storage::concurrent_dynamic_list<pci::driver>> g_driver_list;
constinit static storage::global_wrapper<storage::concurrent_dynamic_list<pci::device>> g_device_list;

#define VBE_DISPI_INDEX_ID 0
#define VBE_DISPI_INDEX_XRES 1
#define VBE_DISPI_INDEX_YRES 2
#define VBE_DISPI_INDEX_BPP 3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_INDEX_BANK 5
#define VBE_DISPI_INDEX_VIRT_WIDTH 6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET 8
#define VBE_DISPI_INDEX_Y_OFFSET 9

namespace vbe {
	enum dispi_index {
		ID = 0,
		XRES = 1,
		YRES = 2,
		BPP = 3,
		ENABLE = 4,
		BANK = 5,
		VIRT_WIDTH = 6,
		VIRT_HEIGHT = 7,
		X_OFFSET = 8,
		Y_OFFSET = 9,
	};
};

// Versions
#define VBE_DISPI_ID0 0xB0C0
#define VBE_DISPI_ID1 0xB0C1
#define VBE_DISPI_ID2 0xB0C2
#define VBE_DISPI_ID3 0xB0C3
#define VBE_DISPI_ID4 0xB0C4

// Mode flags
#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

namespace pci {
	constinit pci::controller *g_controller = nullptr;
}

static inline uint16_t pci_vga_read_bga(const void *base, uint8_t idx) {
	return *reinterpret_cast<const uint16_t *>(&(reinterpret_cast<const uint8_t *>(base)[idx << 1]));
}

static inline void pci_vga_write_bga(void *base, uint8_t idx, uint16_t val) {
	*reinterpret_cast<uint16_t *>(&reinterpret_cast<uint8_t *>(base)[idx << 1]) = val;
}

pci::device *pci::add_device(pci::device* parent, uint16_t bus, uint16_t slot, uint8_t func)
{
	auto& device_list = *(g_device_list.operator->());

	auto dev = pci::device(*g_controller);
	dev.bus = bus;
	dev.slot = slot;
	dev.func = func;
	dev.parent = parent;
	return device_list.insert(dev);
}

void pci::device::scan_child(uint8_t bus, uint8_t slot, uint8_t func) {
	auto& device_list = *(g_device_list.operator->());
	auto dev = pci::device(*g_controller);
	dev.bus = bus;
	dev.slot = slot;
	dev.func = func;
	dev.parent = this;

	// Check Built-In-Self-Test for starting PCI devices self test, if this fails we are freely allowed
	// to assume that the PCI device has failed, so we discard it like if it never existed
	auto bist = dev.read<uint8_t>(0x0F);
	if(bist & 0x80) {
		// Start the self test
		bist |= 0x40;
		dev.write<uint8_t>(0x0F, bist);
		debug_printf("\x01\x1Dself built-in test");

		// Hold until bit 6 is clear
		// TODO: More dynamic & also don't globally lock while polling!
		unsigned timeout = 0;
		while((bist & 0x40) && timeout != 0xFFFF) {
			bist = dev.read<uint8_t>(0x0F);
			timeout++;
		}
		debug_printf("BIST\x01\x23");

		// Check timeout and that BIST is still in progress, in this case the software
		// is free to assume that the device has failed the BIST. We will also check
		// the lower 4 bits for BIST to check the status code.
		if((timeout == 0xFFFF && (bist & 0x40)) || (bist & 0x0F)) {
			debug_printf("BIST\x01\x22");
			return;
		}
		debug_printf("BIST\x01\x21");
	}

	device_list.insert(dev);
}

void pci::device::scan() {
	// Class 6 means it's a bridge PCI-to-PCI
	if(this->read<uint8_t>(0x0B) == 0x06) {
		this->write<uint8_t>(0x19, 0x00); // Secondary bus
		this->write<uint8_t>(0x1A, 0x00); // Subordinate bus
		this->mem_base = this->controller.mmio;
		this->mem_limit = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this->controller.mmio));
		this->io_base = this->controller.pio;
		this->io_limit = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this->controller.pio));
		this->prefetch_base = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this->controller.mmio) + 0xF000000);
		this->prefetch_limit = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this->controller.mmio) + 0xF000000);
		this->update_ranges();
	}
}

pci::controller::controller(void *_ecam, size_t _ecam_size, void *_mmio, size_t _mmio_size, void *_pio, size_t _pio_size)
	: ecam{ *(reinterpret_cast<pci::ecam *>(_ecam)) },
	ecam_size{ _ecam_size },
	mmio{ _mmio },
	mmio_size{ _mmio_size },
	pio{ _pio },
	pio_size{ _pio_size }
{
	// TODO: WIP
	return;

	g_controller = this;

	auto *root = pci::add_device(nullptr, 0, 0, 0);
	root->scan();
	root->device_id = root->read<uint16_t>(0x00);
	root->vendor_id = root->read<uint16_t>(0x02);
	debug_printf("DEVICE=%x,VENDOR=%x", root->device_id, root->vendor_id);

	auto *vga_card = pci::add_device(root, 0, 1, 0);
	vga_card->device_id = vga_card->read<uint16_t>(0x00);
	vga_card->vendor_id = vga_card->read<uint16_t>(0x02);
	debug_printf("DEVICE=%x,VENDOR=%x", vga_card->device_id, vga_card->vendor_id);

	pci::bar::write(*vga_card, 0 * 4, (void *)(0x4F000000));
	pci::bar::write(*vga_card, 2 * 4, (void *)(0x40000000));
	pci::bar::write(*vga_card, 6 * 4, (void *)(0x40010000));
	auto *bar0_addr = pci::bar::get_address(*vga_card, 0 * 4);
	auto *bar2_addr = pci::bar::get_address(*vga_card, 2 * 4);
	auto *bar6_addr = pci::bar::get_address(*vga_card, 6 * 4);
	debug_printf("BGA,BAR0=%p,BAR2=%p,BAR6=%p", bar0_addr, bar2_addr, bar6_addr);

	auto bga_version = pci_vga_read_bga(bar2_addr, VBE_DISPI_INDEX_ID); // Check version of BGA
	debug_printf("BGA V=%p,BAR0=%p,BAR2=%p,BAR6=%p", static_cast<uintptr_t>(bga_version) & 0xFFFF, bar0_addr, bar2_addr, bar6_addr);
	
	// TODO: Support BGA versions before this one
	if(bga_version < VBE_DISPI_ID2) return;

	const virtual_disk::video_mode vmode = {
		.max_width = 1280,
		.max_height = 800,
		.bitdepth = 8,
		.hz = 0,
	};
	
	debug_printf("VMODE %zux%u,BPP=%u", vmode.max_width, vmode.max_height, vmode.bitdepth);
	
	// Disable VBE extensions
	pci_vga_write_bga(bar2_addr, vbe::dispi_index::ENABLE, VBE_DISPI_DISABLED);
	pci_vga_write_bga(bar2_addr, vbe::dispi_index::XRES, vmode.max_width); // Set desired resolution
	pci_vga_write_bga(bar2_addr, vbe::dispi_index::YRES, vmode.max_height);
	pci_vga_write_bga(bar2_addr, vbe::dispi_index::BPP, vmode.bitdepth); // Set the bit depth
	// Enable stuff back and enable the linear framebuffer
	pci_vga_write_bga(bar2_addr, vbe::dispi_index::ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED | VBE_DISPI_NOCLEARMEM);
}
