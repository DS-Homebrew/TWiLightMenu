/* f_xy.c
 *
 * This file was imported from godmode9i, but it is liely not the
 * original source. twltool uses the same file.
 *
 * If you happen to know whom to credit I'd love to add the name
 *
 * Refactored to reduce the pointer casts and remove the dependency 
 * from tonccpy.
 */

#include <string.h>
#include <stdint.h>
#include "u128_math.h"


/************************ Constants / Defines *********************************/

const uint8_t DSi_KEY_MAGIC[16] = { 0x79, 0x3e, 0x4f, 0x1a, 0x5f, 0x0f, 0x68, 0x2a,
                                    0x58, 0x02, 0x59, 0x29, 0x4e, 0xfb, 0xfe, 0xff }; 
const uint8_t DSi_NAND_KEY_Y[16] = { 0x76, 0xdc, 0xb9, 0x0a, 0xd3, 0xc4, 0x4d, 0xbd,
                                     0x1d, 0xdd, 0x2d, 0x20, 0x05, 0x00, 0xa0, 0xe1 };
const uint8_t DSi_ES_KEY_Y[16] = { 0xe5, 0xcc, 0x5a, 0x8b, 0x56, 0xd0, 0xc9,0x72,
                                   0x9c, 0x17, 0xe8, 0xdc, 0x39, 0x12, 0x36, 0xa9 };
const uint8_t DSi_BOOT2_KEY[16] = { 0x98, 0xee, 0x80, 0x80, 0x00, 0x6c, 0xb4, 0xf6,
                                    0x3a, 0xc2, 0x6e, 0x62, 0xf9, 0xec, 0x34, 0xad };

/************************ Functions *******************************************/

void F_XY(uint8_t *key, const uint8_t *key_x, const uint8_t *key_y)
{
	uint8_t key_xy[16];

	for (int i=0; i<16; i++)
    key_xy[i] = key_x[i] ^ key_y[i];

  memcpy(key, DSi_KEY_MAGIC, sizeof(DSi_KEY_MAGIC));

	u128_add(key, key_xy);
	u128_lrot(key, 42);
}

//F_XY_reverse does the reverse of F(X^Y): takes (normal)key, and does F in reverse to generate the original X^Y key_xy.
void F_XY_reverse(const uint8_t *key, uint8_t *key_xy)
{
	memcpy(key_xy, key, 16);
	u128_rrot(key_xy, 42);
	u128_sub(key_xy, DSi_KEY_MAGIC);
}

