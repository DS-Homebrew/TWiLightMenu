#include "crc32.h"

static uint32_t s_table[256];
static int s_ready = 0;

static void build_table(void)
{
	for (uint32_t n = 0; n < 256; n++) {
		uint32_t c = n;
		for (int k = 0; k < 8; k++)
			c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
		s_table[n] = c;
	}
	s_ready = 1;
}

uint32_t crc32_init(void)
{
	if (!s_ready)
		build_table();
	return 0xFFFFFFFFu;
}

uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len)
{
	for (size_t i = 0; i < len; i++)
		crc = s_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
	return crc;
}

uint32_t crc32_final(uint32_t crc)
{
	return crc ^ 0xFFFFFFFFu;
}

uint32_t crc32_buf(const uint8_t *data, size_t len)
{
	return crc32_final(crc32_update(crc32_init(), data, len));
}
