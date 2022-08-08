#ifndef CRYPTO_HXX
#define CRYPTO_HXX 1

#include <types.hxx>

namespace crypto {
	namespace arc4 {
		int encrypt(uint8_t *ctext, const void *bitstream, size_t len, const void *key, size_t keylen);
	}

	namespace blowfish {
		void genkey(uint32_t parray[32], uint32_t sbox[4][256], const void *key, size_t keylen);
		void encrypt(void *out, const uint32_t parray[32], const uint32_t sbox[4][256], const void *data, size_t len);
		void decrypt(void *out, const uint32_t parray[32], const uint32_t sbox[4][256], const void *data, size_t len);
	}
	
	namespace rsa {
		void genkey(uint32_t *_e, uint32_t *_d, uint32_t *_n);
		void encrypt(void *_out, const void *_data, size_t size, uint32_t e, uint32_t n);
		void decrypt(void *_out, const void *_data, size_t size, uint32_t d, uint32_t n);
	}

	namespace xtea {
		void encrypt(void *_out, const void *_data, size_t len, const uint32_t key[4], size_t n_rounds);
		void decrypt(void *_out, const void *_data, size_t len, const uint32_t key[4], size_t n_rounds);
	}
}

#endif
