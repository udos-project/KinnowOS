#ifndef LOCALE_HXX
#define LOCALE_HXX

#include <types.hxx>

namespace locale {
	extern const unsigned char asc2ebc[];
	extern const unsigned char ebc2asc[];

	template<typename T>
	constexpr bool islower(T x)
	{
		return
			((x) == 'a' || (x) == 'b' || (x) == 'c' || (x) == 'd' || (x) == 'e'
			|| (x) == 'f' || (x) == 'g' || (x) == 'h' || (x) == 'i' || (x) == 'j'
			|| (x) == 'k' || (x) == 'l' || (x) == 'm' || (x) == 'n' || (x) == 'o'
			|| (x) == 'p' || (x) == 'q' || (x) == 'r' || (x) == 's' || (x) == 't'
			|| (x) == 'u' || (x) == 'v' || (x) == 'w' || (x) == 'x' || (x) == 'y'
			|| (x) == 'z');
	}

	template<typename T>
	constexpr bool isupper(T x)
	{
		return
			((x) == 'A' || (x) == 'B' || (x) == 'C' || (x) == 'D' || (x) == 'E'
			|| (x) == 'F' || (x) == 'G' || (x) == 'H' || (x) == 'I' || (x) == 'J'
			|| (x) == 'K' || (x) == 'L' || (x) == 'M' || (x) == 'N' || (x) == 'O'
			|| (x) == 'P' || (x) == 'Q' || (x) == 'R' || (x) == 'S' || (x) == 'T'
			|| (x) == 'U' || (x) == 'V' || (x) == 'W' || (x) == 'X' || (x) == 'Y'
			|| (x) == 'Z');
	}

	template<typename T>
	constexpr bool isdigit(T x)
	{
		return
			((x) == '0' || (x) == '1' || (x) == '2' || (x) == '3' || (x) == '4'
			|| (x) == '5' || (x) == '6' || (x) == '7' || (x) == '8' || (x) == '9');
	}

	template<typename T>
	constexpr bool isalpha(T x)
	{
		return islower<T>(x) || isupper<T>(x);
	}

	template<typename T>
	constexpr bool isalnum(T x)
	{
		return isalpha<T>(x) || isdigit<T>(x);
	}

	template<typename T>
	T toupper(T x)
	{
		return
			(x) == 'a' ? 'A' : (x) == 'g' ? 'G' : (x) == 'm' ? 'M' : (x) == 's' ? 'S' :
			(x) == 'b' ? 'B' : (x) == 'h' ? 'H' : (x) == 'n' ? 'N' : (x) == 't' ? 'T' :
			(x) == 'c' ? 'C' : (x) == 'i' ? 'I' : (x) == 'o' ? 'O' : (x) == 'u' ? 'U' :
			(x) == 'd' ? 'D' : (x) == 'j' ? 'J' : (x) == 'p' ? 'P' : (x) == 'v' ? 'V' :
			(x) == 'e' ? 'E' : (x) == 'k' ? 'K' : (x) == 'q' ? 'Q' : (x) == 'w' ? 'W' :
			(x) == 'f' ? 'F' : (x) == 'l' ? 'L' : (x) == 'r' ? 'R' : (x) == 'x' ? 'X' :
			(x) == 'y' ? 'Y' : (x) == 'z' ? 'Z' : (x);
	}

	template<typename T>
	T tolower(T x)
	{
		return
			(x) == 'A' ? 'a' : (x) == 'G' ? 'g' : (x) == 'M' ? 'm' : (x) == 'S' ? 's' :
			(x) == 'B' ? 'b' : (x) == 'H' ? 'h' : (x) == 'N' ? 'n' : (x) == 'T' ? 't' :
			(x) == 'C' ? 'c' : (x) == 'I' ? 'i' : (x) == 'O' ? 'o' : (x) == 'U' ? 'u' :
			(x) == 'D' ? 'd' : (x) == 'J' ? 'j' : (x) == 'P' ? 'p' : (x) == 'V' ? 'v' :
			(x) == 'E' ? 'e' : (x) == 'K' ? 'k' : (x) == 'Q' ? 'q' : (x) == 'W' ? 'w' :
			(x) == 'F' ? 'f' : (x) == 'L' ? 'l' : (x) == 'R' ? 'r' : (x) == 'X' ? 'x' :
			(x) == 'Y' ? 'y' : (x) == 'Z' ? 'z' : (x);
	}

	template<typename T>
	constexpr bool isspace(const T x)
	{
		return x == ' ' || x == '\t' || x == '\v' || x == '\r' || x == '\n' || x == '\f';
	}

	template<typename T>
	constexpr bool ispunct(const T x)
	{
		return x == '.' || x == ',' || x == ';' || x == ':';
	}

	template<typename T>
	constexpr bool isxdigit(T x)
	{
		x = toupper<T>(x);
		return isdigit<T>(x) || x == 'A' || x == 'B' || x == 'C' || x == 'D' || x == 'E' || x == 'F';
	}

	enum charset {
		NATIVE,
		EBCDIC_1047,
		ASCII,
		UTF8,
	};

	constexpr enum charset get_native_cset()
	{
		if constexpr('A' == 65)
			return locale::charset::ASCII;
		return locale::charset::EBCDIC_1047;
	}

	template<typename T = char, enum charset SrcCset, enum charset DstCset>
	constexpr T convert(T ch)
	{
		constexpr enum charset src_cset = ((SrcCset == locale::charset::NATIVE) ? locale::get_native_cset() : SrcCset);
		constexpr enum charset dst_cset = ((DstCset == locale::charset::NATIVE) ? locale::get_native_cset() : DstCset);
		if constexpr(src_cset == dst_cset) // Same charset returns same character
			return ch;
		
		if constexpr(src_cset == locale::charset::ASCII) {
			if constexpr(dst_cset == locale::charset::EBCDIC_1047)
				return asc2ebc[static_cast<int>(ch & 0xFF)];
		} else if constexpr(src_cset == locale::charset::EBCDIC_1047) {
			if constexpr(dst_cset == locale::charset::ASCII)
				return ebc2asc[static_cast<int>(ch & 0xFF)];
		}
	}

	template<typename T = char, enum charset SrcCset, enum charset DstCset>
	constexpr void convert(T *str)
	{
		while(*str != '\0')
			*(str++) = locale::convert<T, SrcCset, DstCset>(*str);
	}
}

#endif
