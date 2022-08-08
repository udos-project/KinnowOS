#ifndef STORAGE_HXX
#define STORAGE_HXX

#include <types.hxx>
#include <arch/asm.hxx>
#include <printf.hxx>
#include <mutex.hxx>
#include <errcode.hxx>

#define abs(x) (((x) < 0) ? (-x) : (x))

namespace storage {
	template<typename T1 = void, typename T2 = void>
	constexpr void *copy(T1 *dest, const T2 *src, size_t n)
	{
		const auto *c_src = reinterpret_cast<const uint8_t *>(src);
		auto *c_dest = reinterpret_cast<uint8_t *>(dest);
		// Check for overlapping argument pointers - copy wasn't made for overlapped storage
		debug_assert((size_t)abs((ptrdiff_t)dest - (ptrdiff_t)src) > n);
		while(n) {
			*(c_dest++) = *(c_src++);
			--n;
		}
		return dest;
	}

	template<typename T1 = void, typename T2 = void>
	constexpr void *move(T1 *dest, const T2 *src, size_t n)
	{
		const auto *c_src = reinterpret_cast<const uint8_t *>(src);
		auto *c_dest = reinterpret_cast<uint8_t *>(dest);
		if((uintptr_t)c_dest < (uintptr_t)c_src) {
			while(n) {
				*(c_dest++) = *(c_src++);
				--n;
			}
		} else {
			c_dest += n;
			c_src += n;
			while(n) {
				*(c_dest--) = *(c_src--);
				--n;
			}
		}
		return c_dest;
	}

	template<typename T>
	constexpr void *fill(T *s, char c, size_t n)
	{
		auto *c_s = reinterpret_cast<uint8_t *>(s);
		while(n) {
			*(c_s++) = c;
			--n;
		}
		return s;
	}

	template<typename T1, typename T2>
	constexpr int compare(const T1 *s1, const T2 *s2, size_t n)
	{
		const auto *_s1 = reinterpret_cast<const uint8_t *>(s1);
		const auto *_s2 = reinterpret_cast<const uint8_t *>(s2);
		int diff = 0;
		while(n) {
			diff += *(_s1++) - *(_s2++);
			--n;
		}
		return diff;
	}
}

namespace storage_string {
	constexpr size_t length(const char *s)
	{
		size_t i = 0;
		while(s[i] != '\0') i++;
		return i;
	}

	constexpr int compare(const char *s1, const char *s2)
	{
		size_t n = 0;
		int diff = 0;
		while(n && *s1 != '\0' && *s2 != '\0') {
			diff += *(s1++) - *(s2++);
			n++;
		}

		if(*s1 != *s2) return -1;
		return diff;
	}

	constexpr char *find_char(const char *s, char c)
	{
		while(*s != '\0' && *s != c)
			++s;
		if(*s == '\0') return nullptr;
		return (char *)s;
	}

	constexpr size_t span(const char *s, const char *accept)
	{
		size_t spn = 0;
		while(*s != '\0') {
			char is_match = 0;
			for(size_t i = 0; i < storage_string::length(accept); i++) {
				if(*s != accept[i]) continue;
				is_match = 1;
				++spn;
				break;
			}
			if(!is_match) return spn;
			++s;
		}
		return spn;
	}

	constexpr char *break_p(char *s, const char *accept)
	{
		while(*s != '\0') {
			for(size_t i = 0; i < storage_string::length(accept); i++) {
				if(*s == accept[i]) return s;
			}
			++s;
		}
		return nullptr;
	}

	constexpr char *copy(char *s1, const char *s2)
	{
		while(*s2 != '\0')
			*(s1++) = *(s2++);
		*(s1++) = '\0';
		return s1;
	}

	constexpr char *concat(char *s1, const char *s2)
	{
		while(*s1 != '\0')
			++s1;
		while(*s2 != '\0')
			*(s1++) = *(s2++);
		*(s1++) = '\0';
		return s1;
	}
}

#include <real.hxx>

namespace storage {
	template<typename T>
	constexpr size_t safe_align(T * = nullptr) noexcept
	{
		return alignof(T);
	}

	template<>
	constexpr size_t safe_align(void *) noexcept
	{
		return 0;
	}

