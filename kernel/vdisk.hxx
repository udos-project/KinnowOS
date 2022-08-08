#ifndef VIRTUAL_DISK_HXX
#define VIRTUAL_DISK_HXX

#include <stdarg.h>
#include <types.hxx>
#include <storage.hxx>
#include <user.hxx>
#include <locale.hxx>

namespace virtual_disk {
	struct fdscb;
	struct node;
	struct handle;
	struct driver;

	struct disk_loc {
		unsigned short cylinder;
		unsigned short track;
		unsigned short record;
	};
	
	struct video_mode {
		unsigned int max_width;
		unsigned int max_height;
		unsigned char bitdepth;
		unsigned char hz;

		enum flags {
			NONE = 0x00,
		};
		enum flags flags;
	};

	struct mode {
		static constexpr int READ = 0x01;
		static constexpr int WRITE = 0x02;
		static constexpr int BUFFERED = 0x04;
	};

	namespace node_flags {
		enum node_flags {
			WRITE = 0x01, // Write
			READ = 0x02, // Read
			ACCESS = 0x04, // Access for dataset/directory
			PRESCENCE = 0x08, // If this dataset will show up on directory listings
			EXECUTE = 0x10, // Execution
			SYSTEM = 0x20, // System marked nodes
			REMOVE = 0x40, // Node allowed to be removed
			ADD_CHILD = 0x80, // If the node can be add childs
			TEXT = 0x100, // Treat dataset as text
		};
	};

	/* A node representing a directory (has children) or a file (no children) */
	class node {
	public:
		constexpr node() = default;
		~node() = default;
		node& operator=(node&) = delete;
		const node& operator=(const node&) = delete;

		static virtual_disk::node *create(const char *path, const char *name);
		static virtual_disk::node *create(virtual_disk::node& root, const char *name);
		static void destroy(virtual_disk::node& node);
		int add_child(virtual_disk::node& child);
		int remove_child(virtual_disk::node& child);
		bool check_perms(const uint8_t req_bits);
#if defined DEBUG
		void dump(int level);
#endif
		unsigned char flags = 0;
		storage::dynamic_list<virtual_disk::node *> children;
		virtual_disk::driver *driver = nullptr;
		void *driver_data = nullptr;
		usersys::user::id owner_id = 0;
		uint8_t user_flags = 0; // Owner's flags
		uint8_t group_flags = 0; // Group-relative flags
		uint8_t sys_flags = 0; // Systemwide flags
		char name[24] = { '\0' };
	};

	// Handle for opening a node, not viewed by the caller and only used internally
	// by the functions below
	class handle {
	public:
		constexpr handle() = default;
		~handle() = default;
		handle& operator=(handle&) = delete;
		const handle& operator=(const handle&) = delete;

		static struct virtual_disk::handle *open(virtual_disk::node& node, int flags);
		static struct virtual_disk::handle *open_path(const char *path, int flags);
		static int close(struct virtual_disk::handle *hdl);
		int write(const void *buf, size_t n);
		int read(void *buf, size_t n);
		int write_disk(const virtual_disk::disk_loc& loc, const void *buf, size_t n);
		int read_disk(const virtual_disk::disk_loc& loc, void *buf, size_t n);
		int ioctl(int cmd, ...);
		int vioctl(int cmd, va_list args);
		int flush();

		virtual_disk::node *node = nullptr;
		int mode = 0;
		void *read_buf = nullptr;
		size_t read_buf_size = 0;
		void *write_buf = nullptr;
		size_t write_buf_size = 0;
		locale::charset cset = locale::charset::NATIVE;
		int flags = 0;
		// If used, driver is responsible for allocation/deallocation
		void *driver_data = 0;
	};

	// Manage nodes via ownership - This is used so drivers can register nodes and
	// when they need to terminate they can just destroy their registered nodes
	// without worrying about leftovers
	class driver {
	public:
		constexpr driver() = default;
		~driver() = default;
		driver& operator=(driver&) = delete;
		const driver& operator=(const driver&) = delete;

		static virtual_disk::driver *create();
		int add_node(virtual_disk::node& node);

		storage::dynamic_list<virtual_disk::node *> nodes;

		int (*open)(virtual_disk::handle& hdl) = nullptr;
		int (*close)(virtual_disk::handle& hdl) = nullptr;
		int (*write)(virtual_disk::handle& hdl, const void *buf, size_t n) = nullptr;
		int (*read)(virtual_disk::handle& hdl, void *buf, size_t n) = nullptr;
		int (*flush)(virtual_disk::handle& hdl) = nullptr;
		int (*ioctl)(virtual_disk::handle& hdl, int cmd, va_list args) = nullptr;
		
		// Device dependant options
		
		//
		// Traditional seeking disk
		//
		int (*write_disk)(virtual_disk::handle& hdl, const virtual_disk::disk_loc& loc, const void *buf, size_t size) = nullptr;

		/// @brief Callback to perform a read on a disk
		/// @param hdl Handle
		/// @param loc Location of read
		/// @param buf Buffer
		/// @param size Size of read
		/// @return int Return code, negative on failure
		int (*read_disk)(virtual_disk::handle& hdl, const virtual_disk::disk_loc& loc, void *buf, size_t size) = nullptr;
		int (*seek_disk)(virtual_disk::handle& hdl, const virtual_disk::disk_loc& loc) = nullptr;
		virtual_disk::disk_loc (*get_last_disk_loc)(virtual_disk::handle& hdl) = nullptr;

		///
		/// Video (framebuffer is treated as block device)
		/// 
		virtual_disk::video_mode *(*get_modes)(virtual_disk::handle& hdl) = nullptr;

		/// @brief Callback to call when adding a new node, useful to allocate
		/// driver_data in this stage.
		int (*_add_node)(virtual_disk::node& node, virtual_disk::node& child) = nullptr;

		/// @brief Callback to call when removing a node, do not deallocate the
		/// node here, it is the caller's responsability to do so - only deallocate
		/// the resources your driver has allocated (driver_data).
		int (*_remove_node)(virtual_disk::node& node, virtual_disk::node& child) = nullptr;

		/// @brief Retargeted-control handlers
		virtual_disk::node *(*request_node)(virtual_disk::node& node, const char *path) = nullptr;

		friend class virtual_disk::node;
	};

	int init();
	virtual_disk::node *resolve_path_relative(virtual_disk::node& base_node, const char *path);
	virtual_disk::node *resolve_path(const char *path);

	char get_drive(const char *path);
#ifdef DEBUG
	void dump();
#endif
}

#endif
