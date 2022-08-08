#ifndef BYTESWAP_HXX
#define BYTESWAP_HXX

#include <types.hxx>

// msvc and clang
#if defined(__BYTE_ORDER__)
#	if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#		define BIG_ENDIAN
#	elif (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#		define LITTLE_ENDIAN
#	endif
#endif

// gcc
#if defined(__BIG_ENDIAN__)
#	define BIG_ENDIAN
#elif defined(__LITTLE_ENDIAN__)
#	define LITTLE_ENDIAN
#endif

constexpr uint16_t bswap16(const uint16_t x) {
	return
		((x << 8) & 0xff00ULL) |
		((x >> 8) & 0x00ffULL);
}

constexpr uint32_t bswap32(const uint32_t x) {
	return
		((x << 24) & 0xff000000ULL) |
		((x << 8) & 0x00ff0000ULL) |
		((x >> 8) & 0x0000ff00ULL) |
		((x >> 24) & 0x000000ffULL);
}

constexpr uint64_t bswap64(const uint64_t x) {
	return
		((x << 56) & 0xff00000000000000ULL) |
		((x << 40) & 0x00ff000000000000ULL) |
		((x << 24) & 0x0000ff0000000000ULL) |
		((x << 8) & 0x000000ff00000000ULL) |
		((x >> 8) & 0x00000000ff000000ULL) |
		((x >> 24) & 0x0000000000ff0000ULL) |
		((x >> 40) & 0x000000000000ff00ULL) |
		((x >> 56) & 0x00000000000000ffULL);
}

#if defined(LITTLE_ENDIAN)
#	define cpu_to_be16(x) bswap16(x)
#	define cpu_to_be32(x) bswap32(x)
#	define cpu_to_be64(x) bswap64(x)
#	define cpu_to_le16(x) (x)
#	define cpu_to_le32(x) (x)
#	define cpu_to_le64(x) (x)
#	define be_to_cpu16(x) bswap16(x)
#	define be_to_cpu32(x) bswap32(x)
#	define be_to_cpu64(x) bswap64(x)
#	define le_to_cpu16(x) (x)
#	define le_to_cpu32(x) (x)
#	define le_to_cpu64(x) (x)
#else
#	define cpu_to_be16(x) (x)
#	define cpu_to_be32(x) (x)
#	define cpu_to_be64(x) (x)
#	define cpu_to_le16(x) bswap16(x)
#	define cpu_to_le32(x) bswap32(x)
#	define cpu_to_le64(x) bswap64(x)
#	define be_to_cpu16(x) (x)
#	define be_to_cpu32(x) (x)
#	define be_to_cpu64(x) (x)
#	define le_to_cpu16(x) bswap16(x)
#	define le_to_cpu32(x) bswap32(x)
#	define le_to_cpu64(x) bswap64(x)
#endif

template<typename T>
constexpr T cpu_to_be(const T value)
{
	if constexpr(sizeof(T) == 2) {
		return cpu_to_be16(value);
	} else if constexpr(sizeof(T) == 4) {
		return cpu_to_be32(value);
	} else if constexpr(sizeof(T) == 8) {
		return cpu_to_be64(value);
	}
	return value;
}

template<typename T>
constexpr T cpu_to_le(const T value)
{
	if constexpr(sizeof(T) == 2) {
		return cpu_to_le16(value);
	} else if constexpr(sizeof(T) == 4) {
		return cpu_to_le32(value);
	} else if constexpr(sizeof(T) == 8) {
		return cpu_to_le64(value);
	}
	return value;
}

template<typename T>
constexpr T be_to_cpu(const T value)
{
	if constexpr(sizeof(T) == 2) {
		return be_to_cpu16(value);
	} else if constexpr(sizeof(T) == 4) {
		return be_to_cpu32(value);
	} else if constexpr(sizeof(T) == 8) {
		return be_to_cpu64(value);
	}
	return value;
}

template<typename T>
constexpr T le_to_cpu(const T value)
{
	if constexpr(sizeof(T) == 2) {
		return le_to_cpu16(value);
	} else if constexpr(sizeof(T) == 4) {
		return le_to_cpu32(value);
	} else if constexpr(sizeof(T) == 8) {
		return le_to_cpu64(value);
	}
	return value;
}

#endif
