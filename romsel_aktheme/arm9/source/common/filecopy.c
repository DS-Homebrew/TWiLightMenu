#include <nds.h>
#include <stdio.h>

#include "common/tonccpy.h"
#include "_ansi.h"

// DO NOT INCREASE OR libfat WILL CRASH

#define COPY_BUF_SIZE 0x2000
u8 copyBuf[COPY_BUF_SIZE];

long fsize(const char *path)
{
	FILE* f = fopen(path, "rb");
	if (!f) return 0;
	fseek(f, 0, SEEK_END);
	long ret = ftell(f);
	fclose(f);
	return ret;
}

int fcopy(const char *sourcePath, const char *destinationPath)
{
	// Get source file's size
	long srcsize = fsize(sourcePath);
	sassert(srcsize >= 0, "source cxi is broken!");
	toncset(copyBuf, 0, COPY_BUF_SIZE);
	FILE* sourceFile = fopen(sourcePath, "rb");
	if (!sourceFile) {
		sassert(0, "invalid source file");
		return 1;
	}

	FILE* destinationFile = fopen(destinationPath, "wb");
	if (!destinationFile) {
		fclose(sourceFile);
		sassert(0, "couldnt open destfile");
		return 1;
	}

	size_t numr = 0;
	while ((numr = fread(copyBuf, 1, COPY_BUF_SIZE, sourceFile)) > 0) {
		// Copy file to destination path
		swiWaitForVBlank();
		fwrite(copyBuf, numr, 1, destinationFile);
		swiWaitForVBlank();
		fflush(destinationFile);
		swiWaitForVBlank();
		swiWaitForVBlank();
	};

	swiWaitForVBlank();
	fclose(destinationFile);
	swiWaitForVBlank();
	fclose(sourceFile);
	return 0;
}

int fsizeincrease(const char *sourcePath, const char *tempPath, size_t newsize)
{
	long srcsize = fsize(sourcePath);

    FILE* sourceFile = fopen(sourcePath, "rb");
    if (!sourceFile) {
		return 1;
	}

    FILE* destinationFile = fopen(tempPath, "wb");
	if (!destinationFile) {
		fclose(sourceFile);
		return 1;
	}

	long offset = 0;
	int numr;
	while (1)
	{
		// Copy file to destination path
		if (offset > srcsize) {
			numr = COPY_BUF_SIZE;
			toncset(copyBuf, 0, COPY_BUF_SIZE);
		} else {
			numr = fread(copyBuf, 1, COPY_BUF_SIZE, sourceFile);
		}
		fwrite(copyBuf, 1, numr, destinationFile);
		offset += COPY_BUF_SIZE;

		if (offset > newsize) {
			fclose(sourceFile);
			fclose(destinationFile);
			remove(sourcePath);
			rename(tempPath, sourcePath);
			return 0;
			break;
		}
	}

	return 1;
}
