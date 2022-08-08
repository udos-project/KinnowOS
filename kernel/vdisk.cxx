// vfs.c
//
// See UDOS System, Virtual Filesystem

#include <storage.hxx>
#include <printf.hxx>
#include <vdisk.hxx>
#include <locale.hxx>
#include <errcode.hxx>

constinit static storage::global_wrapper<virtual_disk::node> g_root_node;

int virtual_disk::init()
{
	auto& root_node = *(g_root_node.operator->());
	root_node.user_flags = virtual_disk::node_flags::ACCESS
		| virtual_disk::node_flags::ADD_CHILD
		| virtual_disk::node_flags::EXECUTE
		| virtual_disk::node_flags::PRESCENCE
		| virtual_disk::node_flags::READ
		| virtual_disk::node_flags::REMOVE
		| virtual_disk::node_flags::SYSTEM
		| virtual_disk::node_flags::WRITE;
	root_node.group_flags = virtual_disk::node_flags::ACCESS
		| virtual_disk::node_flags::ADD_CHILD
		| virtual_disk::node_flags::EXECUTE
		| virtual_disk::node_flags::PRESCENCE
		| virtual_disk::node_flags::READ
		| virtual_disk::node_flags::REMOVE
		| virtual_disk::node_flags::SYSTEM
		| virtual_disk::node_flags::WRITE;
	root_node.sys_flags = virtual_disk::node_flags::ACCESS
		| virtual_disk::node_flags::ADD_CHILD
		| virtual_disk::node_flags::EXECUTE
		| virtual_disk::node_flags::PRESCENCE
		| virtual_disk::node_flags::READ
		| virtual_disk::node_flags::REMOVE
		| virtual_disk::node_flags::SYSTEM
		| virtual_disk::node_flags::WRITE;
	
	// Base filesystem datasets
	auto *system_node = virtual_disk::node::create("/", "SYSTEM");
	auto *devices_node = virtual_disk::node::create("/SYSTEM", "DEVICES");
	return 0;
}

int virtual_disk::node::add_child(virtual_disk::node& child)
{
	if(this->check_perms(virtual_disk::node_flags::ADD_CHILD) == false)
		return error::UNPRIVILEGED; // No permission

	child.user_flags = this->user_flags;
	child.group_flags = this->group_flags;
	child.sys_flags = this->sys_flags;
	if(this->children.insert(&child) == nullptr)
		return error::ALLOCATION; // Insertion failure
	
	if(this->driver != nullptr) {
		if(this->driver->_add_node != nullptr)
			this->driver->_add_node(*this, child);
	}
	return 0;
}

int virtual_disk::node::remove_child(virtual_disk::node& child)
{
	this->children.remove(&child);

	// Please do not deallocate the node in a remove_node call
	if(this->driver != nullptr) {
		if(this->driver->_remove_node != nullptr)
			this->driver->_remove_node(*this, child);
	}

	// NOTE: It is responsability of the caller to deallocate the node properly!
	return 0;
}

