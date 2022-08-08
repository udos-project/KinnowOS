#include <arch/asm.hxx>
#include <printf.hxx>
#include <timeshr.hxx>
#include <vdisk.hxx>
#include <user.hxx>
#include <service.hxx>
#include <s390/css.hxx>

arch_dep::register_t service::common(const uint16_t code, const arch_dep::register_t arg1, const arch_dep::register_t arg2, const arch_dep::register_t arg3, const arch_dep::register_t arg4) {
	auto *job = timeshare::get_current_job();
	auto *user = usersys::user::get_by_id(job->user_id);

	debug_printf("UDOS_SVC_ID=%u", static_cast<size_t>(arg4));
	if(code == SVC_SCHED_YIELD) {
		timeshare::schedule();
	} else if(code == SVC_ABEND) {
		/// @todo Terminate task
		debug_printf("todo: terminate tasks");
	} else if(code == SVC_PRINT_DEBUG) {
		kprintf("%s", reinterpret_cast<const char *>(job->virtual_to_real((void *)arg1)));
	} else if(code == SVC_GET_STORAGE) {
		const auto size = static_cast<size_t>(arg1);
		auto *p = real_storage::alloc(size, virtual_storage::page_align);
		if(p != nullptr) {
			auto *aligned_p = reinterpret_cast<void *>((uintptr_t)p & (~0xfff));
			job->map_range(aligned_p, aligned_p, 0, size);
		}
		return (arch_dep::register_t)p;
	} else if(code == SVC_DROP_STORAGE) {
		storage::free(job->virtual_to_real((void *)arg1));
	} else if(code == SVC_RESIZE_STORAGE) {
		const auto size = static_cast<size_t>(arg2);
		void *old_p = (void *)job->virtual_to_real((void *)arg1);
		void *new_p = real_storage::realloc(old_p, size, virtual_storage::page_align);
		if(new_p != nullptr) {
			auto *aligned_p = reinterpret_cast<void *>((uintptr_t)new_p & (~0xfff));
			job->map_range(aligned_p, aligned_p, 0, size);
		}
		return (arch_dep::register_t)new_p;
	} else if(code == SVC_VFS_OPEN) {
		const auto *path = reinterpret_cast<const char *>(job->virtual_to_real(reinterpret_cast<void *>(arg1)));
		int flags = (int)arg2;
		auto *hdl = virtual_disk::handle::open_path(path, flags);
		debug_printf("OPEN,FILE=%s(%p),FLAGS=%i,RET=%p", path, path, flags, (uintptr_t)arg4);
		return static_cast<arch_dep::register_t>(([&hdl](auto& list) -> size_t {
			// Add to list of handles
			for(size_t i = 0; i < list.size(); i++) {
				if(list[i] != nullptr) {
					list[i] = hdl;
					return i;
				}
			}
			list.insert(hdl);
			return list.size() - 1;
		})(user->handles));
	} else if(code == SVC_VFS_CLOSE || code == SVC_VFS_WRITE || code == SVC_VFS_READ || code == SVC_VFS_FLUSH || code == SVC_VFS_IOCTL || code == SVC_VFS_NODE_COUNT || code == SVC_VFS_FILLDIR) {
		const auto hdl_idx = static_cast<size_t>(arg1);
		if(hdl_idx >= user->handles.size())
			return static_cast<arch_dep::register_t>(-1);

		auto *hdl = user->handles[hdl_idx];
		int r = -1;
		if(code == SVC_VFS_CLOSE) {
			storage::fast_remove(user->handles, hdl_idx);
			r = hdl->close(hdl);
		} else if(code == SVC_VFS_WRITE) {
			r = hdl->write((void *)arg2, (size_t)arg3);
		} else if(code == SVC_VFS_READ) {
			r = hdl->read((void *)arg2, (size_t)arg3);
		} else if(code == SVC_VFS_FLUSH) {
			r = hdl->flush();
		} else if(code == SVC_VFS_IOCTL) {
			r = hdl->vioctl((int)arg2, *((va_list *)arg3));
		} else if(code == SVC_VFS_NODE_COUNT) {
			r = (int)hdl->node->children.size();
		} else if(code == SVC_VFS_FILLDIR) {
			auto *node_tab = (virtual_disk::node *)arg2;
		}
		debug_printf("ACTION,FILE=%s(%p),RET=%i", hdl->node->name, hdl, r);
		return (arch_dep::register_t)r;
	} else if(code == SVC_VFS_ADD_NODE) {
		const auto *path = reinterpret_cast<const char *>(job->virtual_to_real((void *)arg1));
		const auto *name = reinterpret_cast<const char *>(job->virtual_to_real((void *)arg2));
		virtual_disk::node::create(path, name);
	} else if(code == SVC_VFS_REMOVE_NODE) {
		const auto *parent_path = (const char *)job->virtual_to_real((void *)arg1);
		const auto *child_path = (const char *)job->virtual_to_real((void *)arg2);
		auto *parent = virtual_disk::resolve_path(parent_path);
		if(parent == nullptr)
			return (arch_dep::register_t)-1;

		auto *child = virtual_disk::resolve_path_relative(*parent, child_path);
		if(child == nullptr)
			return (arch_dep::register_t)-1;

		int r = parent->remove_child(*child);
		if(r < 0)
			return (arch_dep::register_t)r;
		virtual_disk::node::destroy(*child);
	}
	
#ifdef TARGET_S390
	if(code == SVC_CSS_GET_DEV) {
		css::schid schid = { (uint16_t)((arg1 >> 16) & 0xffff), (uint16_t)(arg1 & 0xffff) };
		auto *dev = css::get_device(schid);
		// TODO: Don't expose dev like this to the application!
		if(dev != nullptr) {
			auto *aligned_p = reinterpret_cast<void *>((uintptr_t)dev & (~0xfff));
			job->map_range(aligned_p, aligned_p, 0, 4096);
		}
		debug_printf("svc.dev=%p", dev);
		return (arch_dep::register_t)dev;
	} else if(code == SVC_CSS_REQ_CREAT) {
		auto *dev = reinterpret_cast<css::device *>(arg1);
		const auto n_ccws = static_cast<size_t>(arg2);
		auto *req = css::request::create(*dev, n_ccws);
		// TODO: Don't expose req like this to the application!
		if(req != nullptr) {
			auto *aligned_p = reinterpret_cast<void *>((uintptr_t)req & (~0xfff));
			job->map_range(aligned_p, aligned_p, 0, 4096);
		}
		debug_printf("svc.dev=%p,p=%p,id=%i,num=%i", dev, req, (int)req->schid.id, (int)req->schid.num);
		return (arch_dep::register_t)req;
	} else if(code == SVC_CSS_REQ_SEND) {
		auto *req = (css::request *)arg1;
		debug_printf("svc flags=%u,p=%p,id=%i,num=%i", (unsigned int)req->flags, req, (int)req->schid.id, (int)req->schid.num);
		req->send();
	} else if(code == SVC_CSS_REQ_DELETE) {
		auto *req = (css::request *)arg1;
		css::request::destroy(req);
	} else if(code == SVC_CSS_REQ_XCHG_FLAGS) {
		auto *req = (css::request *)arg1;
		int flags = (int)arg2;
		req->flags = static_cast<unsigned char>(flags);
	} else if(code == SVC_CSS_REQ_SET_CCW) {
		auto *req = reinterpret_cast<css::request *>(arg1);
		struct css_req_set_ccw_data *data = (struct css_req_set_ccw_data *)arg2;

		debug_printf("DATA,ccw_num=%i,accw_num=%i,cmd=%i,f=%i,sz=%u", data->ccw_num, data->addr_ccw_num, data->cmd, data->flags, data->size);

		// Must be a valid CCW number
		if(data->ccw_num >= (int)req->ccws.size())
			return (arch_dep::register_t)-1;

		auto& ccw = req->ccws[data->ccw_num];
		// Self reference another CCW if inside bounds
		if((size_t)data->addr_ccw_num < req->ccws.size()) {
			ccw.set_addr(&req->ccws[data->addr_ccw_num]);
		} else {
			if(data->addr != nullptr) {
				ccw.set_addr(job->virtual_to_real(data->addr));
			} else {
				ccw.set_addr((void *)nullptr);
			}
		}
		ccw.cmd = static_cast<uint8_t>(data->cmd); // The rest is just copying
		ccw.flags = static_cast<uint8_t>(data->flags);
		ccw.length = static_cast<uint16_t>(data->size);
	} else if(code == SVC_CSS_REQ_AWAIT) {
		auto *req = reinterpret_cast<css::request *>(arg1);
		timeshare::schedule(); // Yield to other threads while sleeping
		return (req->flags & css::request_flags::DONE) ? 1 : 0;
	} else if(code == SVC_CSS_REQ_GET_STATUS) {
		auto *req = (css::request *)arg1;
		return static_cast<arch_dep::register_t>(req->retcode);
	} else if(code == SVC_CSS_DEVICE_QUERY) {
		auto *dev = reinterpret_cast<css::device *>(arg1);
		int query = (int)arg2;
		if(query == SVC_CSS_DEVICE_QUERY_SCSW_COUNT) {
			return static_cast<arch_dep::register_t>(dev->schib.scsw.count);
		}
	}
#endif
	
	if(code == SVC_EXEC_AT) {
		auto *entry = job->virtual_to_real((void *)arg1);
		auto *name = reinterpret_cast<char *>(job->virtual_to_real((void *)arg2));
		auto *new_task = timeshare::task::create(*job, *name);
		auto *new_thread = timeshare::thread::create(*job, *new_task, 8192);
		new_thread->set_pc(entry, false);
	} else if(code == SVC_THREAD_AT) {
		auto *entry = job->virtual_to_real((void *)arg1);
		auto *new_task = &job->tasks[job->current_task];
		auto *new_thread = timeshare::thread::create(*job, *new_task, 8192);
		new_thread->set_pc(entry, false);
	} else if(code == SVC_GET_PDB) {
		auto *task = &job->tasks[job->current_task];

		// Query the real storage
		/// @todo partition storage for tasks!
		real_storage::stats rs_stats;
		real_storage::get_stats(&rs_stats);
		task->pdb.pdb_free_bytes = rs_stats.free_size;
		task->pdb.pdb_used_bytes = rs_stats.used_size;

		auto *dest_area = (void *)arg1; // Final area to place PDB at
		storage::copy(dest_area, &task->pdb, sizeof(task->pdb));
	} else {
		kpanic("Invalid syscall=%u", (size_t)code);
	}
	return 0;
}
