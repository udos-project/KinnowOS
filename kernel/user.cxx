// user.c
//
// Basic user management system which also has groups, generally it does nothing
// on it's own but it should be used to implement some form of ACLs on the VFS
// or to allow certain privileges on a arch dependant basis

// TODO: We are using storage_string::compare which does not bounds check!

#include <user.hxx>
#include <storage.hxx>
#include <printf.hxx>

constinit static storage::global_wrapper<storage::concurrent_dynamic_list<usersys::user>> g_users;
constinit static storage::global_wrapper<storage::concurrent_dynamic_list<usersys::group>> g_groups;
constinit static usersys::user::id g_current_user = 0;

void usersys::init() {
	const auto& users = *(g_users.operator->());
	const auto& groups = *(g_groups.operator->());
	debug_printf("Limit user=%u,group=%u", users.size(), groups.size());
}

usersys::group::id usersys::group::create(const char& name)
{
	auto& groups = *(g_groups.operator->());
	const base::scoped_mutex lock(groups.lock);
	auto *group = groups.insert();
	if(group == nullptr)
		return (usersys::group::id)-1;
	storage_string::copy(group->name, &name);
	return static_cast<usersys::group::id>(groups.size() - 1);
}

usersys::group::id usersys::group::get_by_name(const char& name)
{
	auto& groups = *(g_groups.operator->());
	const base::scoped_mutex lock(groups.lock);
	for(size_t i = 0; i < groups.size(); i++) {
		if(!storage_string::compare(groups[i].name, &name))
			return (usersys::group::id)i;
	}
	return (usersys::group::id)-1;
}

usersys::group *usersys::group::get_by_id(usersys::group::id gid)
{
	auto& groups = *(g_groups.operator->());
	return &groups[gid];
}

usersys::user::id usersys::user::create(const char& name, int flags)
{
	auto& users_list = *(g_users.operator->());
	const base::scoped_mutex lock(users_list.lock);
	auto *user = users_list.insert();
	if(user == nullptr)
		return (usersys::user::id)-1;
	user->flags = flags;
	storage_string::copy(user->name, &name);
	return static_cast<usersys::user::id>(users_list.size() - 1);
}

usersys::user::id usersys::user::get_by_name(const char& name)
{
	auto& users_list = *(g_users.operator->());
	const base::scoped_mutex lock(users_list.lock);
	for(size_t i = 0; i < users_list.size(); i++) {
		if(!storage_string::compare(users_list[i].name, &name))
			return (usersys::user::id)i;
	}
	return (usersys::user::id)-1;
}

usersys::user *usersys::user::get_by_id(usersys::user::id uid)
{
	auto& users = *(g_users.operator->());
	return &users[uid];
}

void usersys::user::set_current(usersys::user::id uid)
{
	g_current_user = uid;
}

usersys::user::id usersys::user::get_current()
{
	return g_current_user;
}