	/// @brief Allocates storage for an object
	/// @tparam T Type of the object
	/// @param size Size of the object
	/// @param align Alignment
	/// @return T* Allocated storage
	template<typename T = void>
	ATTRIB_MALLOC static inline T *alloc(size_t size, size_t align = storage::safe_align<T>())
	{
		if(size == 0) return nullptr;
		T *ptr = static_cast<T *>(real_storage::alloc(size, align));
		return ptr;
	}

	/// @brief Allocate an array
	/// @tparam T Type of the object
	/// @param n Number of objects
	/// @param size Size of each object
	/// @param align Alignment
	/// @return T* Allocated storage 
	template<typename T = void>
	ATTRIB_MALLOC static inline T *alloca(size_t n, size_t size, size_t align = storage::safe_align<T>())
	{
		return storage::alloc<T>(n * size, align);
	}

	/// @brief Allocate and clear storage for object
	/// @tparam T Type of the object
	/// @param size Size of the object
	/// @param align Alignment
	/// @return T* Allocated storage 
	template<typename T = void>
	ATTRIB_MALLOC static inline T *allocz(size_t size, size_t align = storage::safe_align<T>())
	{
		T *ptr = storage::alloc<T>(size, align);
		if(ptr == nullptr) return nullptr;
		storage::fill(ptr, 0, size);
		return static_cast<T *>(ptr);
	}

	/// @brief Allocate and clear an array
	/// @tparam T Type of the object
	/// @param n Number of objects
	/// @param size Size of each object
	/// @param align Alignment
	/// @return T* Allocated storage 
	template<typename T = void>
	ATTRIB_MALLOC static inline T *allocza(size_t n, size_t size, size_t align = storage::safe_align<T>())
	{
		return storage::allocz<T>(n * size, align);
	}

	/// @brief Reallocate an object
	/// @tparam T Type of object
	/// @param ptr Pointer to the object
	/// @param size Size of the object
	/// @param align Alignment
	/// @return T* New allocated storage
	template<typename T = void>
	static inline T *realloc(T *ptr, size_t size, size_t align = storage::safe_align<T>())
	{
		if(size == 0) {
			real_storage::free(ptr);
			return nullptr;
		}
		if(ptr == nullptr) return storage::alloc<T>(size, align);
		return static_cast<T *>(real_storage::realloc(static_cast<void *>(ptr), size, align));
	}

	/// @brief Perform an array reallocation
	/// @tparam T Type of object
	/// @param ptr Pointer to the object
	/// @param n Number of objects
	/// @param size Size of each object
	/// @param align Alignment
	/// @return T* New allocated storage
	template<typename T = void>
	static inline T *realloca(T *ptr, size_t n, size_t size, size_t align = storage::safe_align<T>())
	{
		return storage::realloc<T>(ptr, n * size, align);
	}

	/// @brief Frees the memory of object ptr
	/// @tparam T Type of the object
	/// @param ptr Object to free
	template<typename T = void>
	static inline void free(T *ptr)
	{
		if(ptr == nullptr) return;
		real_storage::free(static_cast<void *>(ptr));
	}

	/// @brief Performs a failsafe reallocation, keeping the original pointer
	/// if the allocation fails for some reason
	/// @tparam T Type of object to reallocate
	/// @param ptr Referece to the pointer of current allocation
	/// @param size New size to give
	/// @param align Alignment
	/// @return T* nullptr if failed
	template<typename T = void>
	static inline T *realloc_failsafe(T *&ptr, size_t size, size_t align = storage::safe_align<T>())
	{
		T *_ptr = ptr;
		_ptr = storage::realloc<T>(ptr, size, align);
		if(_ptr == nullptr && size) return nullptr; // We must've had a positive size
		ptr = _ptr;
		return _ptr;
	}

	/// @brief Array version of realloc_failsafe
	/// @tparam T Type of object to reallocate
	/// @param ptr Referece to the pointer of current allocation
	/// @param n Number of elements
	/// @param size New size to give
	/// @param align Alignment
	/// @return T* nullptr if failed
	template<typename T = void>
	static inline T *realloca_failsafe(T *&ptr, size_t n, size_t size, size_t align = storage::safe_align<T>())
	{
		return storage::realloc_failsafe<T>(ptr, n * size, align);
	}

	/** @todo Implement allocators */
#if 0
	template<typename T>
	class allocator {
	public:
		static constexpr auto default_align = 16;

		static inline T *alloc(size_t size)
		{
			return allocator<T>::alloc_align(size, 8);
		}

