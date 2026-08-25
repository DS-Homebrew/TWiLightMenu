#ifndef FILEQR_PROTOCOL_H
#define FILEQR_PROTOCOL_H

// Protocolo FQR1 — ver shared/PROTOCOL.md
// Remontador em STREAMING: grava cada chunk direto no arquivo (por offset),
// sem bufferizar o arquivo inteiro em RAM. Só um bitmap de frames fica na RAM.
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define FQR_MAGIC0 'F'
#define FQR_MAGIC1 'Q'
#define FQR_MAGIC2 'R'
#define FQR_MAGIC3 '1'

#define FQR_TYPE_MANIFEST 0x00
#define FQR_TYPE_DATA     0x01

#define FQR_HEADER_SIZE   9      // magic(4)+type(1)+index(2)+total(2)
#define FQR_NAME_MAX      255
#define FQR_DIR_MAX       255
#define FQR_PATH_MAX      512

// leitura little-endian
static inline uint16_t fqr_rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static inline uint32_t fqr_rd32(const uint8_t *p) {
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

typedef struct {
	int      have_manifest;
	uint32_t crc32;          // do manifest
	uint32_t size;           // bytes do arquivo
	uint16_t chunk;          // bytes por frame
	uint16_t total;          // nº de frames de dados
	uint16_t got;            // frames de dados já gravados
	char     name[FQR_NAME_MAX + 1];

	char     dir[FQR_DIR_MAX + 1];   // pasta de destino (definida na init)
	char     path[FQR_PATH_MAX + 1]; // caminho final do arquivo
	int      fd;             // descritor POSIX de saída (-1 = fechado)
	uint8_t *bitmap;         // bit-packed: 1 bit por frame de dados
	int      open_failed;    // abertura do arquivo falhou
} fqr_stream;

// dir = pasta onde o arquivo será gravado (ex.: "/", "/roms").
void fqr_stream_init(fqr_stream *s, const char *dir);

// Alimenta um pacote FQR1 cru (bytes do QR em Byte mode).
// Retorna 0 se aceitou/ignorou ok, <0 em erro.
int  fqr_stream_feed(fqr_stream *s, const uint8_t *pkt, size_t len);

// Recebeu manifest + todos os frames?
int  fqr_stream_complete(const fqr_stream *s);

// Fecha o arquivo, relê do SD e confere o CRC32. 1 = ok, 0 = falhou.
int  fqr_stream_verify(fqr_stream *s);

// Libera recursos (fecha arquivo se aberto, libera bitmap).
void fqr_stream_close(fqr_stream *s);

#endif
