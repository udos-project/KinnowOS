#ifndef USER_HXX
#define USER_HXX

#include <types.hxx>
#include <storage.hxx>

namespace virtual_disk {
	struct handle;
};

namespace usersys {
	struct user;
	struct group;
	struct session;

	void init();

	namespace user_flags {
		enum user_flags {
			OVERRIDE_PERMS, // Override all permissions checks
		};
	};

	struct user {
		using id = int8_t;
		static usersys::user::id create(const char& name, int flags);
		static usersys::user::id get_by_name(const char& name);
		static usersys::user *get_by_id(usersys::user::id uid);
		static void set_current(usersys::user::id uid);
		static usersys::user::id get_current();

		char name[8];
		int flags;
		storage::concurrent_dynamic_list<virtual_disk::handle *> handles; // Open VFS handles
	};

	struct group {
		using id = int8_t;
		static usersys::group::id create(const char& name);
		static usersys::group::id get_by_name(const char& name);
		static usersys::group *get_by_id(usersys::group::id gid);

		storage::concurrent_dynamic_list<usersys::user::id *> users;
		char name[8];
	};
}

#endif
