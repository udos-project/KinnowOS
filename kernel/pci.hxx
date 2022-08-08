#ifndef PCI_HXX
#define PCI_HXX

#include <storage.hxx>
#include <byteswap.hxx>

#define PCI_MAX_SEGMENTS 65535
#define PCI_MAX_BUSES 256
#define PCI_MAX_SLOTS 256
#define PCI_MAX_FUNCS 8
#define PCI_ECAM_SIZE 4096

namespace pci {
	class ecam {
	public:
		constexpr ecam() = default;
		~ecam() = default;

		inline size_t get_offset(const uint16_t bus, const uint16_t slot, const uint8_t func)
		{
			return (((bus * PCI_MAX_SLOTS) + (slot * PCI_MAX_FUNCS) + func)) * PCI_ECAM_SIZE;
		}

		template<typename T>
		inline T *get_address(const size_t offset)
		{
			return reinterpret_cast<T *>(&(reinterpret_cast<uint8_t *>(this)[offset]));
		}

		template<typename T>
		inline void write(size_t offset, const T value)
		{
			*this->get_address<T>(offset) = cpu_to_le<T>(value);
		}

		template<typename T>
		inline T read(size_t offset)
		{
			return le_to_cpu<T>(*this->get_address<T>(offset));
		}
	};

	class controller {
	public:
		controller(void *ecam, size_t ecam_size, void *mmio, size_t mmio_size, void *pio, size_t pio_size);

		pci::ecam& ecam;
		size_t ecam_size;
		void *mmio;
		size_t mmio_size;
		void *pio;
		size_t pio_size;
	};
	extern pci::controller *g_controller;

	class device;
	class driver {
	public:
		using id = uint8_t;

		constexpr driver() = default;
		~driver() = default;

		int (*add_self)(pci::device& root, uint16_t bus, uint16_t slot, uint8_t func);
		int (*remove_self)(pci::device& root);
		int (*add_child)(pci::device& root, pci::device& child);
		int (*remove_child)(pci::device& root, pci::device& child);
	};

	class device {
	public:
		constexpr device(pci::controller& _controller)
			: controller{ _controller }
		{

		}
		constexpr device& operator=(const device& dev) {
			storage::copy(this, &dev, sizeof(device));
			return *this;
		}
		~device() = default;

		// Topology location
		uint16_t bus;
		uint16_t slot;
		uint8_t func;

		// Cached data
		uint8_t classcode;
		uint8_t subclass;
		uint8_t prog_if;
		uint16_t vendor_id;
		uint16_t device_id;

		uint8_t secondary_bus;
		uint8_t subordinate_bus;

		// Baselimits for BAR allocation
		void *mem_base;
		void *mem_limit;
		void *io_base;
		void *io_limit;
		void *prefetch_base;
		void *prefetch_limit;

		pci::device *parent; // Parent
		pci::driver::id driver_id; // Driver for this device
		pci::controller& controller;

		void scan_child(uint8_t bus, uint8_t slot, uint8_t func);
		void scan();

		/// @brief Updates the ranges of this device
		inline void update_ranges() const
		{
			debug_printf("URANGE IO=%p-%p,MMIO=%p-%p,PRFTCHMI=%p-%p", io_base, io_limit, mem_base, mem_limit, prefetch_base, prefetch_limit);
			this->write<uint8_t>(0x1C, (reinterpret_cast<uintptr_t>(this->io_base) >> 16) & 0xFF);
			this->write<uint8_t>(0x1D, (reinterpret_cast<uintptr_t>(this->io_limit) >> 16) & 0xFF);
			this->write<uint16_t>(0x30, (reinterpret_cast<uintptr_t>(this->io_base) >> 24) & 0xFFFF);
			this->write<uint16_t>(0x32, (reinterpret_cast<uintptr_t>(this->io_limit) >> 24) & 0xFFFF);
			this->write<uint16_t>(0x20, (reinterpret_cast<uintptr_t>(this->mem_base) >> 16) & 0xFFFF);
			this->write<uint16_t>(0x22, (reinterpret_cast<uintptr_t>(this->mem_limit) >> 16) & 0xFFFF);
			this->write<uint16_t>(0x24, (reinterpret_cast<uintptr_t>(this->prefetch_base) >> 16) & 0xFFFF);
			this->write<uint16_t>(0x26, (reinterpret_cast<uintptr_t>(this->prefetch_limit) >> 16) & 0xFFFF);
			this->write<uint32_t>(0x28, (reinterpret_cast<uintptr_t>(this->prefetch_base) >> 32) & 0xFFFFFFFF);
			this->write<uint32_t>(0x2C, (reinterpret_cast<uintptr_t>(this->prefetch_limit) >> 32) & 0xFFFFFFFF);
		}