		/**
		 * @brief Realloate the object
		 * 
		 * @param ptr 
		 * @param size 
		 * @return T* 
		 */
		static inline T *realloc(T *ptr, size_t size)
		{
			T *new_ptr = allocator<T>::alloc(size);
			if(new_ptr == nullptr) return nullptr;
			storage::copy(new_ptr, ptr, size);
			allocator<T>::free(ptr);
			return new_ptr;
		}

		/**
		 * @brief Required, allocates a piece of memory with alignment
		 * 
		 * @param size 
		 * @param align 
		 * @return void* 
		 */
		static inline T *alloc_align(size_t size, size_t align)
		{
			return nullptr;
		}

		/**
		 * @brief Free the given object pointer
		 * 
		 * @param ptr 
		 */
		static inline void free(T *ptr)
		{

		}
	};
#endif

	/// @todo Use allocator classes (typename A)
	template<typename T>
	class resource_unique {
	public:
		constexpr resource_unique()
		{
			// ...
		}

		inline resource_unique(T *new_ptr)
		{
			this->reset(new_ptr);
		}

		inline ~resource_unique()
		{
			this->reset(nullptr);
		}

		constexpr T *operator*()
		{
			return this->_ptr;
		}

		constexpr const T *operator*() const
		{
			return this->_ptr;
		}

		constexpr T *operator[](const size_t index)
		{
			return &this->_ptr[index];
		}

		constexpr const T *operator[](const size_t index) const
		{
			return &this->_ptr[index];
		}

		constexpr T *get() {
			return this->_ptr;
		}

		constexpr const T *get() const {
			return this->_ptr;
		}

		inline void reset(T *new_ptr) {
			if(this->_ptr != nullptr) // Deallocate the old object
				storage::free(this->_ptr);
			this->_ptr = new_ptr; // And replace with the new one
		}
	private:
		T *_ptr = nullptr;
	};

	template<class T>
	class global_wrapper {
		alignas(T) uint8_t data[sizeof(T)];
	public:
		constexpr global_wrapper() = default;
		
		template<class... Args>
		constexpr global_wrapper(Args&& ...args)
		{
			
		}

		constexpr T& operator=(T& rhs)
		{
			this->operator=(rhs);
		}

		constexpr T *operator->()
		{
			return reinterpret_cast<T *>(reinterpret_cast<void *>(data));
		}

		constexpr const T *operator->() const
		{
			return reinterpret_cast<const T *>(reinterpret_cast<const void *>(data));
		}
	};

	/**
	 * @brief A dynamic list that can be used for many generic purpouses
	 * 
	 * @tparam T The type of the list elements
	 */
	template<typename T>
	class dynamic_list {
	public:
		constexpr dynamic_list()
		{
			static_assert(sizeof(T) != 0);
		}

		dynamic_list(size_t size)
			: _size{ size }
		{
			static_assert(sizeof(T) != 0);
			this->_ptr = storage::realloca_failsafe(this->_ptr, this->_size * sizeof(T));
		}

		~dynamic_list()
		{
			if(this->_ptr != nullptr)
				storage::free(this->_ptr);
		}

		/**
		 * @brief Resizes the dynamic list to fit new elements
		 * 
		 * @param size Size to apply (capacity will expand accordingly)
		 * @return int 0 if success, nonzero otherwise
		 */
		inline int resize(const size_t size)
		{
			this->_size = size;
			// Increment capacity if needed or if nullptr
			if(this->_ptr == nullptr || this->_capacity < this->_size) {
				if(this->_capacity < this->_size) // Fit size inside capacity
					this->_capacity = this->_size;
				// Align to nearest chunk-per-alloc
				this->_capacity = this->_capacity + (this->chunks_per_alloc - (this->_capacity % this->chunks_per_alloc));
				if(storage::realloca_failsafe<T>(this->_ptr, this->_capacity, sizeof(T)) == nullptr)
					return error::ALLOCATION;
			}
			debug_printf("RESIZE %p,SIZE=%u,CAP=%u", this->_ptr, this->_size, this->_capacity);
			return 0; // No resizing done
		}

		/**
		 * @brief Compresses the list and sets capacity to size
		 * 
		 * @param size 
		 * @return int 
		 */
		inline int compress(const size_t size)
		{
			this->_size = size;
			this->_capacity = this->_size;
			if(storage::realloca_failsafe<T>(this->_ptr, this->_capacity, sizeof(T)) == nullptr)
				return error::ALLOCATION;
			return 0;
		}

