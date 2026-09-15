#include <stdint.h>
#include "../mbedtls/aes.h"
#include "crypto.h"
//#include "ticket0.h"
#include "utils.h"

// more info:
//		https://github.com/Jimmy-Z/TWLbf/blob/master/dsi.c
//		https://github.com/Jimmy-Z/bfCL/blob/master/dsi.h
// ported back to 32 bit for ARM9

static const uint32_t DSi_NAND_KEY_Y[4] =
	{0x0ab9dc76u, 0xbd4dc4d3u, 0x202ddd1du, 0xe1a00005u};

static const uint32_t DSi_ES_KEY_Y[4] =
	{0x8b5acce5u, 0x72c9d056u, 0xdce8179cu, 0xa9361239u};

static const uint32_t DSi_BOOT2_KEY[4] =
	{0x8080ee98u, 0xf6b46c00u, 0x626ec23au, 0xad34ecf9u};

static const uint32_t DSi_KEY_MAGIC[4] =
	{0x1a4f3e79u, 0x2a680f5fu, 0x29590258u, 0xfffefb4eu};

static inline void xor_128(uint32_t *x, const uint32_t *a, const uint32_t *b){
	x[0] = a[0] ^ b[0];
	x[1] = a[1] ^ b[1];
	x[2] = a[2] ^ b[2];
	x[3] = a[3] ^ b[3];
}

static inline void add_128(uint32_t *a, const uint32_t *b){
	unsigned c1, c2, c3; // carry
	// round 1
	a[3] += b[3];
	a[2] += b[2];
	a[1] += b[1];
	a[0] += b[0];
	// carry
	c3 = a[2] < b[2];
	c2 = a[1] < b[1];
	c1 = a[0] < b[0];
	// round 2
	a[3] += c3;
	a[2] += c2;
	a[1] += c1;
	// carry
	c3 = a[2] < c2;
	c2 = a[1] < c1;
	// round 3
	a[3] += c3;
	a[2] += c2;
	// carry
	c3 = a[2] < c2;
	// round 4
	a[3] += c3;
}

static inline void add_128_32(uint32_t *a, uint32_t b){
	a[0] += b;
	if (a[0] < b){
		a[1] += 1;
		if (a[1] == 0) {
			a[2] += 1;
			if (a[2] == 0) {
				a[3] += 1;
			}
		}
	}
}

// Answer to life, universe and everything.
static inline void rol42_128(uint32_t *a){
	uint32_t t3 = a[3], t2 = a[2];
	a[3] = (a[2] << 10) | (a[1] >> 22);
	a[2] = (a[1] << 10) | (a[0] >> 22);
	a[1] = (a[0] << 10) | (t3 >> 22);
	a[0] = (t3 << 10) | (t2 >> 22);
}

static void dsi_aes_set_key(uint32_t *rk, const uint32_t *console_id, key_mode_t mode) {
	uint32_t key[4];
	switch (mode) {
	case NAND:
		key[0] = console_id[0];
		key[1] = console_id[0] ^ 0x24ee6906;
		key[2] = console_id[1] ^ 0xe65b601d;
		key[3] = console_id[1];
		break;
	case NAND_3DS:
		key[0] = console_id[0];
		key[1] = 0x544e494e;
		key[2] = 0x4f444e45;
		key[3] = console_id[1];
		break;
	case ES:
		key[0] = 0x4e00004a;
		key[1] = 0x4a00004e;
		key[2] = console_id[1] ^ 0xc80c4b72;
		key[3] = console_id[0];
		break;
	default:
		break;
	}
	// Key = ((Key_X XOR Key_Y) + FFFEFB4E295902582A680F5F1A4F3E79h) ROL 42
	// equivalent to F_XY in twltool/f_xy.c
	xor_128(key, key, mode == ES ? DSi_ES_KEY_Y : DSi_NAND_KEY_Y);
	// iprintf("AES KEY: XOR KEY_Y:\n");
	// print_bytes(key, 16);
	add_128(key, DSi_KEY_MAGIC);
	// iprintf("AES KEY: + MAGIC:\n");
	// print_bytes(key, 16);
	rol42_128(key);
	// iprintf("AES KEY: ROL 42:\n");
	// print_bytes(key, 16);
	aes_set_key_enc_128_be(rk, (uint8_t*)key);
}

int dsi_sha1_verify(const void *digest_verify, const void *data, unsigned len) {
	uint8_t digest[SHA1_LEN];
	swiSHA1Calc(digest, data, len);
	// return type of swiSHA1Verify() is declared void, so how exactly should we use it?
	int ret = memcmp(digest, digest_verify, SHA1_LEN);
	if (ret != 0) {
		//printf("  ");
		print_bytes(digest_verify, SHA1_LEN);
		//printf("\n  ");
		print_bytes(digest, SHA1_LEN);
		//printf("\n");
	}
	return ret;
}

