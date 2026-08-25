#include "protocol.h"
#include "crc32.h"
#include <stdio.h>   // snprintf
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>   // open, O_*
#include <unistd.h>  // read, write, lseek, close

// I/O via descritores POSIX (open/write/lseek) — NÃO usa o lock recursivo
// por-FILE do newlib, que quebra no calico/libnds (crash em threadRemoveWaiter).

// -------- bitmap bit-packed --------
static int bit_get(const uint8_t *bm, uint32_t i) { return (bm[i >> 3] >> (i & 7)) & 1; }
static void bit_set(uint8_t *bm, uint32_t i) { bm[i >> 3] |= (uint8_t)(1u << (i & 7)); }

// -------- caminho --------
static void sanitize_name(char *name)
{
	for (char *p = name; *p; p++)
		if (*p == '/' || *p == '\\' || *p == ':')
			*p = '_';
	if (name[0] == '\0')
		strcpy(name, "recebido.bin");
}

static void path_join(char *dst, size_t cap, const char *dir, const char *name)
{
	if (strcmp(dir, "/") == 0)
		snprintf(dst, cap, "/%s", name);
	else
		snprintf(dst, cap, "%s/%s", dir, name);
}

static int valid_magic(const uint8_t *p)
{
	return p[0] == FQR_MAGIC0 && p[1] == FQR_MAGIC1 && p[2] == FQR_MAGIC2 && p[3] == FQR_MAGIC3;
}

static int write_all(int fd, const uint8_t *buf, size_t n)
{
	size_t done = 0;
	while (done < n) {
		ssize_t w = write(fd, buf + done, n - done);
		if (w <= 0)
			return -1;
		done += (size_t)w;
	}
	return 0;
}

void fqr_stream_init(fqr_stream *s, const char *dir)
{
	memset(s, 0, sizeof(*s));
	s->fd = -1;
	strncpy(s->dir, dir, FQR_DIR_MAX);
	s->dir[FQR_DIR_MAX] = '\0';
}

static int feed_manifest(fqr_stream *s, const uint8_t *pkt, size_t len)
{
	if (len < FQR_HEADER_SIZE + 11)
		return -1;
	if (s->have_manifest)
		return 0; // já temos

	const uint8_t *p = pkt + FQR_HEADER_SIZE;
	uint32_t crc = fqr_rd32(p + 0);
	uint32_t size = fqr_rd32(p + 4);
	uint16_t chunk = fqr_rd16(p + 8);
	uint8_t nameLen = p[10];
	if (size == 0 || chunk == 0)
		return -1;
	if (len < (size_t)(FQR_HEADER_SIZE + 11 + nameLen))
		return -1;
	uint16_t total = fqr_rd16(pkt + 7);
	if (total == 0)
		return -1;

	memcpy(s->name, p + 11, nameLen);
	s->name[nameLen] = '\0';
	sanitize_name(s->name);
	path_join(s->path, sizeof s->path, s->dir, s->name);

	s->fd = open(s->path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (s->fd < 0) {
		s->open_failed = 1;
		return -2;
	}
	// Pré-aloca o arquivo com o tamanho final (zeros sequenciais). Assim toda
	// escrita de chunk depois é "no lugar" (offset < size), evitando o
	// lseek-além-do-EOF que o libfat do DSi não trata bem.
	{
		static const uint8_t zeros[4096] = { 0 };
		uint32_t left = size;
		while (left > 0) {
			size_t w = left < sizeof zeros ? left : sizeof zeros;
			if (write_all(s->fd, zeros, w) != 0) {
				close(s->fd);
				s->fd = -1;
				s->open_failed = 1;
				return -2;
			}
			left -= (uint32_t)w;
		}
	}
	s->bitmap = (uint8_t *)calloc(((size_t)total + 7) / 8, 1);
	if (!s->bitmap) {
		close(s->fd);
		s->fd = -1;
		s->open_failed = 1;
		return -2;
	}

	s->crc32 = crc;
	s->size = size;
	s->chunk = chunk;
	s->total = total;
	s->have_manifest = 1;
	return 0;
}

static int feed_data(fqr_stream *s, const uint8_t *pkt, size_t len)
{
	if (!s->have_manifest || s->fd < 0)
		return -1;
	uint16_t index = fqr_rd16(pkt + 5); // 1-based
	if (index == 0 || index > s->total)
		return -1;
	if (bit_get(s->bitmap, index - 1))
		return 0; // duplicado

	size_t payload = len - FQR_HEADER_SIZE;
	size_t off = (size_t)(index - 1) * s->chunk;
	if (off >= s->size)
		return -1;
	size_t maxcopy = s->size - off;
	if (payload > maxcopy)
		payload = maxcopy;

	if (lseek(s->fd, (off_t)off, SEEK_SET) < 0)
		return -1;
	if (write_all(s->fd, pkt + FQR_HEADER_SIZE, payload) != 0)
		return -1;

	bit_set(s->bitmap, index - 1);
	s->got++;
	return 0;
}

int fqr_stream_feed(fqr_stream *s, const uint8_t *pkt, size_t len)
{
	if (len < FQR_HEADER_SIZE || !valid_magic(pkt))
		return -1;
	switch (pkt[4]) {
	case FQR_TYPE_MANIFEST: return feed_manifest(s, pkt, len);
	case FQR_TYPE_DATA:     return feed_data(s, pkt, len);
	default:                return -1;
	}
}

int fqr_stream_complete(const fqr_stream *s)
{
	return s->have_manifest && s->got == s->total;
}

int fqr_stream_verify(fqr_stream *s)
{
	if (!fqr_stream_complete(s))
		return 0;
	// fecha a escrita e reabre só p/ leitura
	if (s->fd >= 0) {
		close(s->fd);
		s->fd = -1;
	}
	int rf = open(s->path, O_RDONLY);
	if (rf < 0)
		return 0;

	uint8_t buf[1024];
	uint32_t c = crc32_init();
	uint32_t remaining = s->size;
	int ok = 1;
	while (remaining > 0) {
		size_t want = remaining < sizeof buf ? remaining : sizeof buf;
		ssize_t rd = read(rf, buf, want);
		if (rd <= 0) { ok = 0; break; }
		c = crc32_update(c, buf, (size_t)rd);
		remaining -= (uint32_t)rd;
	}
	close(rf);
	return ok && crc32_final(c) == s->crc32;
}

void fqr_stream_close(fqr_stream *s)
{
	if (s->fd >= 0) {
		close(s->fd);
		s->fd = -1;
	}
	free(s->bitmap);
	s->bitmap = NULL;
}
