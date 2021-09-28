
#pragma once

#include <stdint.h>

#define RK_LEN 44 //round key length

// modified to work on reversed byte order input/output
// it could work by wrapping it between byte reversed I/O, minmize modification to actual AES code
// this is just my OCD to eliminate some copy
// original mbedTLS AES GET/PUT_UINT32 macros on little endian I/O regardless of CPU endianness
// seems like Nintendo used big endian hardware AES with little endian CPU
// by byte reversing on I/O, this mimics Nintendo behavior on little endian CPU
// calling it BE is not very accurate, it becomes little endian on big endian CPU

#define GET_UINT32_BE(n, b, i) \
	((uint8_t*)&(n))[0] = (b)[i + 3]; \
	((uint8_t*)&(n))[1] = (b)[i + 2]; \
	((uint8_t*)&(n))[2] = (b)[i + 1]; \
	((uint8_t*)&(n))[3] = (b)[i + 0]
#define PUT_UINT32_BE(n, b, i) \
	(b)[i + 0] = ((uint8_t*)&(n))[3]; \
	(b)[i + 1] = ((uint8_t*)&(n))[2]; \
	(b)[i + 2] = ((uint8_t*)&(n))[1]; \
	(b)[i + 3] = ((uint8_t*)&(n))[0]

void aes_gen_tables(void);

void aes_set_key_enc_128_be(uint32_t rk[RK_LEN], const unsigned char *key);

void aes_encrypt_128_be(const uint32_t rk[RK_LEN], const unsigned char input[16], unsigned char output[16]);

