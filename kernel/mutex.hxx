#ifndef MUTEX_HXX
#define MUTEX_HXX

namespace base {
	typedef volatile int short atomic_int;

	struct mutex {
		enum {
			UNLOCKED = 0,
			LOCKED = 1,
		};

		mutex(mutex& lhs) = delete;
		mutex(const mutex& lhs) = delete;
		mutex(mutex&& lhs) = delete;
		mutex(const mutex&& lhs) = delete;
		
		constexpr mutex() = default;
		~mutex() = default;

		inline bool try_lock()
		{
			if(this->_lock == base::mutex::LOCKED)
				return false;
			this->_lock = base::mutex::LOCKED;
			return true;
		}

		inline void lock()
		{
			while(!this->try_lock()) {
				// Loop until mutex is aquired
			}
		}

		inline void unlock()
		{
			this->_lock = base::mutex::UNLOCKED;
		}

		atomic_int _lock = base::mutex::UNLOCKED;
	};

	struct scoped_mutex {
		scoped_mutex() = delete;
		scoped_mutex(scoped_mutex& lhs) = delete;
		scoped_mutex(const scoped_mutex& lhs) = delete;
		scoped_mutex(scoped_mutex&& lhs) = delete;
		scoped_mutex(const scoped_mutex&& lhs) = delete;

		scoped_mutex(base::mutex& _lock)
			: lock{ _lock }
		{
			this->lock.lock();
		}

		~scoped_mutex()
		{
			this->lock.unlock();
		}

		base::mutex& lock;
	};
}

#endif
