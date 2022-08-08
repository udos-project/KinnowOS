#include <uart.hxx>
#include <vdisk.hxx>
#include <locale.hxx>
#include <errcode.hxx>

int uart::init(void *mmio_base)
{
	debug_printf("\x01\x09 UART driver @ %p", mmio_base);

	auto *driver = virtual_disk::driver::create();
	if(driver == nullptr)
		return error::ALLOCATION;
	
	driver->write = [](virtual_disk::handle& hdl, const void *buf, size_t n) -> int {
		volatile auto *base = reinterpret_cast<volatile uint8_t *>(hdl.node->driver_data);
		volatile auto *data_reg = base + 0x00; // Data register
		for(size_t i = 0; i < n; i++) {
			char ch = reinterpret_cast<const char *>(buf)[i];
			ch = locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>(ch);
			*data_reg = ch;
		}
		*data_reg = locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>('\n');
		return static_cast<int>(n);
	};
	
	virtual_disk::node *node = virtual_disk::node::create("/", "UART");
	if(node == nullptr) return error::ALLOCATION;
	// Store mmio base on the node
	node->driver_data = (void *)mmio_base;

	driver->add_node(*node);
	return 0;
}
