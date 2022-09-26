#include "dsi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "u128_math.h"

void dsi_set_key( dsi_context* ctx,
				 const unsigned char key[16] )
{
	unsigned char keyswap[16];
	u128_swap(keyswap, key) ;

	aes_setkey_enc(&ctx->aes, keyswap, 128);
}

void dsi_add_ctr( dsi_context* ctx,
                  unsigned int carry)
{
	unsigned int counter[4];
	unsigned char *outctr = (unsigned char*)ctx->ctr;
	int sum;
	signed int i;

	for (i = 0; i < 4; i++)
		counter[i] = (outctr[i * 4 + 0] << 24) | (outctr[i * 4 + 1] << 16) |
		             (outctr[i * 4 + 2] <<  8) | (outctr[i * 4 + 3] << 0);

	for (i = 3; i >= 0; i--) {
		sum = counter[i] + carry;
		carry = (uint)(sum < counter[i]);

		counter[i] = sum;
	}

	for (i = 0; i < 4; i++) {
		outctr[i * 4 + 0] = counter[i] >> 24;
		outctr[i * 4 + 1] = counter[i] >> 16;
		outctr[i * 4 + 2] = counter[i] >> 8;
		outctr[i * 4 + 3] = counter[i] >> 0;
	}
}

void dsi_set_ctr( dsi_context* ctx,
				  const unsigned char ctr[16] )
{
	for (int i = 0; i < 16; i++)
		ctx->ctr[i] = ctr[15-i];
}

void dsi_init_ctr( dsi_context* ctx,
				   const unsigned char key[16],
				   const unsigned char ctr[12] )
{
	dsi_set_key(ctx, key);
	dsi_set_ctr(ctx, ctr);
}

void dsi_crypt_ctr( dsi_context* ctx,
                    const void* in,
                    void* out,
                    unsigned int len)
{
	unsigned int i;
	for (i = 0; i < len; i += 0x10) {
		dsi_crypt_ctr_block(ctx, in+i, out+i);
	}
}

void dsi_crypt_ctr_block( dsi_context* ctx, 
						  const unsigned char input[16], 
						  unsigned char output[16] )
{
	int i;
	unsigned char stream[16];


	aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->ctr, stream);


	if (input) {
		for (i=0; i<16; i++) {
			output[i] = stream[15-i] ^ input[i];
		}
	} else {
		for (i=0; i<16; i++)
			output[i] = stream[15-i];
	}

	dsi_add_ctr(ctx, 1);
}


void dsi_init_ccm( dsi_context* ctx,
				   unsigned char key[16],
				   unsigned int maclength,
				   unsigned int payloadlength,
				   unsigned int assoclength,
				   unsigned char nonce[12] )
{
	int i;

	dsi_set_key(ctx, key);

	ctx->maclen = maclength;

	maclength = (maclength-2)/2;

	payloadlength = (payloadlength+15) & ~15;

	// CCM B0 block:
	// [1-byte flags] [12-byte nonce] [3-byte size]
	ctx->mac[0] = (maclength<<3) | 2;
	if (assoclength)
		ctx->mac[0] |= (1<<6);
	for (i=0; i<12; i++)
		ctx->mac[1+i] = nonce[11-i];
	ctx->mac[13] = payloadlength>>16;
	ctx->mac[14] = payloadlength>>8;
	ctx->mac[15] = payloadlength>>0;

	aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->mac, ctx->mac);

	// CCM CTR:
	// [1-byte flags] [12-byte nonce] [3-byte ctr]
	ctx->ctr[0] = 2;
	for (i=0; i<12; i++)
		ctx->ctr[1+i] = nonce[11-i];
	ctx->ctr[13] = 0;
	ctx->ctr[14] = 0;
	ctx->ctr[15] = 0;

	dsi_crypt_ctr_block(ctx, 0, ctx->S0);
}

void dsi_encrypt_ccm_block( dsi_context* ctx, 
							unsigned char input[16], 
							unsigned char output[16],
							unsigned char* mac )
{
	int i;

	for (i=0; i<16; i++)
		ctx->mac[i] ^= input[15-i];

	aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->mac, ctx->mac);

	if (mac) {
		for (i=0; i<16; i++)
			mac[i] = ctx->mac[15-i] ^ ctx->S0[i];
	}

	if (output)
		dsi_crypt_ctr_block(ctx, input, output);
}


void dsi_decrypt_ccm_block( dsi_context* ctx, 
							unsigned char input[16], 
							unsigned char output[16],
							unsigned char* mac )
{
	int i;


	if (output) {
		dsi_crypt_ctr_block(ctx, input, output);


		for (i=0; i<16; i++)
			ctx->mac[i] ^= output[15-i];
	} else {
		for (i=0; i<16; i++)
			ctx->mac[i] ^= input[15-i];
	}

	aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->mac, ctx->mac);


	if (mac) {
		for (i=0; i<16; i++)
			mac[i] = ctx->mac[15-i] ^ ctx->S0[i];
	}
}


