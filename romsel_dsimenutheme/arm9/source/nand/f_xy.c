#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include "common/tonccpy.h"

//#define DEBUG


void n128_lrot(uint64_t *num, uint32_t shift)
{
	uint64_t tmp[2];

	tmp[0] = num[0]<<shift;
	tmp[1] = num[1]<<shift;
	tmp[0] |= (num[1]>>(64-shift));
	tmp[1] |= (num[0]>>(64-shift));

	num[0] = tmp[0];
	num[1] = tmp[1];
}
void n128_rrot(uint64_t *num, uint32_t shift)
{
	uint64_t tmp[2];

	tmp[0] = num[0]>>shift;
	tmp[1] = num[1]>>shift;
	tmp[0] |= (num[1]<<(64-shift));
	tmp[1] |= (num[0]<<(64-shift));

	num[0] = tmp[0];
	num[1] = tmp[1];
}

void n128_add(uint64_t *a, uint64_t *b)
{
	uint64_t *a64 = a;
	uint64_t *b64 = b;
	uint64_t tmp = (a64[0]>>1)+(b64[0]>>1) + (a64[0] & b64[0] & 1);
	 
	tmp = tmp >> 63;
        a64[0] = a64[0] + b64[0];
        a64[1] = a64[1] + b64[1] + tmp;
}

void n128_sub(uint64_t *a, uint64_t *b)
{
	uint64_t *a64 = a;
	uint64_t *b64 = b;
	uint64_t tmp = (a64[0]>>1)-(b64[0]>>1) - ((a64[0]>>63) & (b64[0]>>63) & 1);
        
	tmp = tmp >> 63;
        a64[0] = a64[0] - b64[0];
        a64[1] = a64[1] - b64[1] - tmp;
}

void F_XY(uint32_t *key, uint32_t *key_x, uint32_t *key_y)
{
	int i;
	unsigned char key_xy[16];

	toncset(key_xy, 0, 16);
	toncset(key, 0, 16);
	for (i=0; i<16; i++)key_xy[i] = ((unsigned char*)key_x)[i] ^ ((unsigned char*)key_y)[i];

	key[0] = 0x1a4f3e79;
	key[1] = 0x2a680f5f;
	key[2] = 0x29590258;
	key[3] = 0xfffefb4e;

	n128_add((uint64_t*)key, (uint64_t*)key_xy);
	n128_lrot((uint64_t*)key, 42);
}

//F_XY_reverse does the reverse of F(X^Y): takes (normal)key, and does F in reverse to generate the original X^Y key_xy.
void F_XY_reverse(uint32_t *key, uint32_t *key_xy)
{
	uint32_t tmpkey[4];
	toncset(key_xy, 0, 16);
	toncset(tmpkey, 0, 16);
	tonccpy(tmpkey, key, 16);

	key_xy[0] = 0x1a4f3e79;
	key_xy[1] = 0x2a680f5f;
	key_xy[2] = 0x29590258;
	key_xy[3] = 0xfffefb4e;

	n128_rrot((uint64_t*)tmpkey, 42);
	n128_sub((uint64_t*)tmpkey, (uint64_t*)key_xy);
	tonccpy(key_xy, tmpkey, 16);
}