virtual_disk::node *virtual_disk::resolve_path_relative(virtual_disk::node& base_node, const char *path)
{
	auto *root = &base_node;
	const char *tmpbuf = path; // The pointer based off buffer for name comparasions
	// An starting path separator means this is a path that does not employ
	// fast-indexing - so we skip the fast-indexing part
	if(*tmpbuf == '/' || *tmpbuf == '\\' || *tmpbuf == '.') {
		tmpbuf++;
		goto find_file;
	}

	// We will keep recursing the nodes using fast-indexing method
	// (single letter indexing) while there is alphabetical characters
	// on the path (a semicolon must be found)
	while((*tmpbuf >= 'A' && *tmpbuf <= 'Z') || (*tmpbuf >= 'a' && *tmpbuf <= 'z')) {
		const auto letter = virtual_disk::get_drive(tmpbuf);
		
		// Only iterate to next child when the letter is valid
		if((char)letter >= root->children.size()) return nullptr;
		root = root->children[(int)letter];
		tmpbuf++;
	}

	// Colon must proceed after the drive letter (to state that fast-indexing
	// is over and now normal-indexing is used)
	if(*tmpbuf != ':') goto found_file;
	tmpbuf++;

	// Skip the path separator (it is required to have it after the colon)
	if(*tmpbuf != '/' && *tmpbuf != '\\' && *tmpbuf != '.') goto found_file;
	tmpbuf++;

	const char *filename_end;
	size_t filename_len;
find_file:
	if(*tmpbuf == '\0') goto found_file; // File found

	filename_end = storage_string::break_p((char *)tmpbuf, "/\\.");
	filename_len = (filename_end == nullptr) ? storage_string::length(tmpbuf) : (size_t)((ptrdiff_t)filename_end - (ptrdiff_t)tmpbuf);
	
	/// @todo Allow multiple asterisk nodes
	debug_printf("\x01\x12,CHILD=%u,NAME=%s", root->children.size(), root->name);
	for(size_t i = 0; i < root->children.size(); i++) {
		auto& child = *root->children[i];

		// Retarget-control mode, ask the driver for the node instead
		if(child.driver != nullptr && child.driver->request_node != nullptr) {
			// Make path be relative to the current node :-)
			tmpbuf += filename_len;
			if(*tmpbuf == '/' || *tmpbuf == '\\' || *tmpbuf == '.') tmpbuf++;
			/*return child->driver->request_node(child, tmpbuf);*/
			kpanic("return child->driver->request_node(child, tmpbuf);");
		}

		// Must be same length
		if(storage_string::length(child.name) != filename_len) continue;

		/// @todo Case insensitive node search
		if(!storage::compare(tmpbuf, child.name, filename_len)) {
			root = &child;

			// If this is not the end of the filepath then we just advance
			// past the path separator
			tmpbuf += filename_len;
			if(*tmpbuf == '/' || *tmpbuf == '\\' || *tmpbuf == '.') tmpbuf++;
			goto find_file;
		}
	}

	// If none of the children matches then the file wasn't found
	return nullptr;
found_file:
	return root;
}

// Resolve the path and return a node
virtual_disk::node *virtual_disk::resolve_path(const char *path)
{
	auto& root_node = *(g_root_node.operator->());
	debug_printf("DFNODE=%s,PATH=%s", root_node.name, path);
	return virtual_disk::resolve_path_relative(root_node, path);
}

virtual_disk::node *virtual_disk::node::create(const char *path, const char *name)
{
	auto& root_node = *(g_root_node.operator->());
	debug_assert(path != nullptr && name != nullptr);
	// Add children to root node
	auto *root = virtual_disk::resolve_path(path);
	if(root == nullptr) root = &root_node;
	return virtual_disk::node::create(*root, name);
}

virtual_disk::node *virtual_disk::node::create(virtual_disk::node& root, const char *name)
{
	debug_assert(name != nullptr);

	auto *node = storage::allocz<virtual_disk::node>(sizeof(virtual_disk::node));
	if(node == nullptr) return nullptr;

	// Copy the name and truncate it if nescesary
	size_t len = storage_string::length(name);
	if(len > sizeof(node->name)) len = sizeof(node->name);
	storage::copy(node->name, name, len);
	node->name[len] = '\0';

	// Add children to root node
	if(root.add_child(*node) != 0) {
		storage::free(node);
		return nullptr;
	}

	debug_printf("Create node %s", name);
#ifdef DEBUG
	virtual_disk::dump();
#endif
	return node;
}

void virtual_disk::node::destroy(virtual_disk::node& node)
{
	storage::free(&node);
}

bool virtual_disk::node::check_perms(const uint8_t req_bits)
{
	auto user_id = usersys::user::get_current();
	const auto *user = usersys::user::get_by_id(user_id);
	if(user->flags & usersys::user_flags::OVERRIDE_PERMS)
		return true; // Pass

	if(user_id != this->owner_id) {
		if((this->sys_flags & req_bits) != req_bits)
			return false; // Block
		return true; // Pass
	}

	if((this->user_flags & req_bits) != req_bits)
		return false; // Block
	return true; // Pass
}

