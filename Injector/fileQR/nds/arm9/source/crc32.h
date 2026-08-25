#ifndef FILEQR_CRC32_H
#define FILEQR_CRC32_H

#include <stddef.h>
#include <stdint.h>

// CRC32 zlib/PNG (poly 0xEDB88320). Igual ao crc32() do sender (app.js).

// De uma vez só:
uint32_t crc32_buf(const uint8_t *data, size_t len);

// Incremental (p/ ler o arquivo gravado no SD em blocos):
//   uint32_t c = crc32_init();
//   c = crc32_update(c, buf, n);  // repetir
//   uint32_t final = crc32_final(c);
uint32_t crc32_init(void);
uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len);
uint32_t crc32_final(uint32_t crc);

#endif
