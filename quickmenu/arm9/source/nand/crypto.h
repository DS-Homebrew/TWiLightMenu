#pragma once

#include <nds.h>

#define SHA1_LEN 20

#define AES_BLOCK_SIZE 16

typedef enum {
	ENCRYPT,
	DECRYPT
} crypt_mode_t;

typedef enum {
	NAND,
	NAND_3DS,
	ES
} key_mode_t;

// don't want to include nds.h just for this
void swiSHA1Calc(void *digest, const void *buf, size_t len);

int dsi_sha1_verify(const void *digest_verify, const void *data, unsigned len);

void dsi_crypt_init(const uint8_t *console_id_be, const uint8_t *emmc_cid, int is3DS);

void dsi_nand_crypt_1(uint8_t *out, const uint8_t* in, u32 offset);

void dsi_nand_crypt(uint8_t *out, const uint8_t* in, u32 offset, unsigned count);

int dsi_es_block_crypt(uint8_t *buf, unsigned buf_len, crypt_mode_t mode);

void dsi_boot2_crypt_set_ctr(uint32_t size_r);

void dsi_boot2_crypt(uint8_t* out, const uint8_t* in, unsigned count);
