
#include <stdio.h>
#include <sys/statvfs.h>
#include <nds.h>
#include "utils.h"

swiSHA1context_t sha1ctx;

static inline int htoi(char a){
	if(a >= '0' && a <= '9'){
		return a - '0';
	}else if(a >= 'a' && a <= 'f'){
		return a - ('a' - 0xa);
	}else if(a >= 'A' && a <= 'F'){
		return a - ('A' - 0xa);
	}else{
		return -1;
	}
}

int hex2bytes(uint8_t *out, unsigned byte_len, const char *in){
	if (strlen(in) < byte_len << 1){
		iprintf("%s: invalid input length, expecting %u, got %u.\n",
			__FUNCTION__, (unsigned)byte_len << 1, (unsigned)strlen(in));
		return -1;
	}
	for(unsigned i = 0; i < byte_len; ++i){
		int h = htoi(*in++), l = htoi(*in++);
		if(h == -1 || l == -1){
			iprintf("%s: invalid input \"%c%c\"\n",
				__FUNCTION__, *(in - 2), *(in - 1));
			return -2;
		}
		*out++ = (h << 4) + l;
	}
	return 0;
}

static char str_buf[0x10];

const char *to_mebi(size_t size) {
	if (size % (1024 * 1024)) {
		sprintf(str_buf, "%.2f", (float)(((double)size) / 1024 / 1024));
	} else {
		siprintf(str_buf, "%u", (unsigned)(size >> 20));
	}
	return str_buf;
}

int save_file(const char *filename, const void *buffer, size_t size, int save_sha1) {
	FILE *f = fopen(filename, "wb");
	if (f == 0) {
		//iprintf("failed to open %s to write\n", filename);
		return -1;
	}
	size_t written = fwrite(buffer, 1, size, f);
	fclose(f);
	if (written != size) {
		//iprintf("error writting %s\n", filename);
		return -2;
	} else {
		//iprintf("saved %s\n", filename);
	}
	if (save_sha1) {
		sha1ctx.sha_block = 0;
		swiSHA1Init(&sha1ctx);
		swiSHA1Update(&sha1ctx, buffer, size);
		save_sha1_file(filename);
	}
	return 0;
}

int load_file(void **pbuf, size_t *psize, const char *filename, int verify_sha1, int align) {
	FILE *f = fopen(filename, "rb");
	if (f == 0) {
		//iprintf("failed to open %s to read\n", filename);
		return -1;
	}
	int ret;
	fseek(f, 0, SEEK_END);
	*psize = ftell(f);
	if (*psize == 0) {
		*pbuf = 0;
		ret = 1;
	} else {
		if (align) {
			*pbuf = memalign(align, *psize);
		} else {
			*pbuf = malloc(*psize);
		}
		if (*pbuf == 0) {
			//printf("failed to alloc memory\n");
			ret = -1;
		} else {
			fseek(f, 0, SEEK_SET);
			unsigned read = fread(*pbuf, 1, *psize, f);
			if (read != *psize) {
				//iprintf("error reading %s\n", filename);
				free(*pbuf);
				*pbuf = 0;
				ret = -2;
			} else {
				//iprintf("loaded %s(%u)\n", filename, read);
				if (verify_sha1) {
					//TODO:
					//iprintf("%s: not implemented\n", __FUNCTION__);
				}
				ret = 0;
			}
		}
	}
	fclose(f);
	return ret;
}

int load_block_from_file(void *buf, const char *filename, unsigned offset, unsigned size) {
	FILE *f = fopen(filename, "rb");
	if (f == 0) {
		//iprintf("failed to open %s\n", filename);
		return -1;
	}
	unsigned read;
	int ret;
	if (offset != 0 && fseek(f, offset, SEEK_SET) != 0) {
		//printf("seek error\n");
		ret = -1;
	} else if ((read = fread(buf, 1, size, f)) != size) {
		//iprintf("read error, expecting %u, got %u\n", size, read);
		ret = -1;
	} else {
		ret = 0;
	}
	fclose(f);
	return ret;
}

// you should have updated the sha1 context before calling save_sha1_file
// example: save_file() in this file and backup() in nand.c

int save_sha1_file(const char *filename) {
	size_t len_fn = strlen(filename);
	char *sha1_fn = (char *)malloc(len_fn + 6);
	siprintf(sha1_fn, "%s.sha1", filename);
	// 20 bytes each use 2 chars, space, asterisk, filename, new line
	size_t len_buf = 2 * 20 + 1 + 1 + len_fn + 1;
	char *sha1_buf = (char *)malloc(len_buf + 1); // extra for \0
	char *p = sha1_buf;
	char *digest = (char *)malloc(20);
	swiSHA1Final(digest, &sha1ctx);
	for (int i = 0; i < 20; ++i) {
		p += siprintf(p, "%02X", digest[i]);
	}
	free(digest);
	siprintf(p, " *%s\n", filename);
	int ret = save_file(sha1_fn, (u8*)sha1_buf, len_buf, false);
	free(sha1_fn);
	free(sha1_buf);
	return ret;
}

void print_bytes(const void *buf, size_t len) {
	const unsigned char *p = (const unsigned char *)buf;
	for(size_t i = 0; i < len; ++i) {
		iprintf("%02x", *p++);
	}
}

// out must be big enough
// can work in place
void utf16_to_ascii(uint8_t *out, const uint16_t *in, unsigned len) {
	const uint16_t *end = in + len;
	while (in < end){
		uint16_t c = *in++;
		if (c == 0) {
			*out = 0;
			break;
		} else if (c < 0x80) {
			*out++ = (uint8_t)c;
		}
	}
}

size_t df(const char *path, int verbose) {
	// it's amazing libfat even got this to work
	struct statvfs s;
	statvfs(path, &s);
	size_t free = s.f_bsize * s.f_bfree;
	if (verbose) {
		//iprintf("%s", to_mebi(free));
		//iprintf("/%s MB (free/total)\n", to_mebi(s.f_bsize * s.f_blocks));
	}
	return free;
}