static uint32_t nand_rk[RK_LEN];
static uint32_t nand_ctr_iv[4];
static uint32_t es_rk[RK_LEN];
static uint32_t boot2_rk[RK_LEN];

static int tables_generated = 0;

void dsi_crypt_init(const uint8_t *console_id_be, const uint8_t *emmc_cid, int is3DS) {
	if (tables_generated == 0) {
		aes_gen_tables();
		tables_generated = 1;
	}
	
	uint32_t console_id[2];
	GET_UINT32_BE(console_id[0], console_id_be, 4);
	GET_UINT32_BE(console_id[1], console_id_be, 0);

	dsi_aes_set_key(nand_rk, console_id, is3DS ? NAND_3DS : NAND);
	dsi_aes_set_key(es_rk, console_id, ES);

	aes_set_key_enc_128_be(boot2_rk, (uint8_t*)DSi_BOOT2_KEY);

	uint32_t digest[SHA1_LEN / sizeof(uint32_t)];
	swiSHA1Calc(digest, emmc_cid, 16);
	nand_ctr_iv[0] = digest[0];
	nand_ctr_iv[1] = digest[1];
	nand_ctr_iv[2] = digest[2];
	nand_ctr_iv[3] = digest[3];
}

static inline void aes_ctr(const uint32_t *rk, const uint32_t *ctr, uint32_t *in, uint32_t *out) {
	uint32_t xor[4];
	aes_encrypt_128_be(rk, (uint8_t*)ctr, (uint8_t*)xor);
	xor_128(out, in, xor);
}

// crypt one block, in/out must be aligned to 32 bit(restriction induced by xor_128)
// offset as block offset, block as AES block
void dsi_nand_crypt_1(uint8_t* out, const uint8_t* in, uint32_t offset) {
	uint32_t ctr[4] = { nand_ctr_iv[0], nand_ctr_iv[1], nand_ctr_iv[2], nand_ctr_iv[3] };
	add_128_32(ctr, offset);
	// iprintf("AES CTR:\n");
	// print_bytes(buf, 16);
	aes_ctr(nand_rk, ctr, (uint32_t*)in, (uint32_t*)out);
}

void dsi_nand_crypt(uint8_t* out, const uint8_t* in, uint32_t offset, unsigned count) {
	uint32_t ctr[4] = { nand_ctr_iv[0], nand_ctr_iv[1], nand_ctr_iv[2], nand_ctr_iv[3] };
	add_128_32(ctr, offset);
	for (unsigned i = 0; i < count; ++i) {
		aes_ctr(nand_rk, ctr, (uint32_t*)in, (uint32_t*)out);
		out += AES_BLOCK_SIZE;
		in += AES_BLOCK_SIZE;
		add_128_32(ctr, 1);
	}
}
	
static uint32_t boot2_ctr[4];

void dsi_boot2_crypt_set_ctr(uint32_t size_r) {
	boot2_ctr[0] = size_r;
	boot2_ctr[1] = -size_r;
	boot2_ctr[2] = ~size_r;
	boot2_ctr[3] = 0;
}

void dsi_boot2_crypt(uint8_t* out, const uint8_t* in, unsigned count) {
	for (unsigned i = 0; i < count; ++i) {
		aes_ctr(boot2_rk, boot2_ctr, (uint32_t*)in, (uint32_t*)out);
		out += AES_BLOCK_SIZE;
		in += AES_BLOCK_SIZE;
		add_128_32(boot2_ctr, 1);
	}
}

