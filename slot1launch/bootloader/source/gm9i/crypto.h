#pragma once

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************ Constants / Defines *********************************/

#define AES_BLOCK_SIZE          16
#define SHA1_LEN                20

#define KEYSEED_DSI_NAND_0      0x24ee6906
#define KEYSEED_DSI_NAND_1      0xe65b601d

#define KEYSEED_3DS_NAND_0      0xb358a6af
#define KEYSEED_3DS_NAND_1      0x544e494e
#define KEYSEED_3DS_NAND_2      0x4f444e45
#define KEYSEED_3DS_NAND_3      0x08c267b7

#define KEYSEED_ES_0            0x4e00004a
#define KEYSEED_ES_1            0x4a00004e
#define KEYSEED_ES_2            0xc80c4b72

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

#ifdef __cplusplus
}
#endif