char virtual_disk::get_drive(const char *path)
{
	char letter = locale::convert<char, locale::charset::NATIVE, locale::charset::EBCDIC_1047>(path[0]);
	// Convert to a number
	if(letter >= 'A' && letter <= 'I')
		letter -= 'A';
	else if(letter >= 'J' && letter <= 'R')
		letter -= 'J';
	else if(letter >= 'S' && letter <= 'Z')
		letter -= 'S';
	return letter;
}

virtual_disk::handle *virtual_disk::handle::open(virtual_disk::node& node, int flags)
{
	if(node.driver == nullptr) {
		debug_printf("Null driver for NODE=%p", &node, node.driver);
		return nullptr;
	}
	
	virtual_disk::handle *hdl = storage::allocz<virtual_disk::handle>(sizeof(virtual_disk::handle));
	if(hdl == nullptr) return nullptr;
	
	hdl->node = &node;
	if(hdl->node->driver->open != nullptr) {
		int r = hdl->node->driver->open(*hdl);
		if(r < 0) {
			debug_printf("\x01\x22OPEN,RC=%i", r);
			storage::free(hdl);
			return nullptr;
		}
	}
	hdl->flags = flags;
	return hdl;
}

virtual_disk::handle *virtual_disk::handle::open_path(const char *path, int flags)
{
	auto *abs_node = virtual_disk::resolve_path(path);
	if(abs_node == nullptr) {
		debug_printf("\x01\x22 PATH=\"%s\"", path);
		return nullptr;
	}
	auto *hdl = virtual_disk::handle::open(*abs_node, flags);
	return hdl;
}

int virtual_disk::handle::close(virtual_disk::handle *hdl)
{
	if(hdl == nullptr)
		return 0;
	
	int r = 0;
	if(hdl->node == nullptr || hdl->node->driver == nullptr)
		return error::INVALID_SETUP;
	
	if(hdl->node->driver->close != nullptr) {
		r = hdl->node->driver->close(*hdl);
		/// @todo Check hdl->driver_data is deallocated
	}
	
	storage::free(hdl);
	return r;
}

int virtual_disk::handle::write(const void *buf, size_t n)
{
	debug_printf("\x01\x12\x01\x20,hdl=%p,buf=%p,n=%u", this, buf, n);
	if(n == 0) return 0; // Nothing to write
	if(this->node == nullptr || this->node->driver == nullptr || this->node->driver->write == nullptr)
		return error::INVALID_SETUP; // No write driver function
	if(this->node->check_perms(virtual_disk::node_flags::WRITE) == false)
		return error::UNPRIVILEGED; // No permission

	// Concat the buffer to our write buffer when we later flush it
	if(this->flags & virtual_disk::mode::BUFFERED) {
		this->write_buf = storage::realloc(this->write_buf, this->write_buf_size + n);
		if(this->write_buf == nullptr) return error::ALLOCATION;
		storage::copy(&reinterpret_cast<char *>(this->write_buf)[this->write_buf_size], buf, n);
		this->write_buf_size += n;
		return static_cast<int>(n);
	}
	// On no-buffer mode the data is directly written instead of being buffered
	// by the handler buffer holders
	else {
		return this->node->driver->write(*this, buf, n);
	}
}

int virtual_disk::handle::read(void *buf, size_t n)
{
	debug_assert(buf != nullptr);
	if(n == 0) return 0; // Nothing to write
	if(this->node == nullptr || this->node->driver == nullptr || this->node->driver->read == nullptr)
		return error::INVALID_SETUP; // No read function
	if(this->node->check_perms(virtual_disk::node_flags::READ) == false)
		return error::UNPRIVILEGED; // No permission
	
	auto r = this->node->driver->read(*this, buf, n);

	// TODO: Use charset
	if((this->flags & virtual_disk::node_flags::TEXT) != 0) // Text translation
		locale::convert<char, locale::charset::ASCII, locale::charset::NATIVE>(reinterpret_cast<char *>(buf));
	return r;
}