// http://problemkaputt.de/gbatek.htm#dsiesblockencryption
// works in place, also must be aligned to 32 bit
// why is it called ES?
/*int dsi_es_block_crypt(uint8_t *buf, unsigned buf_len, crypt_mode_t mode) {
	es_block_footer_t *footer;
	footer = (es_block_footer_t*)(buf + buf_len - sizeof(es_block_footer_t));
	// backup mac since it might be overwritten by padding
	// and also nonce, it becomes garbage after decryption
	uint8_t ccm_mac[AES_CCM_MAC_LEN];
	uint8_t nonce[AES_CCM_NONCE_LEN];
	memcpy(ccm_mac, footer->ccm_mac, AES_CCM_MAC_LEN);
	memcpy(nonce, footer->nonce, AES_CCM_NONCE_LEN);

	uint32_t ctr32[4], pad32[4], mac32[4];
// I'm too paranoid to use more stack variables
#define ctr ((uint8_t*)ctr32)
#define pad ((uint8_t*)pad32)
#define mac ((uint8_t*)mac32)
#define zero(a) static_assert(sizeof(a[0]) == 4, "invalid operand"); \
	a[0] = 0; a[1] = 0; a[2] = 0; a[3] = 0
	if (mode == DECRYPT) {
		// decrypt footer
		zero(ctr32);
		memcpy(ctr + 1, nonce, AES_CCM_NONCE_LEN);
		// footer might not be 32 bit aligned after all, so we copy it out to decrypt
		memcpy(pad, footer->encrypted, AES_BLOCK_SIZE);
		aes_ctr(es_rk, ctr32, pad32, pad32);
		memcpy(footer->encrypted, pad, AES_BLOCK_SIZE);
	}
	// check decrypted footer
	if (footer->fixed_3a != 0x3a) {
		i//printff("ES block footer offset 0x10 should be 0x3a, got 0x%02x\n", footer->fixed_3a);
		return 1;
	}
	uint32_t block_size;
	GET_UINT32_BE(block_size, footer->len32be, 0);
	block_size &= 0xffffff;
	if (block_size + sizeof(es_block_footer_t) != buf_len) {
		i//printff("block size in footer doesn't match, %06x != %06x\n",
			(unsigned)block_size, (unsigned)(buf_len - sizeof(es_block_footer_t)));
		return 1;
	}
	// padding to multiple of 16
	uint32_t remainder = block_size & 0xf;
	if (remainder != 0) {
		zero(pad32);
		if (mode == DECRYPT) {
			ctr32[0] = (block_size >> 4) + 1;
			memcpy(ctr + 3, nonce, AES_CCM_NONCE_LEN);
			ctr[0xf] = 2;
			aes_ctr(es_rk, ctr32, pad32, pad32);
		}
		memcpy(buf + block_size, pad + remainder, 16 - remainder);
		block_size += 16 - remainder;
	}
	// AES-CCM MAC
	mac32[0] = block_size;
	memcpy(mac + 3, nonce, AES_CCM_NONCE_LEN);
	mac[0xf] = 0x3a;
	aes_encrypt_128_be(es_rk, mac, mac);
	// AES-CCM CTR
	ctr32[0] = 0;
	memcpy(ctr + 3, nonce, AES_CCM_NONCE_LEN);
	ctr[0xf] = 2;
	// AES-CCM start
	zero(pad32);
	aes_ctr(es_rk, ctr32, pad32, pad32);
	add_128_32(ctr32, 1);
	// AES-CCM loop
	if (mode == DECRYPT) {
		for (unsigned i = 0; i < block_size; i += 16) {
			aes_ctr(es_rk, ctr32, (uint32_t*)(buf + i), (uint32_t*)(buf + i));
			add_128_32(ctr32, 1);
			xor_128(mac32, mac32, (uint32_t*)(buf + i));
			aes_encrypt_128_be(es_rk, mac, mac);
		}
	} else {
		for (unsigned i = 0; i < block_size; i += 16) {
			xor_128(mac32, mac32, (uint32_t*)(buf + i));
			aes_encrypt_128_be(es_rk, mac, mac);
			aes_ctr(es_rk, ctr32, (uint32_t*)(buf + i), (uint32_t*)(buf + i));
			add_128_32(ctr32, 1);
		}
	}
	// AES-CCM MAC final
	xor_128(mac32, mac32, pad32);
	if (mode == DECRYPT) {
		if (memcmp(mac, ccm_mac, 16) == 0) {
			if (remainder != 0) {
				// restore mac
				memcpy(footer->ccm_mac, ccm_mac, AES_CCM_MAC_LEN);
			}
			// restore nonce
			memcpy(footer->nonce, nonce, AES_CCM_NONCE_LEN);
			return 0;
		} else {
			//printf("MAC verification failed\n");
			return 1;
		}
	} else {
		memcpy(footer->ccm_mac, mac, AES_CCM_MAC_LEN);
		// AES-CTR crypt later half of footer
		zero(ctr32);
		memcpy(ctr + 1, nonce, AES_CCM_NONCE_LEN);
		memcpy(pad, footer->encrypted, AES_BLOCK_SIZE);
		aes_ctr(es_rk, ctr32, pad32, pad32);
		memcpy(footer->encrypted, pad, AES_BLOCK_SIZE);
		// restore nonce
		memcpy(footer->nonce, nonce, AES_CCM_NONCE_LEN);
		return 0;
	}
#undef ctr
#undef pad
#undef mac
#undef zero
}*/