		/**
		 * @brief Inserts an element onto the list
		 * 
		 * @param c The object to insert
		 * @return T* The inserted object inside the list
		 */
		inline T* insert(const T& c)
		{
			debug_printf("INSERT %p,SIZE=%u,CAP=%u", this->_ptr, this->_size, this->_capacity);
			if(this->resize(this->_size + 1) != 0)
				return nullptr;
			this->_ptr[this->_size - 1] = c;
			return &this->_ptr[this->_size - 1];
		}

		/**
		 * @brief Inserts an element onto the list but without a previous object
		 * 
		 * @return T* The inserted object inside the list
		 */
		inline T* insert()
		{
			if(this->resize(this->_size + 1) != 0)
				return nullptr;
			return &this->_ptr[this->_size - 1];
		}

		/**
		 * @brief Removes one or more elements from the list
		 * 
		 * @param idx Index to remove from
		 * @param n Number of elements to remove
		 */
		inline void remove(const size_t idx, const size_t n = 1)
		{
			debug_assert(idx < this->_size); // Index can't be greater than size
			storage::move(&this->_ptr[idx], &this->_ptr[idx + n], (this->_size - (idx + n)) * sizeof(T)); // Erase items
			this->_size--;
		}

		inline void remove(const T& ptr)
		{
			for(size_t i = 0; i < this->_size; i++) {
				if((uintptr_t)&ptr == (uintptr_t)&this->_ptr[i]) {
					this->remove(i, 1);
					return;
				}
			}
		}

		/**
		 * @brief Obtains the size of the list
		 * 
		 * @return size_t Size
		 */
		constexpr size_t size() const { return this->_size; }

		/**
		 * @brief Obtains the capacity of the list
		 * 
		 * @return size_t Capacity
		 */
		inline size_t capacity() const { return this->capacity; }

		/**
		 * @brief Indexes into the list
		 * 
		 * @param i Index
		 * @return T& Returned object reference
		 */
		inline T& operator[](const size_t i) const
		{
			debug_assert(i < this->_size); // Perform bounds checking
			return *&this->_ptr[i];
		}

		/**
		 * @brief A version of operator[] without the bounds checking
		 * 
		 * @param i Index
		 * @return T& Returned pointer
		 */
		inline T& unsafe_at(const size_t i) const
		{
			return *&this->_ptr[i];
		}

		/**
		 * @brief Whetever the current list holds no items
		 * 
		 * @return true List is empty
		 * @return false List is not empty
		 */
		inline bool empty() const
		{
			return this->size() == 0;
		}

		constexpr T **get_as_array() const
		{
			return &this->_ptr;
		}

		constexpr T **get_as_array()
		{
			return &this->_ptr;
		}

		static constexpr size_t chunks_per_alloc = 16;
	private:
		T *_ptr = nullptr;
		size_t _size = 0;
		size_t _capacity = 0;
	};

	template<class T, class M = base::mutex>
	class concurrent_dynamic_list : public dynamic_list<T> {
	public:
		M lock;
	};

	/**
	 * @brief Removes an element from a list, but only leaving a nullptr/invalid
	 * object behind instead of shrinking the whole list
	 * 
	 * @tparam T Type of the list
	 * @tparam P Predicate to fullfil
	 * @param list List itself
	 * @param pred Predicate
	 */
	template<typename T, typename P>
	inline void fast_remove_if(const T& list, P pred)
	{
		for(size_t i = 0; i < list.size(); i++) {
			if(pred(list[i])) {
				list[i] = nullptr;
				return;
			}
		}
	}

	template<typename T>
	inline void fast_remove(const T& list, size_t idx)
	{
		debug_assert(idx < list.size());
		list[idx] = nullptr;
	}
}

namespace storage {
	struct symbol {
		char name[28];
		void *address;
		size_t size;
	};
}

// C++ overloaded operators
inline void *operator new(size_t size, void *p) noexcept
{
	return storage::realloc(p, size);
}

inline void *operator new[](size_t size, void *p) noexcept
{
	return storage::realloc(p, size);
}

inline void *operator new(size_t size) noexcept
{
	return storage::alloc(size);
}

inline void operator delete(void *p) noexcept
{
	storage::free(p);
}

inline void operator delete(void *p, size_t) noexcept
{
	storage::free(p);
}

inline void operator delete[](void *p) noexcept
{
	storage::free(p);
}

inline void operator delete[](void *p, size_t) noexcept
{
	storage::free(p);
}

#endif