int virtual_disk::handle::write_disk(const virtual_disk::disk_loc& loc, const void *buf, size_t n)
{
	if(n == 0) return 0; // Nothing to write
	if(this->node == nullptr || this->node->driver == nullptr || this->node->driver->write_disk == nullptr)
		return error::INVALID_SETUP; // No write driver function
	if(this->node->check_perms(virtual_disk::node_flags::WRITE) == false)
		return error::UNPRIVILEGED; // No permission

	// Concat the buffer to our write buffer when we later flush it
	if(this->flags & virtual_disk::mode::BUFFERED) {
		this->write_buf = storage::realloc(this->write_buf, this->write_buf_size + n);
		if(this->write_buf == nullptr) return error::ALLOCATION;
		storage::copy(&((unsigned char *)this->write_buf)[this->write_buf_size], buf, n);
		this->write_buf_size += n;
		return (int)n;
	}
	// On no-buffer mode the data is directly written instead of being buffered
	// by the handler buffer holders
	else {
		return this->node->driver->write_disk(*this, loc, buf, n);
	}
}

int virtual_disk::handle::read_disk(const virtual_disk::disk_loc& loc, void *buf, size_t n)
{
	debug_assert(buf != nullptr);
	if(n == 0) return 0; // Nothing to write
	if(this->node == nullptr || this->node->driver == nullptr || this->node->driver->read_disk == nullptr)
		return error::INVALID_SETUP; // No read function
	if(this->node->check_perms(virtual_disk::node_flags::READ) == false)
		return error::UNPRIVILEGED; // No permission
	return this->node->driver->read_disk(*this, loc, buf, n);
}

int virtual_disk::handle::ioctl(int cmd, ...)
{
	va_list args;
	va_start(args, cmd);
	int r = this->vioctl(cmd, args);
	va_end(args);
	return r;
}

int virtual_disk::handle::vioctl(int cmd, va_list args)
{
	if(this->node == nullptr || this->node->driver == nullptr || this->node->driver->ioctl == nullptr)
		return error::INVALID_SETUP; // No IOCTL function
	if(this->node->check_perms(virtual_disk::node_flags::WRITE) == false)
		return error::UNPRIVILEGED; // No permission
	return this->node->driver->ioctl(*this, cmd, args);
}

int virtual_disk::handle::flush()
{
	int r = 0;

	// Only flush when there is a write buffer and the mode is buffered
	if(this->flags & virtual_disk::mode::BUFFERED) {
		if(this->write_buf_size == 0 || this->write_buf == nullptr)
			return 0;
		// There must be a write callback
		if(this->node->driver->write == nullptr)
			return error::INVALID_SETUP;
		// Write it all at once in a single call
		r = this->write(this->write_buf, this->write_buf_size);
		storage::free(this->write_buf); // Free up buffer
		this->write_buf = nullptr;
		this->write_buf_size = 0;
	}
	return r;
}

virtual_disk::driver *virtual_disk::driver::create()
{
	auto *driver = reinterpret_cast<virtual_disk::driver *>(storage::allocz(sizeof(virtual_disk::driver)));
	if(driver == nullptr)
		return nullptr;
	return driver;
}

int virtual_disk::driver::add_node(virtual_disk::node& node)
{
	if(this->nodes.insert(&node) == nullptr)
		return error::ALLOCATION; // Insertion failure
	node.driver = this;
	return 0;
}

#if defined DEBUG
void virtual_disk::node::dump(int level)
{
	char ident[16];
	size_t i;
	for(i = 0; i < (size_t)level && i < sizeof(ident) - 1; i++)
		ident[i] = '@';
	ident[i] = '\0';

	debug_printf("%s%s", ident, this->name);

	for(i = 0; i < this->children.size(); i++) {
		auto *child = this->children[i];
		if(child == nullptr) continue;
		child->dump(level + 1);
	}
}

void virtual_disk::dump()
{
	g_root_node->dump(0);
}
#endif