void dsi_decrypt_ccm( dsi_context* ctx, 
					  unsigned char* input, 
					  unsigned char* output,
					  unsigned int size,
					  unsigned char* mac )
{
	unsigned char block[16];
	unsigned char ctr[16];

	while (size > 16) {
		dsi_decrypt_ccm_block(ctx, input, output, mac);

		if (input)
			input += 16;
		if (output)
			output += 16;

		size -= 16;
	}

	memcpy(ctr, ctx->ctr, 16);
	memset(block, 0, 16);	
	dsi_crypt_ctr_block(ctx, block, block);
	memcpy(ctx->ctr, ctr, 16);
	memcpy(block, input, size);


	dsi_decrypt_ccm_block(ctx, block, block, mac);
	memcpy(output, block, size);
}


void dsi_encrypt_ccm( dsi_context* ctx, 
					  unsigned char* input, 
					  unsigned char* output,
					  unsigned int size,
					  unsigned char* mac )
{
	unsigned char block[16];

	while (size > 16) {
		dsi_encrypt_ccm_block(ctx, input, output, mac);

		if (input)
			input += 16;
		if (output)
			output += 16;

		size -= 16;
	}

	memset(block, 0, 16);
	memcpy(block, input, size);
	dsi_encrypt_ccm_block(ctx, block, block, mac);
	memcpy(output, block, size);
}

void dsi_es_init( dsi_es_context* ctx,
				  unsigned char key[16] )
{
	memcpy(ctx->key, key, 16);
	ctx->randomnonce = 1;
}

void dsi_es_set_nonce( dsi_es_context* ctx,
					   unsigned char nonce[12] )
{
	memcpy(ctx->nonce, nonce, 12);
	ctx->randomnonce = 0;
}

void dsi_es_set_random_nonce( dsi_es_context* ctx )
{
	ctx->randomnonce = 1;
}

int dsi_es_decrypt( dsi_es_context* ctx,
				    unsigned char* buffer,
				    unsigned char metablock[32],
					unsigned int size )
{
	unsigned char ctr[16];
	unsigned char nonce[12];
	unsigned char scratchpad[16];
	unsigned char chkmac[16];
	unsigned char genmac[16];
	dsi_context cryptoctx;
	unsigned int chksize;


	memcpy(chkmac, metablock, 16);

	memcpy(ctr, metablock + 16, 16);
	ctr[0] = 0;
	ctr[13] = 0;
	ctr[14] = 0;
	ctr[15] = 0;

	dsi_init_ctr(&cryptoctx, ctx->key, ctr);
	dsi_crypt_ctr_block(&cryptoctx, metablock+16, scratchpad);

	chksize = (scratchpad[13]<<16) | (scratchpad[14]<<8) | (scratchpad[15]<<0);

	if (scratchpad[0] != 0x3A) {
		return -1;
	}

	if (chksize != size) {
		return -2;
	}

	memcpy(nonce, metablock + 17, 12);

	dsi_init_ccm(&cryptoctx, ctx->key, 16, size, 0, nonce);
	dsi_decrypt_ccm(&cryptoctx, buffer, buffer, size, genmac);

	if (memcmp(genmac, chkmac, 16) != 0) {
		return -3;
	}

	return 0;
}


void dsi_es_encrypt( dsi_es_context* ctx,
				     unsigned char* buffer,
				     unsigned char metablock[32],
				 	 unsigned int size )
{
	int i;
	unsigned char nonce[12];
	unsigned char mac[16];
	unsigned char ctr[16];
	unsigned char scratchpad[16];
	dsi_context cryptoctx;

	if (ctx->randomnonce) {
		srand( (unsigned int)time(0) );

		for (i=0; i<12; i++)
			nonce[i] = rand();
	} else {
		memcpy(nonce, ctx->nonce, 12);
	}

	dsi_init_ccm(&cryptoctx, ctx->key, 16, size, 0, nonce);
	dsi_encrypt_ccm(&cryptoctx, buffer, buffer, size, mac);

	memset(scratchpad, 0, 16);
	scratchpad[0] = 0x3A;
	scratchpad[13] = size >> 16;
	scratchpad[14] = size >> 8;
	scratchpad[15] = size >> 0;

	memset(ctr, 0, 16);
	memcpy(ctr+1, nonce, 12);

	dsi_init_ctr(&cryptoctx, ctx->key, ctr);
	dsi_crypt_ctr_block(&cryptoctx, scratchpad, metablock+16);
	memcpy(metablock+17, nonce, 12);

	memcpy(metablock, mac, 16);

}
