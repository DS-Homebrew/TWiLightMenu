#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/dlplayPatch.h"
#include "common/logging.h"

#define DLPLAY_HEADER_SIZE 0x160 // Up to the header CRC, which differs between versions

// Each regional pair shares its patch locations between DSi and 3DS.
static const struct {
	const char* name;
	unsigned int size;
	unsigned int originalCrc;
	unsigned int patchedCrc;
	unsigned int patchOffsets[2];
} dlplayVersions[] = {
	{"DSi", 0x69BC0, 0x5F5BFC6A, 0x3D505BFF, {0xD898, 0xDC44}},
	{"DSi iQue", 0x8E1C0, 0x3BB32953, 0xE4BCDB47, {0xD890, 0xDC3C}},
	{"DSi Korean", 0x77DC0, 0xD713F18A, 0xDC551BD6, {0xD898, 0xDC44}},
	{"3DS", 0x69BC0, 0x57D48E69, 0x35DF29FC, {0xD898, 0xDC44}},
	{"3DS Chinese", 0x8E1C0, 0xE760852F, 0x386F773B, {0xD890, 0xDC3C}},
	{"3DS Korean", 0x77DC0, 0x37940F0B, 0x3CD2E557, {0xD898, 0xDC44}},
};
static const int dlplayVersionCount = sizeof(dlplayVersions) / sizeof(dlplayVersions[0]);

static const unsigned char patchCode[] = {0x01, 0x20, 0x70, 0x47}; // movs r0, #1; bx lr

static unsigned int crc32(const unsigned char* buf, size_t size) {
	static unsigned int table[256];
	if (table[1] == 0) {
		for (unsigned int i = 0; i < 256; i++) {
			unsigned int crc = i;
			for (int j = 0; j < 8; j++) {
				crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
			}
			table[i] = crc;
		}
	}

	unsigned int crc = 0xFFFFFFFF;
	for (size_t i = 0; i < size; i++) {
		crc = (crc >> 8) ^ table[(crc ^ buf[i]) & 0xFF];
	}
	return ~crc;
}

// Loads up to `size` bytes and returns the recognized file size, or zero on failure.
static unsigned int loadDlplay(const char* path, unsigned char* buf, size_t size) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		return 0;
	}
	fseek(file, 0, SEEK_END);
	const unsigned int fileSize = ftell(file);
	bool sizeMatches = false;
	for (int i = 0; i < dlplayVersionCount; i++) {
		sizeMatches |= (fileSize == dlplayVersions[i].size);
	}
	fseek(file, 0, SEEK_SET);
	const size_t readSize = size < fileSize ? size : fileSize;
	const bool loaded = sizeMatches && (fread(buf, 1, readSize, file) == readSize);
	fclose(file);
	return loaded ? fileSize : 0;
}

const char* dlplayGetBootPath(const char* srcPath, const char* patchedPath) {
	unsigned char srcHeader[DLPLAY_HEADER_SIZE];
	const unsigned int size = loadDlplay(srcPath, srcHeader, sizeof(srcHeader));
	if (!size) {
		logPrint("DS Download Play RSA patch: Unsupported file, launching unpatched\n");
		return srcPath;
	}
	unsigned char* buf = (unsigned char*)malloc(size);
	if (!buf) {
		logPrint("DS Download Play RSA patch: Out of memory, launching unpatched\n");
		return srcPath;
	}

	// Reuse the patched copy from a previous launch, if it was made from this source
	if (loadDlplay(patchedPath, buf, size) == size
	 && memcmp(buf, srcHeader, sizeof(srcHeader)) == 0) {
		const unsigned int crc = crc32(buf, size);
		for (int i = 0; i < dlplayVersionCount; i++) {
			if (crc == dlplayVersions[i].patchedCrc) {
				free(buf);
				logPrint("DS Download Play RSA patch: Using %s (%s)\n", patchedPath, dlplayVersions[i].name);
				return patchedPath;
			}
		}
	}

	int version = -1;
	if (loadDlplay(srcPath, buf, size) == size) {
		const unsigned int crc = crc32(buf, size);
		for (int i = 0; i < dlplayVersionCount; i++) {
			if (crc == dlplayVersions[i].originalCrc) {
				version = i;
				break;
			}
		}
	}
	if (version < 0) {
		free(buf);
		logPrint("DS Download Play RSA patch: Unsupported file, launching unpatched\n");
		return srcPath;
	}

	for (unsigned int offset : dlplayVersions[version].patchOffsets) {
		memcpy(buf + offset, patchCode, sizeof(patchCode));
	}

	bool written = false;
	if (crc32(buf, size) == dlplayVersions[version].patchedCrc) {
		FILE* file = fopen(patchedPath, "wb");
		if (file) {
			written = (fwrite(buf, 1, size, file) == size);
			fclose(file);
		}
	}
	free(buf);

	if (!written) {
		remove(patchedPath);
		logPrint("DS Download Play RSA patch: Failed to write %s, launching unpatched\n", patchedPath);
		return srcPath;
	}

	logPrint("DS Download Play RSA patch: Created %s (%s)\n", patchedPath, dlplayVersions[version].name);
	return patchedPath;
}