		template<typename T>
		inline void write(size_t offset, const T value) const
		{
			auto base = this->controller.ecam.get_offset(this->bus, this->slot, this->func);
			debug_printf("WRITE ECAM=%p,ADDR=%x+%x", &this->controller.ecam, base, offset);
			this->controller.ecam.write<T>(base + offset, value);
		}

		template<typename T>
		inline T read(size_t offset) const
		{
			auto base = this->controller.ecam.get_offset(this->bus, this->slot, this->func);
			debug_printf("READ ECAM=%p,ADDR=%x+%x", &this->controller.ecam, base, offset);
			return this->controller.ecam.read<T>(base + offset);
		}
	};

	class bar {
	public:
		enum flags {
			PIO = 1 << 0,
			MMIO_16 = 1 << 1,
			MMIO_32 = 0 << 1,
			MMIO_64 = 2 << 1,
			PREFETCH = 3 << 1,
		};

		constexpr bar() = default;
		~bar() = default;

		/// @brief Get the addr object
		/// @return void* The returned address
		static inline void *get_address(pci::device& dev, int offset)
		{
			uint64_t raw = static_cast<uint64_t>(le_to_cpu32(dev.read<uint32_t>(offset)));
			if(raw & pci::bar::flags::PIO) { // Bit 0 is set when it's an I/O BAR
				return reinterpret_cast<void *>(raw & (~0b11));
			} else {
				// Whetever prefetch is set
				const uint8_t type = raw & 0b110; // Obtain the type
				switch(type) {
				case pci::bar::MMIO_16: // 16-bits
					return reinterpret_cast<void *>(raw & 0xFFF0);
				case pci::bar::MMIO_32: // 32-bits
					return reinterpret_cast<void *>(raw & 0xFFFFFFF0);
				case pci::bar::MMIO_64: { // 64-bits
					raw = le_to_cpu64(dev.read<uint64_t>(offset));
					return reinterpret_cast<void *>(raw & 0xFFFFFFFFFFFFFFF0);
				};
				default: break;
				}
			}
			return nullptr; // Not a valid BAR
		}

		static inline void write(pci::device& dev, int offset, void *new_addr)
		{
/*#ifdef DEBUG
			const auto align = pci::bar::get_align(dev, offset);
			debug_printf("ALIGN=%x,ADDR=%p", align, new_addr);
			debug_assert(reinterpret_cast<uintptr_t>(new_addr) & align); // Address should be aligned
#endif*/
			const uint8_t type = dev.read<uint32_t>(0x10 + offset) & 0b110;
			switch(type) {
			case pci::bar::MMIO_16: // 16-bits
				dev.write<uint16_t>(0x10 + offset, reinterpret_cast<uintptr_t>(new_addr));
				break;
			case pci::bar::MMIO_32: // 32-bits
				dev.write<uint32_t>(0x10 + offset, reinterpret_cast<uintptr_t>(new_addr));
				break;
			case pci::bar::MMIO_64: // 64-bits
				dev.write<uint64_t>(0x10 + offset, reinterpret_cast<uintptr_t>(new_addr));
				break;
			default: break;
			}
		}

		static inline size_t get_align(pci::device& dev, int offset)
		{
			uint64_t original = static_cast<uint64_t>(dev.read<uint32_t>(0x10 + offset));
			const uint8_t type = original & 0b110;
			size_t align = 0;
			switch(type) {
			case pci::bar::MMIO_16: // 16-bits
				dev.write<uint32_t>(0x10 + offset, 0xFFFF);
				align = static_cast<size_t>(dev.read<uint32_t>(0x10 + offset));
				dev.write<uint32_t>(0x10 + offset, static_cast<uint32_t>(original));
				break;
			case pci::bar::MMIO_32: // 32-bits
				dev.write<uint32_t>(0x10 + offset, 0xFFFFFFFF);
				align = static_cast<size_t>(dev.read<uint32_t>(0x10 + offset));
				dev.write<uint32_t>(0x10 + offset, static_cast<uint32_t>(original));
				break;
			case pci::bar::MMIO_64: { // 64-bits
				original = dev.read<uint64_t>(0x10 + offset);
				dev.write<uint64_t>(0x10 + offset, 0xFFFFFFFFFFFFFFFF);
				align = static_cast<size_t>(dev.read<uint64_t>(0x10 + offset));
				dev.write<uint64_t>(0x10 + offset, original);
			} break;
			default: break;
			}
			debug_assert(align != 0);
			return align;
		}
	};

	pci::device *add_device(pci::device* parent, uint16_t bus, uint16_t slot, uint8_t func);
	int init(void *ecam, size_t ecam_size, void *mmio, size_t mmio_size, void *pio, size_t pio_size);
}

#endif
